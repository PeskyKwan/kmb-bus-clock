"""Tests the normal Save button; restores the original theme and route config."""
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
  if r.get('event')==event:return r
 # If an acknowledgement was lost, read the actual state; never repeat a tap.
 s.write((json.dumps({'cmd':'settings','action':'status'} if event=='settings' else {'cmd':'state'})+'\n').encode())
 end=time.monotonic()+10
 while time.monotonic()<end:
  try:r=json.loads(s.readline())
  except (ValueError,UnicodeError):continue
  if r.get('event')==event:return r
 raise TimeoutError(event)
def ui(action='status',**kw):return cmd({'cmd':'settings','action':action,**kw})
def tap(x,y):return ui('touch',x=x,y=y)
def choose(mode):
 ui('open');assert tap(50,150)['page']==9
 assert tap(50,60)['page']==10
 assert tap(50,60+49*mode)['themeMode']==mode
 assert tap(50,215)['page']==9
 assert tap(50,215)['saved']
 ui('close')
 assert cmd({'cmd':'state'},'state')['themeMode']==mode
baseline=cmd({'cmd':'state'},'state');original=baseline['themeMode']
try:
 choose(1 if original!=1 else 2)
 ui('open');assert ui()['themeMode']!=(original);ui('close')
 print('Theme selection, Save, reopen PASS',flush=True)
finally:
 choose(original)
 end=cmd({'cmd':'state'},'state')
 for key in ('route','bound','service','stop','threshold','brightness','themeMode'):assert end[key]==baseline[key],key
 s.close()
 print('Original theme and bus configuration restored PASS',flush=True)
