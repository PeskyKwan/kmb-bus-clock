"""Optional USB smoke test. Exercises native hit targets; never presses Save."""
import argparse,json,time
import serial
p=argparse.ArgumentParser();p.add_argument('--port',required=True);args=p.parse_args()
s=serial.Serial(port=None,baudrate=115200,timeout=.3);s.port=args.port;s.dtr=False;s.rts=False;s.open()
def command(obj,event='settings'):
 s.write((json.dumps(obj)+'\n').encode());end=time.monotonic()+20
 while time.monotonic()<end:
  raw=s.readline()
  try:reply=json.loads(raw)
  except (ValueError,UnicodeError):
   line=raw.decode(errors="replace").strip()
   if line and any(t in line for t in ("Guru", "Backtrace", "assert", "watchdog", "panic", "Reboot", "rst:")):print(line,flush=True)
   continue
  if reply.get('event')==event:return reply
 raise TimeoutError(event)
def ui(action='status',**kw):return command({'cmd':'settings','action':action,**kw})
def tap(x,y):return ui('touch',x=x,y=y)
def wait_page(page,timeout=45):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  r=ui()
  if r['page']==page and not r['busy']:return r
  if r.get('error') and r['page']!=7:raise RuntimeError(r['error'])
  time.sleep(.5)
 raise TimeoutError('native page '+str(page))
try:
 end=time.monotonic()+50
 while True:
  baseline=command({'cmd':'state'},'state')
  if baseline.get('connected') and baseline.get('etaCode') in (1,2):break
  if time.monotonic()>end:raise TimeoutError('Wi-Fi/ETA ready')
  time.sleep(2)
 ui('open');tap(50,80);tap(50,80);tap(50,215);tap(261,149);r=tap(160,89);assert r['input']=='92',r
 print('Keypad input92 PASS',flush=True)
 tap(200,215);r=wait_page(2);assert r['variants']>=2,r
 print('Live direction/service catalog PASS:',r['variants'],flush=True)
 tap(50,67);r=wait_page(3);assert r['stops']>=8,r
 print('Live stop list PASS:',r['stops'],flush=True)
 tap(270,215);r=tap(270,215);assert r['offset']==6,r
 tap(50,115);r=wait_page(8);assert r['draftRoute']=='92' and r['draftStop']=='5089C69E080B7A43',r
 print('Paged stop selection + live metadata PASS',flush=True)
 ui('close');after=command({'cmd':'state'},'state')
 for key in ['route','bound','service','stop','threshold','brightness']:
  assert after[key]==baseline[key],key
 print('Unsaved draft did not change active config PASS',flush=True)
finally:
 try:ui('close')
 except Exception:pass
 s.close()
