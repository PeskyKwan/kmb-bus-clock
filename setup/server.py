"""Local USB setup bridge; no credential files/logging, loopback only."""
import http.server,json,secrets,threading,time,urllib.parse
from pathlib import Path
import serial,requests,os,glob,sys
ROOT=Path(__file__).resolve().parent
PORT=8767
TOKEN=secrets.token_urlsafe(32)
state={'message':'正在連接 USB 巴士鬧鐘…'}
lock=threading.Lock();cache={}
link=serial.Serial(port=None,baudrate=115200,timeout=.5)
candidates=glob.glob('/dev/cu.usbserial*')+glob.glob('/dev/ttyUSB*')+glob.glob('/dev/ttyACM*')
selected=os.environ.get('KMB_SERIAL_PORT')
if not selected:
 if len(candidates)!=1:sys.exit('Set KMB_SERIAL_PORT to your board serial port (for example COM3 on Windows).')
 selected=candidates[0]
link.port=selected;link.dtr=False;link.rts=False;link.open()
def receive():
 global state
 while True:
  try:
   line=link.readline()
   if line.startswith(b'{'):
    obj=json.loads(line)
    if obj.get('event')=='state':state=obj
  except (ValueError,UnicodeError):pass
  except serial.SerialException:state={'message':'USB 已斷開，請重新接上後重啟設定工具。'};return
threading.Thread(target=receive,daemon=True).start()
class Handler(http.server.BaseHTTPRequestHandler):
 def log_message(self,*args):pass
 def allowed(self):return self.headers.get('Host') in (f'127.0.0.1:{PORT}',f'localhost:{PORT}')
 def respond(self,code,obj,typ='application/json'):
  b=(json.dumps(obj,ensure_ascii=False) if typ=='application/json' else obj).encode();self.send_response(code);self.send_header('Content-Type',typ+'; charset=utf-8');self.send_header('Cache-Control','no-store');self.send_header('X-Content-Type-Options','nosniff');self.end_headers();self.wfile.write(b)
 def do_GET(self):
  if not self.allowed():return self.respond(403,{'error':'Invalid host'})
  if self.path=='/':
   html=(ROOT/'index.html').read_text().replace('<script>',f'<script>window.SETUP_TOKEN={json.dumps(TOKEN)};</script><script>',1);return self.respond(200,html,'text/html')
  if self.path=='/state':return self.respond(200,state)
  if self.path.startswith('/kmb/'):
   path=self.path[5:]
   import re
   if not re.fullmatch(r'(route/?|stop/?|route-stop/[A-Z0-9]{1,6}/(inbound|outbound)/[0-9]{1,2})',path):return self.respond(400,{'error':'Invalid API path'})
   try:
    if path not in cache or time.time()-cache[path][0]>3600:
     r=requests.get('https://data.etabus.gov.hk/v1/transport/kmb/'+path,timeout=15);r.raise_for_status();cache[path]=(time.time(),r.json())
    return self.respond(200,cache[path][1])
   except requests.RequestException:return self.respond(502,{'error':'九巴資料暫時未能讀取'})
  self.respond(404,{'error':'Not found'})
 def do_POST(self):
  global state
  if not self.allowed() or self.headers.get('Origin') not in (f'http://127.0.0.1:{PORT}',f'http://localhost:{PORT}'):return self.respond(403,{'error':'Invalid origin'})
  if self.path!='/config':return self.respond(404,{'error':'Not found'})
  try:
   n=int(self.headers.get('Content-Length',0))
   if n<1 or n>4000:raise ValueError()
   obj=json.loads(self.rfile.read(n))
   if not secrets.compare_digest(str(obj.pop('key','')),TOKEN):return self.respond(403,{'error':'請重新開啟設定頁'})
   raw=json.dumps(obj,ensure_ascii=False,separators=(',',':')).encode()
   if len(raw)>3800:raise ValueError()
   with lock:link.write(raw+b'\n');link.flush()
   state={'message':'設定已送出，請等待板上確認及連線…'}
   self.respond(200,{'ok':True})
  except (ValueError,TypeError):self.respond(400,{'error':'設定格式不正確'})
  except serial.SerialException:self.respond(503,{'error':'USB 未連接'})
server=http.server.ThreadingHTTPServer(('127.0.0.1',PORT),Handler)
print(f'USB setup ready at http://127.0.0.1:{PORT}',flush=True)
try:server.serve_forever()
finally:link.close();server.server_close()
