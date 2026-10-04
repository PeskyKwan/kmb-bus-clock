"""Read-only no-reset heap evidence capture.

Passively records the firmware's periodic USB state events and, at a slower
interval, the region-level `{"cmd":"heap"}` dump. Network identity is removed
before anything is written. Optional `--command` sends one JSON command first
(for the hidden self-heal test hooks).
"""
import argparse
import datetime
import json
from pathlib import Path
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parent))
from device_position_audit import NoResetSerial  # noqa: E402


def stamp():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--seconds', type=float, default=600)
    parser.add_argument('--heap-every', type=float, default=60)
    parser.add_argument('--command', action='append', default=[])
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    serial = NoResetSerial(args.port)
    end = time.monotonic() + args.seconds
    next_heap = time.monotonic()
    regions = []
    try:
        for raw in args.command:
            serial.write(json.loads(raw))
        with args.output.open('a') as out:
            while time.monotonic() < end:
                if time.monotonic() >= next_heap:
                    serial.write({'cmd': 'heap'})
                    next_heap = time.monotonic() + args.heap_every
                line = serial.line(1.0)
                if not line:
                    continue
                text = line.decode('utf-8', 'replace').strip()
                try:
                    event = json.loads(text)
                except ValueError:
                    # heap_caps_print_heap_info region lines precede the JSON summary.
                    if text.startswith('At 0x') or 'Totals:' in text or text.startswith('Heap summary'):
                        regions.append(text)
                    elif any(k in text for k in ('Guru', 'Backtrace', 'abort', 'rst:', 'panic', 'KMB_CLOCK_LIVE_READY')):
                        out.write(json.dumps({'event': 'console', 'text': text, 'capturedUTC': stamp()}) + '\n')
                        print({'console': text}, flush=True)
                    continue
                if not isinstance(event, dict):
                    continue
                event.pop('ssid', None)
                event.pop('ip', None)
                event['capturedUTC'] = stamp()
                if event.get('event') == 'heap':
                    event['regions'] = regions
                    regions = []
                out.write(json.dumps(event, ensure_ascii=False) + '\n')
                out.flush()
                if event.get('event') in ('state', 'heap', 'heal'):
                    keys = ('event', 'uptimeS', 'etaCode', 'heap8', 'largest8', 'minLargest8', 'preTlsLargest8',
                            'failedAllocs', 'free', 'largest', 'freeBlocks', 'healStage', 'softRecoveries')
                    print({k: event[k] for k in keys if k in event}, flush=True)
    finally:
        serial.close()


if __name__ == '__main__':
    main()
