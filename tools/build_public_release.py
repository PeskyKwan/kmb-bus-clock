"""Build clean public firmware with pinned OFL fonts; never reads a device/NVS."""
import hashlib,json,os,subprocess,sys,urllib.request,shutil
from pathlib import Path
r=Path(__file__).resolve().parents[1]
for f in json.loads((r/'fonts/sources.json').read_text()):
 path=r/'fonts'/f['file']
 if not path.exists():urllib.request.urlretrieve(f['url'],path)
 if hashlib.sha256(path.read_bytes()).hexdigest()!=f['sha256']:raise SystemExit('Font hash mismatch: '+f['file'])
env=dict(os.environ,KMB_CJK_FONT=str(r/'fonts/NotoSansTC.ttf'),KMB_LATIN_FONT=str(r/'fonts/NotoSans.ttf'))
subprocess.run([sys.executable,str(r/'tools/prepare_assets.py')],env=env,cwd=r,check=True)
pio=str(r/'.venv/bin/pio') if (r/'.venv/bin/pio').exists() else 'pio'
subprocess.run([pio,'run'],cwd=r,env=env,check=True)
out=r/'installer/firmware';out.mkdir(exist_ok=True)
inputs=[('bootloader.bin',r/'.pio/build/board/bootloader.bin',4096),('partitions.bin',r/'.pio/build/board/partitions.bin',32768),('boot_app0.bin',r/'.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin',57344),('firmware.bin',r/'.pio/build/board/firmware.bin',65536)]
parts=[]
for name,src,offset in inputs:
 dst=out/name;shutil.copyfile(src,dst);data=dst.read_bytes();parts.append(dict(path='firmware/'+name,offset=offset,size=len(data),sha256=hashlib.sha256(data).hexdigest()))
assert parts[1]['offset']+parts[1]['size']<=0x9000
assert parts[-1]['size']<0x300000
manifest=dict(name='KMB Bus Clock',version='0.2.3',chip='ESP32',board='E32R28T-1',parts=parts)
(r/'installer/manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Public firmware prepared with OFL fonts. No device data read.')
