"""Sample the clock's LAN `/state` JSON on an interval and append JSONL.

Read-only HTTP GET; no USB, no reset, no settings change. Network identity
fields are dropped before writing. Failed polls are recorded, not retried.
"""
import argparse
import datetime
import json
from pathlib import Path
import time
import urllib.request


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--url', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--interval', type=float, default=60)
    parser.add_argument('--hours', type=float, default=1)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    end = time.monotonic() + args.hours * 3600
    while time.monotonic() < end:
        started = time.monotonic()
        record = {'capturedUTC': datetime.datetime.now(datetime.timezone.utc).isoformat()}
        try:
            with urllib.request.urlopen(args.url, timeout=10) as response:
                state = json.loads(response.read().decode('utf-8'))
            for key in ('ssid', 'ip'):
                state.pop(key, None)
            record.update(state)
        except Exception as error:  # keep sampling through transient LAN/board issues
            record['sampleError'] = f'{type(error).__name__}: {error}'
        with args.output.open('a') as out:
            out.write(json.dumps(record, ensure_ascii=False) + '\n')
        time.sleep(max(0, args.interval - (time.monotonic() - started)))


if __name__ == '__main__':
    main()
