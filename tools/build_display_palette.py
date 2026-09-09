from pathlib import Path
from PIL import Image
import re,math
r=Path(__file__).resolve().parents[1]
def unpack(v):return ((v>>11&31)*255//31,(v>>5&63)*255//63,(v&31)*255//31)
def pack(c):return ((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3)
def blend(f,b,a):return (((((f>>11)&31)*a+((b>>11)&31)*(15-a)+7)//15)<<11)|(((((f>>5)&63)*a+((b>>5)&63)*(15-a)+7)//15)<<5)|(((f&31)*a+(b&31)*(15-a)+7)//15)
source=''.join((r/'src'/x).read_text() for x in ['main.cpp','native_settings.inc','map_render.inc'])
reserved=set(int(s,16) for s in re.findall(r'\b0x[0-9A-Fa-f]{4}\b',source))
pairs=[]
for ink,bg,mp,panel,feature,field in [(0x21e6,0xff7a,0xd71d,0xef18,0xf6b8,0xfffe),(0xff7a,0x10e3,0x224c,0x29e8,0x4289,0x1924)]:
 pairs.extend([(ink,x) for x in (bg,mp,panel,feature,field)]+[(bg,ink),(bg,0xca07)])
pairs.extend([(0xfffe,0xca07),(0x21e6,0xfffe)])
for fg,bg in pairs:reserved.update(blend(fg,bg,a) for a in range(16))
print("Reserved:",len(reserved));colors=sorted(reserved);assert len(colors)<256
im=Image.new('RGB',(16,16));im.putdata([(x*17,y*17,(x+y)*8) for y in range(16) for x in range(16)]);q=im.quantize(colors=256-len(colors));pal=q.getpalette()
for i in range(0,3*(256-len(colors)),3):
 c=pack(pal[i:i+3])
 if c not in colors:colors.append(c)
while len(colors)<256:colors.append(colors[-1])
rgb=[unpack(c) for c in colors];exact={c:i for i,c in enumerate(colors)};lookup=[]
for c in range(65536):
 if c in exact:lookup.append(exact[c]);continue
 red,green,blue=unpack(c)
 lookup.append(min(range(256),key=lambda i:2*(red-rgb[i][0])**2+3*(green-rgb[i][1])**2+(blue-rgb[i][2])**2))
out=['#pragma once','#include <Arduino.h>','const uint16_t displayPalette[] PROGMEM={'+','.join(map(str,colors))+'};','const uint8_t displayColorIndex[] PROGMEM={']
for i in range(0,65536,40):out.append(','.join(map(str,lookup[i:i+40]))+',')
out.append('};');(r/'src/display_palette.h').write_text('\n'.join(out))
assert all(colors[lookup[c]]==c for c in reserved)
preview=Image.new('RGB',im.size);preview.putdata([rgb[lookup[pack(c)]] for c in im.getdata()]);preview.save(r/'assets/palette-preview.png')
print('Exact UI/antialias colors preserved:',len(reserved))
