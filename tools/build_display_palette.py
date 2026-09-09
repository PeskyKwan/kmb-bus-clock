from pathlib import Path
from PIL import Image
import re,math
r=Path(__file__).resolve().parents[1]
def unpack(v):return ((v>>11&31)*255//31,(v>>5&63)*255//63,(v&31)*255//31)
def pack(c):return ((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3)
def blend(f,b,a):return (((((f>>11)&31)*a+((b>>11)&31)*(15-a)+7)//15)<<11)|(((((f>>5)&63)*a+((b>>5)&63)*(15-a)+7)//15)<<5)|(((f&31)*a+(b&31)*(15-a)+7)//15)
source=''.join((r/'src'/x).read_text() for x in ['main.cpp','native_settings.inc'])
reserved=set(int(s,16) for s in re.findall(r'\b0x[0-9A-Fa-f]{4}\b',source))
for fg,bg in [(0x21e6,x) for x in [0xff9b,0xfffe,0xef7b,0xf6b8,0xef18,0xeddf]]+[(0xff9b,0xca07),(0xff9b,0x21e6),(0xca07,0xff9b),(0xfffe,0xca07)]:
 reserved.update(blend(fg,bg,a) for a in range(16))
colors=sorted(reserved);assert len(colors)<256
im=Image.open(r/'assets/design-base.png').convert('RGB');q=im.quantize(colors=256-len(colors));pal=q.getpalette()
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
