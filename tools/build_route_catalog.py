"""Generate a compact route/destination index; selected stops are verified live."""
import json
from pathlib import Path
r=Path(__file__).resolve().parents[1]
d=json.loads((r/'docs/routes.json').read_text())
indexed={(x['route'],x['bound'],int(x['service_type'])):x['dest_tc'] for x in d['data']}
rows=sorted(((route,bound,service,destination) for (route,bound,service),destination in indexed.items()),key=lambda x:(x[0],x[1]!='O',x[2]))
assert all(len(x[0])<=6 and x[1] in ('O','I') and 1<=x[2]<=50 for x in rows)
blob=bytearray();offsets={}
for *_,destination in rows:
 if destination not in offsets:offsets[destination]=len(blob);blob.extend(destination.encode());blob.append(0)
out=['#pragma once','#include <Arduino.h>','struct RouteCandidate{char route[7];char bound;uint8_t service;uint32_t destination;};','const RouteCandidate routeCandidates[] PROGMEM={']
out.extend('{"%s",\'%s\',%d,%d},'%(route,bound,service,offsets[destination]) for route,bound,service,destination in rows);out.append('};\nconstexpr int routeCandidateCount=sizeof(routeCandidates)/sizeof(routeCandidates[0]);')
out.append('const char routeDestinationText[] PROGMEM={')
for i in range(0,len(blob),40):out.append(','.join(map(str,blob[i:i+40]))+',')
out.append('};')
(r/'src/route_catalog.h').write_text('\n'.join(out));print(len(rows),'route/service candidates')
