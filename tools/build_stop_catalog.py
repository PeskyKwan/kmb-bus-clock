"""Compact, read-only stop labels for native search. Live selection is revalidated."""
import json
from pathlib import Path
r=Path(__file__).resolve().parents[1]
rows=sorted(json.loads((r/'docs/stops.json').read_text())['data'],key=lambda s:s['stop']);names=bytearray();entries=[]
for s in rows:
 off=len(names);names.extend(s['name_tc'].encode()+b'\0');entries.append((int(s['stop'],16),off))
out=['#pragma once','#include <Arduino.h>','struct StopLabelEntry{uint64_t id;uint32_t offset;};','const StopLabelEntry stopLabels[] PROGMEM={']
out.extend('{0x%016xULL,%d},'%e for e in entries);out.append('};\nconst uint8_t stopLabelText[] PROGMEM={')
for i in range(0,len(names),40):out.append(','.join(map(str,names[i:i+40]))+',')
out.append('};\nconstexpr int stopLabelCount=sizeof(stopLabels)/sizeof(stopLabels[0]);')
(r/'src/stop_catalog.h').write_text('\n'.join(out));print('Native stop labels:',len(rows))
