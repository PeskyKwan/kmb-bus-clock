"""USB appearance QA; previews themes without saving, restores main UI."""
import argparse,json,time
from pathlib import Path
import serial
p=argparse.ArgumentParser();p.add_argument('--port',required=True);args=p.parse_args()
s=serial.Serial(port=None,baudrate=115200,timeout=.5);s.port=args.port;s.dtr=False;s.rts=False;s.open();time.sleep(10);s.reset_input_buffer()
def command(o,event='settings'):
 s.reset_input_buffer()
 s.write(('\n'+json.dumps(o)+'\n').encode());s.flush();end=time.monotonic()+45
 while time.monotonic()<end:
  try:r=json.loads(s.readline())
  except (ValueError,UnicodeError):continue
  if r.get('event')==event:return r
 raise TimeoutError(event)
def ui(action='status',**kw):return command({'cmd':'settings','action':action,**kw})
def tap(x,y):return ui('touch',x=x,y=y)
def capture(name):
 s.write(b'{"cmd":"settings","action":"capture"}\n');rows={};end=time.monotonic()+50
 while len(rows)<240 and time.monotonic()<end:
  raw=s.readline().decode(errors='replace').strip()
  if not raw.startswith('PIX '):continue
  _,y,data=raw.split(' ',2);assert len(data)==1280
  rgb=bytearray()
  for i in range(0,1280,4):
   c=int(data[i:i+4],16);rgb.extend(((c>>11&31)*255//31,(c>>5&63)*255//63,(c&31)*255//31))
  rows[int(y)]=rgb
 assert len(rows)==240,len(rows)
 if name in ('day','night'):
  red=sum(1 for y in range(1,121) for x in range(6,126) if rows[y][x*3]>150 and rows[y][x*3+1]<130 and rows[y][x*3+2]<130)
  assert red>1000,('station plate missing',red)
 while time.monotonic()<end:
  try:reply=json.loads(s.readline())
  except (ValueError,UnicodeError):continue
  if reply.get('event')=='settings':break
 Path('/tmp/clock-'+name+'.ppm').write_bytes(b'P6\n320 240\n255\n'+b''.join(rows[y] for y in range(240)))
 print(name+' framebuffer captured',flush=True)
try:
 base=command({'cmd':'state'},'state')
 assert base['displayReady']
 print('Display ready; live connectivity:',base['connected'],'clock synchronized:',base['clockSynced'],flush=True)
 ui('open');tap(50,80);tap(50,80);tap(50,215);tap(261,149);r=tap(160,89);assert r['input']=='92' and r['letters']=='R',r
 tap(50,179);capture('keypad')
 ui('close');ui('open');tap(60,150);tap(60,60);r=tap(60,158);assert r['nightMode'] and r['themeMode']==2
 ui('preview');time.sleep(.3);capture('night')
 ui('close');ui('open');tap(60,150);tap(60,60);r=tap(60,109);assert not r['nightMode'] and r['themeMode']==1
 ui('preview');time.sleep(.3);capture('day')
 ui('close');after=command({'cmd':'state'},'state')
 for k in ('route','bound','service','stop','brightness','themeMode'):assert after[k]==base[k],k
 print('Theme previews / cancel / preserved config PASS',flush=True)
finally:
 try:ui('close')
 except Exception:pass
 s.close()
