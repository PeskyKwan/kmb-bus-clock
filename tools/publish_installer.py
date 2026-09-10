"""Publish verified static installer to the independent repository's gh-pages."""
from pathlib import Path
import subprocess,tempfile,shutil
r=Path(__file__).resolve().parents[1]
subprocess.run(['node','tests/installer_artifact_test.mjs'],cwd=r,check=True)
remote='https://github.com/PeskyKwan/kmb-bus-clock.git'
existing=subprocess.run(['git','ls-remote',remote,'refs/heads/gh-pages'],capture_output=True,text=True,check=True).stdout.strip()
with tempfile.TemporaryDirectory(prefix='kmb-clock-pages-') as tmp:
 path=Path(tmp)
 if existing:subprocess.run(['git','clone','--depth','1','--branch','gh-pages',remote,tmp],check=True)
 else:subprocess.run(['git','init','-b','gh-pages',tmp],check=True)
 # Only overwrite this installer's own generated website files.
 for name in ('index.html','style.css','app.js','app.js.LEGAL.txt','manifest.json'):
  src=r/'installer'/name
  if src.exists():shutil.copy2(src,path/name)
 for name in ('firmware','licenses'):shutil.copytree(r/'installer'/name,path/name,dirs_exist_ok=True)
 (path/'.nojekyll').touch()
 for key,value in [('user.name','PeskyKwan'),('user.email','PeskyKwan@users.noreply.github.com')]:subprocess.run(['git','config',key,value],cwd=path,check=True)
 subprocess.run(['git','add','.'],cwd=path,check=True)
 if subprocess.run(['git','diff','--cached','--quiet'],cwd=path).returncode:
  subprocess.run(['git','commit','-m','Publish KMB Bus Clock installer 0.2.1'],cwd=path,check=True)
  subprocess.run(['git','-c','credential.helper=','-c','credential.helper=!gh auth git-credential','push',remote,'HEAD:gh-pages'],cwd=path,check=True)
print('Static installer pushed; enable/verify GitHub Pages separately.')
