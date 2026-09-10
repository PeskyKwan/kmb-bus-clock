"""Verify the real framebuffer has a visible primary bus, using no-reset USB."""
import argparse
import json
from pathlib import Path
import time
from device_position_audit import NoResetSerial

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', required=True)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--version', default='0.2.7')
parser.add_argument('--buses', type=int, choices=(1,2), default=1)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
serial = NoResetSerial(args.port)
try:
    end = time.monotonic()+180
    while time.monotonic() < end:
        if serial is None:
            serial = NoResetSerial(args.port)
        state = serial.command({'cmd': 'state'})
        serial.close()
        serial = None
        print({k: state.get(k) for k in ('version','route','etaCode','roadReady','animationVisible','animationSource','heap')}, flush=True)
        if state.get('roadReady') and state.get('animationVisible') and state.get('animationCount',0)>=args.buses:
            break
        time.sleep(3)
    else:
        raise TimeoutError('visible bus on loaded map')
    assert state['version']==args.version and state['displayReady']
    assert 0 < state['mapMeters'] <= 3001
    assert state.get('animationSource') in ('matched','eta-guess','held')
    if args.buses==2:
        assert state.get('animationSource2') in ('matched','eta-guess','held')
    state.pop('ssid', None)
    state.pop('ip', None)
    (args.output/'visible-state.json').write_text(json.dumps(state,ensure_ascii=False,indent=2)+'\n')
    time.sleep(.8)  # Let the initial sprite fade-in reach its exact body colour.
    serial = NoResetSerial(args.port)
    serial.write({'cmd':'settings','action':'capture-rle'})
    rows = {}
    end = time.monotonic()+60
    while len(rows)<240 and time.monotonic()<end:
        raw = serial.line().decode(errors='replace').strip()
        if raw.startswith('RLE '):
            fields=raw.split();y=int(fields[1]);pixels=[]
            for field in fields[2:]:
                count,value=field.split(':');count=int(count)
                assert 1<=count<=320
                pixels.extend([int(value,16)]*count)
                assert len(pixels)<=320
            assert len(pixels)==320 and 0<=y<240
            rows[y]=pixels
    assert len(rows)==240, len(rows)
    # This body colour is unique to the little bus, inside the map bounds.
    bus_pixels = [(x,y) for y in range(35,175) for x in range(134,308) if rows[y][x]==0xD328]
    assert len(bus_pixels)>=35, ('bus body absent from framebuffer',len(bus_pixels))
    remaining=set(bus_pixels)
    components=[]
    while remaining:
        stack=[remaining.pop()];component=[]
        while stack:
            x,y=stack.pop();component.append((x,y))
            for point in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
                if point in remaining:
                    remaining.remove(point);stack.append(point)
        if len(component)>=35:
            components.append(component)
    assert len(components)>=args.buses, ('distinct bus bodies',len(components),'expected',args.buses)
    rgb = bytearray()
    for y in range(240):
        for pixel in rows[y]:
            rgb.extend(((pixel>>11&31)*255//31,(pixel>>5&63)*255//63,(pixel&31)*255//31))
    from PIL import Image
    Image.frombytes('RGB',(320,240),bytes(rgb)).save(args.output/'physical-framebuffer.png')
    bounds=[(min(x for x,y in c),min(y for x,y in c),max(x for x,y in c),max(y for x,y in c)) for c in components]
    print('Physical distinct buses PASS:',len(components),'pixels',len(bus_pixels),'bounds',bounds,flush=True)
finally:
    if serial is not None:
        serial.close()
