from pathlib import Path
import json
r=Path(__file__).resolve().parents[1];data=json.loads((r/'docs/map-sources.json').read_text())['sources']
out=['#pragma once','#include <Arduino.h>','struct MapSource{char route[7];char bound;uint32_t id;uint8_t special;};','const MapSource mapSources[] PROGMEM={']
out.extend('{"%s",\'%s\',%d,%d},'%(a['route'],a['bound'],a['id'],a['special']) for a in data);out.append('};\nconstexpr int mapSourceCount=sizeof(mapSources)/sizeof(mapSources[0]);');(r/'src/map_sources.h').write_text('\n'.join(out))
