"""USB route-chooser regression: 1A search and both directions, never saves."""
import argparse,time
from device_position_audit import NoResetSerial

p=argparse.ArgumentParser();p.add_argument('--port',required=True);args=p.parse_args()

def exchange(payload,event):
 # A CH340 handle can become invalid between commands. Open only for each
 # exchange, without DTR/RTS resets; cleanup gets its own fresh handle too.
 serial=NoResetSerial(args.port)
 try:return serial.command(payload,event,timeout=20)
 finally:serial.close()

def command(action='status',**fields):
 payload={'cmd':'settings','action':action};payload.update(fields)
 return exchange(payload,'settings')

def wait_ready():
 end=time.monotonic()+60
 while time.monotonic()<end:
  r=exchange({'cmd':'state'},'state')
  if r.get('connected') and r.get('etaCode') in (1,2):return
  time.sleep(.5)
 raise TimeoutError('Wi-Fi')

def touch(x,y):return command('touch',x=x,y=y)
def settled(page,timeout=45):
 end=time.monotonic()+timeout;last=None
 while time.monotonic()<end:
  last=command()
  if not last['busy']:
   assert last['page']==page,last
   return last
  time.sleep(.4)
 raise TimeoutError(last)

try:
 wait_ready();time.sleep(5)
 for attempt in range(3):
  try:command('open');break
  except TimeoutError:
   if attempt==2:raise
 touch(50,80);touch(50,80);touch(50,215);touch(50,89);touch(50,179);r=touch(50,89);assert r['input']=='1A',r
 touch(200,215);r=settled(2);assert r['variants']>=2,r
 for choice in range(2):
  touch(50,67+choice*48);r=settled(3);assert r['stops']>0,r
  if choice==0:touch(50,20)
 print('1A both destinations reach stop selection without saving PASS',flush=True)
finally:
 try:command('close')
 except Exception:pass
