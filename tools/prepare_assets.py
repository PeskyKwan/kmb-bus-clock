"""Rebuild local raster assets and embedded setup page; does not contact a board."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[1]
(root/'assets').mkdir(exist_ok=True)
for script in ('build_ui_assets.py','build_smooth_font.py','build_stop_catalog.py','build_route_catalog.py','build_display_palette.py','build_map_sources.py'):
 subprocess.run([sys.executable,str(root/'tools'/script)],check=True)
html=(root/'setup/index.html').read_text()
assert ')PAGE"' not in html
(root/'src/setup_page.h').write_text('#pragma once\nconst char setupPage[] PROGMEM=R"PAGE('+html+')PAGE";\n')
print('Ready: run pio run. Nothing flashed.')
