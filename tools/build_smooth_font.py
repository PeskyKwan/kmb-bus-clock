from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json,math,os
r=Path(__file__).resolve().parents[1]
chars=set(''.join(s['name_tc'] for s in json.loads((r/'docs/stops.json').read_text())['data']))
chars.update((r/'src/main.cpp').read_text());chars.update((r/'src/native_settings.inc').read_text());chars.update((r/'src/map_render.inc').read_text());chars.update(chr(i) for i in range(32,127));chars.update('道路圖預估未有預報等待同步分鐘到')
chars=sorted({c for c in chars if 32<=ord(c)<65536},key=ord)
fonts=[os.environ.get('KMB_CJK_FONT','/System/Library/Fonts/STHeiti Medium.ttc'),os.environ.get('KMB_LATIN_FONT','/System/Library/Fonts/Supplemental/Arial Bold.ttf')]
out=['#pragma once','#include <Arduino.h>','const uint16_t aaCodes[] PROGMEM={'+','.join(str(ord(c)) for c in chars)+'};','constexpr int aaCount=sizeof(aaCodes)/sizeof(aaCodes[0]);']
for sz in [12,16,20,39,67]:
 cs=chars if sz<=20 else [chr(i) for i in range(32,127)]
 values=[];widths=[]
 for c in cs:
  f=ImageFont.truetype(fonts[1 if ord(c)<128 else 0],sz*3)
  advance=min(sz,math.ceil(f.getlength(c)/3)) if ord(c)<128 else sz
  im=Image.new('L',(sz*3,sz*3));d=ImageDraw.Draw(im);box=d.textbbox((0,0),c,font=f);d.text(((advance*3-(box[2]-box[0]))/2-box[0],(sz*3-(box[3]-box[1]))/2-box[1]),c,font=f,fill=255)
  im=im.resize((sz,sz),Image.Resampling.LANCZOS);vals=[round(v/17) for v in im.getdata()]
  if len(vals)%2:vals.append(0)
  values.extend((vals[i]<<4)|vals[i+1] for i in range(0,len(vals),2));widths.append(advance)
 out.append(f'const uint8_t aaWidths{sz}[] PROGMEM={{'+','.join(map(str,widths))+'};')
 out.append(f'const uint8_t aaData{sz}[] PROGMEM={{')
 for i in range(0,len(values),40):out.append(','.join(map(str,values[i:i+40]))+',')
 out.append('};')
(r/'src/smooth_font.h').write_text('\n'.join(out))
print('Smooth glyph set generated:',len(chars))
