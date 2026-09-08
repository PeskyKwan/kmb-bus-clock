from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
import json,math,os
root=Path(__file__).resolve().parents[1]; S=3
BG='#fff3d9'; INK='#263f36'; RED='#c94239'
font=os.environ.get('KMB_CJK_FONT','/System/Library/Fonts/STHeiti Medium.ttc'); bold=os.environ.get('KMB_LATIN_FONT','/System/Library/Fonts/Supplemental/Arial Bold.ttf')
def make(w,h,color=BG):return Image.new('RGB',(w*S,h*S),color)
def text(im,xy,s,size,color=INK,latin=False,anchor=None):
 d=ImageDraw.Draw(im);d.text(tuple(v*S for v in xy),s,font=ImageFont.truetype(bold if latin else font,size*S),fill=color,anchor=anchor)
def rect(im,box,color,r=0):
 d=ImageDraw.Draw(im);b=tuple(v*S for v in box)
 if r:d.rounded_rectangle(b,r*S,fill=color)
 else:d.rectangle(b,fill=color)
def project(lon,lat):
 x=810000*math.radians(lon-114.2599)+52
 m=lambda a:math.log(math.tan(math.pi/4+math.radians(a)/2))
 return (x,72-810000*(m(lat)-m(22.3684)))
im=make(320,240);d=ImageDraw.Draw(im)
d.ellipse((14*S,10*S,118*S,114*S),fill=RED)
text(im,(66,21),'巴士站',11,'#fffaf0',anchor='mt')
rect(im,(26,38,106,81),'#fffaf0')
# Centre visible glyph bounds, not the font ascent/line box.
route_font=ImageFont.truetype(bold,39*S)
left,top,right,bottom=d.textbbox((0,0),'92',font=route_font)
d.text((66*S-(left+right)/2,59.5*S-(top+bottom)/2),'92',font=route_font,fill=INK)
text(im,(66,85),'白沙灣',17,'#fffaf0',anchor='mt')
text(im,(135,10),'往彩虹',15);text(im,(251,12),'示範',15,RED)
text(im,(66,179),'分鐘到',17,anchor='mt')
mp=make(174,155,'#e8eddc');md=ImageDraw.Draw(mp)
data=json.loads((root/'docs/map-roads.json').read_text()); points=[]
for way in data['elements']:
 p=[project(n['lon'],n['lat']) for n in way['geometry']];q=[(round(x*S),round(y*S)) for x,y in p]
 md.line(q,fill='#d4cbb4',width=15*S,joint='curve');md.line(q,fill='#fff9e9',width=10*S,joint='curve')
 points.extend(p)
stops=[('大涌口',114.260106,22.372007),('白沙臺',114.260133,22.367542),('白沙灣',114.259413,22.364778)]
for i,(name,lon,lat) in enumerate(stops):
 x,y=project(lon,lat);r=6 if i==2 else 4
 md.ellipse(((x-r)*S,(y-r)*S,(x+r)*S,(y+r)*S),fill=RED if i==2 else '#fff9e9',outline='#596e5d',width=S)
 text(mp,(x+13,y-7),name,15 if i==2 else 13)
text(mp,(151,0),'北',11);text(mp,(151,15),'↑',16);text(mp,(9,56),'↓',18,RED);text(mp,(113,137),'位置示範',11)
im.paste(mp,(134*S,35*S));rect(im,(12,204,308,232),INK,8);text(im,(160,210),'按一下試播到站動畫',15,BG,anchor='mt')
im=im.resize((320,240),Image.Resampling.LANCZOS);im.save(root/'assets/design-base.png')
header=['#pragma once','#include <Arduino.h>']
def array(name,img):
 rgb=list(img.getdata()); vals=[((r>>3)<<11)|((g>>2)<<5)|(b>>3) for r,g,b in rgb]
 header.append('const uint16_t '+name+'[] PROGMEM = {')
 for n in range(0,len(vals),24):header.append(','.join(hex(v) for v in vals[n:n+24])+',')
 header.append('};')
array('uiBackground',im)
for n in range(9):
 digit=make(110,67);text(digit,(55,3),str(n),67,latin=True,anchor='mt');array('digit'+str(n),digit.resize((110,67),Image.Resampling.LANCZOS))
header.append('const uint16_t* const uiDigits[] = {'+','.join('digit'+str(n) for n in range(9))+'};')
y0=project(stops[0][1],stops[0][2])[1];y1=project(stops[-1][1],stops[-1][2])[1]
p=sorted((x,y) for x,y in points if y0<=y<=y1 and abs(x-52)<35)
p.sort(key=lambda t:t[1]);assert len(p)>3
header.append('const int16_t busPath[][2] = {'+','.join('{'+str(round(x+134))+','+str(round(y+35))+'}' for x,y in p)+'};')
header.append('constexpr size_t busPathCount = sizeof(busPath)/sizeof(busPath[0]);')
(root/'src/ui_assets.h').write_text('\n'.join(header)+'\n')
# Preview includes the dynamic number and bus marker.
preview=im.copy();dig=make(110,67);text(dig,(55,3),'8',67,latin=True,anchor='mt');preview.paste(dig.resize((110,67),Image.Resampling.LANCZOS),(11,113));preview.save(root/'assets/design-preview.png')
print('UI assets generated; path points',len(p))
