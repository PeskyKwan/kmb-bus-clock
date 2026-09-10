"""USB route-chooser regression: 1A search and both directions, never saves."""
import argparse,json,time,os,select,termios,tty

p=argparse.ArgumentParser();p.add_argument('--port',required=True);args=p.parse_args()
fd=os.open(args.port,os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK);tty.setraw(fd);attrs=termios.tcgetattr(fd);attrs[4]=termios.B115200;attrs[5]=termios.B115200;termios.tcsetattr(fd,termios.TCSANOW,attrs);rx=b''

def write(data):os.write(fd,data)
def readline(timeout=.4):
 global rx
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  if b'\n' in rx:
   line,rx=rx.split(b'\n',1);return line
  ready,_,_=select.select([fd],[],[],min(.2,end-time.monotonic()))
  if ready:
   try:rx+=os.read(fd,8192)
   except BlockingIOError:pass
 return b''

def command(action='status',**fields):
 payload={'cmd':'settings','action':action};payload.update(fields);write((json.dumps(payload)+'\n').encode());end=time.monotonic()+20
 while time.monotonic()<end:
  raw=readline()
  try:r=json.loads(raw)
  except (ValueError,UnicodeError):
   line=raw.decode(errors='replace').strip()
   if line.startswith('NATIVE_ERROR'):print(line,flush=True)
   continue
  if r.get('event')=='settings':return r
 raise TimeoutError(action)

def wait_ready():
 end=time.monotonic()+60
 while time.monotonic()<end:
  write(b'{"cmd":"state"}\n');until=time.monotonic()+5
  while time.monotonic()<until:
   try:r=json.loads(readline())
   except (ValueError,UnicodeError):continue
   if r.get('event')=='state' and r.get('connected') and r.get('etaCode') in (1,2):return
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
 os.close(fd)
