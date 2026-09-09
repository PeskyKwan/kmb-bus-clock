"""Tests mode selection persists without Save, including Back to main; restores original mode."""
import json,time,argparse
import serial
p=argparse.ArgumentParser();p.add_argument('--port',required=True);a=p.parse_args()
s=serial.Serial(port=None,baudrate=115200,timeout=.5);s.port=a.port;s.dtr=False;s.rts=False;s.open();time.sleep(10);s.reset_input_buffer()
def cmd(o,event='settings'):
 s.reset_input_buffer()
 s.write(('\n'+json.dumps(o)+'\n').encode());end=time.monotonic()+20
 while time.monotonic()<end:
  try:r=json.loads(s.readline())
  except (ValueError,UnicodeError):continue
  if r.get('event')==event:
   time.sleep(.2)
   return r
 # If an acknowledgement was lost, read the actual state; never repeat a tap.
 s.write((json.dumps({'cmd':'settings','action':'status'} if event=='settings' else {'cmd':'state'})+'\n').encode())
 end=time.monotonic()+10
 while time.monotonic()<end:
  try:r=json.loads(s.readline())
  except (ValueError,UnicodeError):continue
  if r.get('event')==event:
   time.sleep(.2)
   return r
 raise TimeoutError(event)
def ui(action='status',**kw):return cmd({'cmd':'settings','action':action,**kw})
def tap(x,y):return ui('touch',x=x,y=y)
def choose(mode):
 r=ui('open');assert r['page']==0,r
 r=tap(50,150);assert r['page']==9,r
 assert tap(50,60)['page']==10
 assert tap(50,60+49*mode)['themeMode']==mode
 assert tap(25,18)['page']==9
 assert tap(25,18)['page']==0
 assert not tap(25,18)['open']
 state=cmd({'cmd':'state'},'state');assert state['themeMode']==mode
 if mode:assert state['nightMode']==(mode==2)
baseline=cmd({'cmd':'state'},'state');original=baseline['themeMode']
try:
 choose(1 if original!=1 else 2)
 ui('open');assert ui()['themeMode']!=(original);ui('close')
 print('Mode tap, Back to main without Save, reopen PASS',flush=True)
finally:
 choose(original)
 end=cmd({'cmd':'state'},'state')
 for key in ('route','bound','service','stop','threshold','brightness','themeMode'):assert end[key]==baseline[key],key
 s.close()
 print('Original theme and bus configuration restored PASS',flush=True)
