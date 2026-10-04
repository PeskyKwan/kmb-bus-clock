"""Exercise ETA self-heal on the real board through hidden USB hooks (no reset).

Simulated TLS allocation failures never touch the network. Stage `soft` proves
soft recovery and automatic ETA return. Stage `dry` walks soft -> soft+Wi-Fi STA
-> last-resort restart with the restart replaced by a dry run, then proves the
six-hour guard blocks a second one. Stage `real` performs exactly one guarded
restart, checks the recorded reason, proves the guard blocks a second restart,
then clears the test record. Saved settings/NVS are never written.
"""
import argparse
import datetime
import json
from pathlib import Path
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parent))
from device_position_audit import NoResetSerial  # noqa: E402


class Board:
    def __init__(self, port, log):
        self.serial = NoResetSerial(port)
        self.log = log.open('a')
        self.last = {}

    def close(self):
        self.serial.close()
        self.log.close()

    def send(self, obj):
        self.serial.write(obj)
        self.record({'event': 'sent', 'command': obj})

    def record(self, event):
        event.pop('ssid', None)
        event.pop('ip', None)
        event['capturedUTC'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        self.log.write(json.dumps(event, ensure_ascii=False) + '\n')
        self.log.flush()

    def wait(self, predicate, timeout, label):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            raw = self.serial.line(1.0)
            if not raw:
                continue
            text = raw.decode('utf-8', 'replace').strip()
            try:
                event = json.loads(text)
            except ValueError:
                if any(k in text for k in ('Guru', 'Backtrace', 'rst:', 'KMB_CLOCK_LIVE_READY')):
                    event = {'event': 'console', 'text': text}
                else:
                    continue
            if not isinstance(event, dict):
                continue
            self.record(event)
            if event.get('event') in ('state', 'healState'):
                self.last.update(event)
            if event.get('event') in ('heal', 'console'):
                print(' ', event, flush=True)
            if predicate(event):
                print('PASS', label, flush=True)
                return event
        raise TimeoutError(label)

    def heal(self, **command):
        self.send({'cmd': 'heal', **command})
        return self.wait(lambda e: e.get('event') == 'healState', 10, 'heal ' + json.dumps(command))


def healthy(e):
    return e.get('event') == 'state' and e.get('etaCode') == 2 and e.get('etaFailStreak') == 0


def fail_until_soft(board, extra):
    """Queue simulated failures and force polls until soft recovery fires."""
    board.heal(fail=extra, poll=True)
    for streak in (1, 2):
        board.wait(lambda e, s=streak: e.get('event') == 'state' and e.get('etaFailStreak') == s, 60, f'streak {streak}')
        board.heal(poll=True)
    return board.wait(lambda e: e.get('event') == 'heal' and e.get('action') == 'soft', 60, 'soft recovery after 3 failures')


def sta_restart(board, attempt):
    before = board.heal(skipS=180)
    board.wait(lambda e: e.get('event') == 'heal' and e.get('action') == 'soft-sta', 30, f'soft + Wi-Fi STA restart (attempt {attempt})')
    board.wait(lambda e: e.get('event') == 'state' and e.get('connected') and e.get('staDisconnects', 0) > before['staDisconnects']
               and e.get('staGotIp', 0) > before['staGotIp'], 60, 'STA really disconnected and got IP again with saved credentials')


def walk_to_restart(board, action):
    fail_until_soft(board, 50)
    for attempt in (2, 3):
        sta_restart(board, attempt)
    baseline = board.heal(skipS=600)['wouldRestart']
    time.sleep(6)
    state = board.heal()
    assert state['healStage'] == 3 and state['wouldRestart'] == baseline, 'no restart before 25 minutes of failure'
    board.heal(skipS=1200)
    return board.wait(lambda e: e.get('event') == 'heal' and e.get('action') == action, 30, f'{action} after soft recovery exhausted and 25 min failing')


def guard_blocks(board):
    time.sleep(12)
    state = board.heal()
    assert state['restartBlock'] == 2, ('expected six-hour spacing block', state['restartBlock'])
    return state


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--stage', choices=('soft', 'dry', 'real'), required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    board = Board(args.port, args.output)
    try:
        board.send({'cmd': 'state'})
        board.wait(healthy, 90, 'healthy ETA before test')
        if args.stage == 'soft':
            board.heal(reset=True, dryRun=True)
            fail_until_soft(board, 3)
            board.wait(healthy, 60, 'ETA recovered without restart')
            state = board.heal()
            assert state['softRecoveries'] >= 1 and state['healStage'] == 0
        elif args.stage == 'dry':
            board.heal(reset=True, dryRun=True)
            walk_to_restart(board, 'would-restart')
            state = guard_blocks(board)
            assert state['wouldRestart'] == 1, state['wouldRestart']
            board.heal(reset=True, poll=True)
            board.wait(healthy, 90, 'ETA recovered after dry run')
        else:
            board.heal(reset=True, clearRecord=True)
            walk_to_restart(board, 'restart')
            board.wait(lambda e: e.get('event') == 'console' and 'KMB_CLOCK_LIVE_READY' in e.get('text', ''), 60, 'board rebooted')
            board.wait(lambda e: e.get('event') == 'state' and e.get('restartReason') == 'self-heal' and e.get('healRestarts') == 1, 30, 'restart reason recorded as self-heal')
            board.wait(healthy, 120, 'ETA healthy after guarded restart')
            walk_to_restart_blocked(board)
            board.heal(reset=True, clearRecord=True, poll=True)
            board.wait(healthy, 90, 'ETA recovered; test record cleared')
        state = board.last
        print({k: state.get(k) for k in ('version', 'route', 'bound', 'service', 'leadSeconds', 'brightness', 'themeMode', 'screenFlipped',
                                         'connected', 'etaCode', 'softRecoveries', 'healRestarts', 'restartReason', 'wouldRestart')}, flush=True)
    finally:
        board.close()


def walk_to_restart_blocked(board):
    """Second outage inside six hours: soft recovery runs, restart stays blocked."""
    fail_until_soft(board, 50)
    for attempt in (2, 3):
        sta_restart(board, attempt)
    board.heal(skipS=1800)
    state = guard_blocks(board)
    assert state['healRestarts'] == 1 and state['healStage'] == 3
    print('PASS second restart blocked by six-hour guard; board stayed up', flush=True)


if __name__ == '__main__':
    main()
