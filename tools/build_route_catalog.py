"""Generate a compact candidate index; firmware verifies each candidate live."""
import json
from pathlib import Path
r=Path(__file__).resolve().parents[1]
d=json.loads((r/'docs/routes.json').read_text())
rows=sorted({(x['route'],x['bound'],int(x['service_type'])) for x in d['data']},key=lambda x:(x[0],x[1]!='O',x[2]))
assert all(len(x[0])<=6 and x[1] in ('O','I') and 1<=x[2]<=50 for x in rows)
out=['#pragma once','#include <Arduino.h>','struct RouteCandidate{char route[7];char bound;uint8_t service;};','const RouteCandidate routeCandidates[] PROGMEM={']
out.extend('{"%s",\'%s\',%d},'%x for x in rows);out.append('};\nconstexpr int routeCandidateCount=sizeof(routeCandidates)/sizeof(routeCandidates[0]);')
(r/'src/route_catalog.h').write_text('\n'.join(out));print(len(rows),'route/service candidates')
