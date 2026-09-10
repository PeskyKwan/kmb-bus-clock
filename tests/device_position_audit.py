"""Read-only POSIX/no-reset serial and optional official-feed evidence capture."""
import argparse
import datetime
import json
import os
from pathlib import Path
import select
import termios
import time
import tty
import urllib.request


class NoResetSerial:
    def __init__(self, port):
        self.fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        self.rx = b''
        tty.setraw(self.fd)
        attrs = termios.tcgetattr(self.fd)
        attrs[4] = attrs[5] = termios.B115200
        termios.tcsetattr(self.fd, termios.TCSANOW, attrs)

    def close(self):
        os.close(self.fd)

    def write(self, obj):
        os.write(self.fd, (json.dumps(obj) + '\n').encode())

    def line(self, timeout=.5):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            if b'\n' in self.rx:
                line, self.rx = self.rx.split(b'\n', 1)
                return line
            if select.select([self.fd], [], [], max(0, min(.2, end-time.monotonic())))[0]:
                self.rx += os.read(self.fd, 16384)
        return b''

    def command(self, obj, event='state', timeout=15):
        self.write(obj)
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            raw = self.line()
            try:
                result = json.loads(raw)
            except (ValueError, UnicodeError):
                continue
            if result.get('event') == event:
                return result
        raise TimeoutError(event)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--samples', type=int, default=1)
    parser.add_argument('--interval', type=float, default=30)
    parser.add_argument('--feeds', action='store_true')
    parser.add_argument('--position', action='store_true')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    serial = NoResetSerial(args.port)
    try:
        for i in range(args.samples):
            start = time.monotonic()
            if serial is None:
                serial = NoResetSerial(args.port)
            state = serial.command({'cmd': 'state'})
            # Do not persist network identifiers or credentials in diagnostics.
            state.pop('ssid', None)
            state.pop('ip', None)
            state['capturedUTC'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
            with (args.output/'states.jsonl').open('a') as f:
                f.write(json.dumps(state, ensure_ascii=False)+'\n')
            print({k: state.get(k) for k in ('capturedUTC','version','route','etaCode','positionSamples','positionSamples2','animationCount','heap')}, flush=True)
            if args.position:
                details = serial.command({'cmd': 'position'}, 'position')
                (args.output/f'{i:03d}-position.json').write_text(json.dumps(details, ensure_ascii=False))
            # Avoid retaining an idle CH340 handle through network calls/waits.
            serial.close()
            serial = None
            if args.feeds:
                for name, path in (
                    ('target', f"eta/{state['stop']}/{state['route']}/{state['service']}"),
                    ('route', f"route-eta/{state['route']}/{state['service']}"),
                ):
                    try:
                        with urllib.request.urlopen('https://data.etabus.gov.hk/v1/transport/kmb/'+path, timeout=20) as response:
                            data = response.read(262145)
                        assert len(data) <= 262144
                        json.loads(data)
                        (args.output/f'{i:03d}-{name}.json').write_bytes(data)
                    except Exception as error:
                        print(name, type(error).__name__, str(error), flush=True)
            if i+1 < args.samples:
                time.sleep(max(0, args.interval-(time.monotonic()-start)))
    finally:
        if serial is not None:
            serial.close()


if __name__ == '__main__':
    main()
