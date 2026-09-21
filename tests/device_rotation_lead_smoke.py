"""Exercise rotation and ETA-lead settings; restore normal/90s before exit."""
import argparse,json,time
import serial

p=argparse.ArgumentParser();p.add_argument('--port',required=True);a=p.parse_args()
s=serial.Serial(port=None,baudrate=115200,timeout=.5);s.port=a.port;s.dtr=False;s.rts=False;s.open();time.sleep(3)

def command(obj,event='settings'):
 s.reset_input_buffer();s.write(('\n'+json.dumps(obj)+'\n').encode());end=time.monotonic()+20
 while time.monotonic()<end:
  try:r=json.loads(s.readline())
  except (ValueError,UnicodeError):continue
  if r.get('event')==event:
   time.sleep(.2);return r
 raise TimeoutError(event)
def ui(action='status',**kw):return command({'cmd':'settings','action':action,**kw})
def tap(x,y):return ui('touch',x=x,y=y)
def state():return command({'cmd':'state'},'state')

baseline=state();assert baseline['version'] in ('0.3.0','0.3.1')
try:
 r=ui('open');assert r['page']==0,r
 r=tap(50,150);assert r['page']==9,r
 r=tap(50,139);assert r['page']==15,r
 before=state()['sentTiles'];r=tap(50,139);assert r['screenFlipped'] and r['draftFlipped'],r
 time.sleep(.8);flipped=state();assert flipped['screenFlipped'] and flipped['sentTiles']-before>=300,flipped
 before=flipped['sentTiles'];r=tap(50,79);assert not r['screenFlipped'] and not r['draftFlipped'],r
 time.sleep(.8);normal=state();assert not normal['screenFlipped'] and normal['sentTiles']-before>=300,normal
 tap(25,18);tap(25,18);r=tap(25,18);assert not r['open'],r
 print('Normal -> 180-degree -> normal full-frame retransmit PASS',flush=True)

 r=ui('open');assert r['page']==0,r
 r=tap(50,80);assert r['page']==8,r
 r=tap(50,166);assert r['page']==14 and r['leadSeconds']==90,r
 r=tap(40,80);assert r['leadSeconds']==60,r
 r=tap(270,80);assert r['leadSeconds']==90,r
 r=tap(160,216);assert r['page']==8,r
 r=tap(160,216);assert r['saved'],r
 ui('close')
 after=state();assert after['leadSeconds']==90 and not after['screenFlipped']
 for key in ('route','bound','service','stop','threshold','brightness'):
  assert after[key]==baseline[key],key
 print('0/30/60/90 safety-lead controls + save PASS',flush=True)
finally:
 try:
  if state().get('screenFlipped'):
   ui('open');tap(50,150);tap(50,139);tap(50,79);ui('close')
  else:ui('close')
 except Exception:pass
 s.close()
