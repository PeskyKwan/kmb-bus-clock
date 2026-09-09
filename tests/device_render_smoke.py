"""USB render test: configure a downloaded-road route such as2A with a fresh ETA first. No Save action is used."""
import argparse,json,time
import serial
p=argparse.ArgumentParser();p.add_argument('--port',required=True);a=p.parse_args();s=serial.Serial(port=None,baudrate=115200,timeout=.3);s.port=a.port;s.dtr=False;s.rts=False;s.open()
def cmd(obj,event='state'):
 s.write((json.dumps(obj)+'\n').encode());end=time.monotonic()+8
 while time.monotonic()<end:
  raw=s.readline()
  try:d=json.loads(raw)
  except (ValueError,UnicodeError):
   line=raw.decode(errors='replace').strip()
   if any(k in line for k in ('Guru','Backtrace','assert','panic')):print(line,flush=True)
   continue
  if d.get('event')==event:return d
 raise TimeoutError(event)
def state():return cmd({'cmd':'state'})
def ui(action,**kwargs):return cmd({'cmd':'settings','action':action,**kwargs},'settings')
try:
 end=time.monotonic()+90
 while True:
  d=state()
  if 'displayReady' in d:assert d['displayReady'],'framebuffer allocation failed'
  print({k:d.get(k) for k in ['connected','etaCode','roadReady','displayReady','apiDiagnostic','roadDiagnostic','heap','largestHeap']},flush=True)
  if d.get('connected') and d.get('etaCode')==2 and d.get('roadReady'):break
  if time.monotonic()>end:raise RuntimeError({k:d.get(k) for k in ['connected','etaCode','roadReady','roadPoints','heap']})
  time.sleep(1)
 assert 0<=d['mapMeters']<=3001 and d['mapStops']>=1
 print('LIVE ROAD / DISPLAY PASS', {k:d.get(k) for k in ['route','roadPoints','mapStops','mapMeters','heap','displayReady']},flush=True)
 ui('open');ui('touch',x=50,y=80);ui('touch',x=50,y=215);time.sleep(.5)
 before=state();ui('touch',x=310,y=42);time.sleep(.5);after=state();delta=after['sentTiles']-before['sentTiles'];assert delta==0,delta
 print('Blank tap:0 changed tiles PASS',flush=True)
 before=after;ui('touch',x=50,y=89);time.sleep(.5);after=state();delta=after['sentTiles']-before['sentTiles'];assert 0<delta<20,delta
 print('Key entry changed tiles:',delta,'of300 PASS',flush=True)
 ui('close');time.sleep(.5);before=state();time.sleep(20);after=state();delta=after['sentTiles']-before['sentTiles'];assert delta<300,delta
 print('Idle animation over20s:',delta,'tiles; no full-frame refresh PASS',flush=True)
finally:
 try:ui('close')
 except Exception:pass
 s.close()
