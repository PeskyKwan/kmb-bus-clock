"""Verify the real framebuffer has a visible primary bus, using no-reset USB."""
import argparse
import json
from pathlib import Path
import time
from device_position_audit import NoResetSerial

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
serial = NoResetSerial(args.port)
try:
    end = time.monotonic()+180
    while time.monotonic() < end:
        state = serial.command({'cmd': 'state'})
        print({k: state.get(k) for k in ('version','route','etaCode','roadReady','animationVisible','animationSource','heap')}, flush=True)
        if state.get('roadReady') and state.get('animationVisible'):
            break
        time.sleep(3)
    else:
        raise TimeoutError('visible bus on loaded map')
    assert state['version']=='0.2.6' and state['displayReady']
    assert 0 < state['mapMeters'] <= 3001
    assert state.get('animationSource') in ('matched','eta-guess','held')
    state.pop('ssid', None)
    state.pop('ip', None)
    (args.output/'visible-state.json').write_text(json.dumps(state,ensure_ascii=False,indent=2)+'\n')
    serial.write({'cmd':'settings','action':'capture'})
    rows = {}
    end = time.monotonic()+60
    while len(rows)<240 and time.monotonic()<end:
        raw = serial.line().decode(errors='replace').strip()
        if not raw.startswith('PIX '):
            continue
        _, y, data = raw.split(' ', 2)
        assert len(data)==1280
        rows[int(y)] = [int(data[i:i+4],16) for i in range(0,1280,4)]
    assert len(rows)==240, len(rows)
    # This body colour is unique to the little bus, inside the map bounds.
    bus_pixels = [(x,y) for y in range(35,175) for x in range(134,308) if rows[y][x]==0xD328]
    assert len(bus_pixels)>=35, ('bus body absent from framebuffer',len(bus_pixels))
    rgb = bytearray()
    for y in range(240):
        for pixel in rows[y]:
            rgb.extend(((pixel>>11&31)*255//31,(pixel>>5&63)*255//63,(pixel&31)*255//31))
    from PIL import Image
    Image.frombytes('RGB',(320,240),bytes(rgb)).save(args.output/'physical-framebuffer.png')
    print('Physical bus pixels PASS:',len(bus_pixels),'bounds',min(x for x,y in bus_pixels),min(y for x,y in bus_pixels),max(x for x,y in bus_pixels),max(y for x,y in bus_pixels),flush=True)
finally:
    serial.close()
