"""Exercise device Wi-Fi UI without printing credentials or network names."""
import argparse,json,time,uuid
import serial
p=argparse.ArgumentParser();p.add_argument('--port',required=True);args=p.parse_args()
s=serial.Serial(port=None,baudrate=115200,timeout=2);s.port=args.port;s.dtr=False;s.rts=False;s.open();time.sleep(10);seq=0

def call(data,event):
 global seq
 seq+=1
 if event=='wifi':data['id']=str(seq)
 s.reset_input_buffer();s.write(('\n'+json.dumps(data)+'\n').encode());end=time.monotonic()+20
 pending=b'';retried=False
 while time.monotonic()<end:
  raw=s.readline();pending+=raw
  if not pending.endswith(b'\n'):continue
  try:r=json.loads(pending.decode(errors='replace'))
  except (ValueError,UnicodeError):pending=b'';continue
  pending=b''
  if isinstance(r,dict) and r.get('event')==event and (event!='wifi' or r.get('id')==str(seq)):
   time.sleep(.2);return r
  if time.monotonic()>end-5 and not retried:
   query={'cmd':'wifi','action':'status','id':str(seq)} if event=='wifi' else {'cmd':'settings','action':'status'} if event=='settings' else {'cmd':'state'}
   s.write(('\n'+json.dumps(query)+'\n').encode());retried=True
 raise TimeoutError(event)
def wifi(action='status',**kw):return call({'cmd':'wifi','action':action,**kw},'wifi')
def ui(action='status',**kw):return call({'cmd':'settings','action':action,**kw},'settings')
def tap(x,y):return ui('touch',x=x,y=y)
def state():return call({'cmd':'state'},'state')
def wait_wifi(seconds=65):
 end=time.monotonic()+seconds
 while time.monotonic()<end:
  r=wifi()
  if r['stage']==0:return r
  time.sleep(.5)
 raise TimeoutError('Wi-Fi operation')
try:
 base=state();assert base.get('app')=='kmb-bus-clock'
 ui('open');tap(50,150);r=tap(50,166);assert r['page']==6
 r=wait_wifi();assert len(r['networks'])>0,{'message':r['message'],'diagnostic':r.get('diagnostic')}
 networks=r['networks'];print('Native Wi-Fi scan PASS:',len(networks),'networks',flush=True)
 tap(100,215);assert ui()['page']==11
 alphabet='qwertyuiopasdfghjklzxcvbnm-_. '
 for c in 'kmb-test-no-network':
  i=alphabet.index(c);tap(20+i%10*31,109+i//10*30)
 tap(260,211);assert ui()['page']==12
 tap(25,212) # upper
 i='QWERTYUIOPASDFGHJKLZXCVBNM-_. '.index('A');tap(20+i%10*31,109+i//10*30)
 tap(80,212) # numbers
 for i in (8,10):tap(20+i%10*31,109+i//10*30) # 9!
 tap(25,212) # lower
 for c in 'abcde':
  i=alphabet.index(c);tap(20+i%10*31,109+i//10*30)
 r=tap(260,211);assert r['page']==13
 # Wait for a failed connection; previous saved network must remain intact.
 r=wait_wifi();assert r['message']=='連線失敗',r['message']
 after=state();assert after['ssid']==base['ssid'];assert after['route']==base['route']
 print('Case/digits/symbol password UI + failure preserves network/route PASS',flush=True)
 ui('close')
 if base['ssid'] and any(n['ssid']==base['ssid'] for n in networks):
  wifi('connect',ssid=base['ssid'],password='');r=wait_wifi()
  assert r['connected'] and r['message']=='已連線',r['message']
  print('Successful saved-network reconnect + commit PASS',flush=True)
 else:print('Saved network not in scan; positive connection needs user network test',flush=True)
finally:
 try:wifi('cancel');ui('close')
 except Exception:pass
 s.close()
