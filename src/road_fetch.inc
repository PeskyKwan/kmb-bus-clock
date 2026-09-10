#include "route_geometry.h"
#include "road_simplify.h"
#include "map_sources.h"
#include "map_ca.h"
#include <vector>
#include "stop_catalog.h"
bool api(const String&,DynamicJsonDocument&);
RoadData pendingRoad,activeRoad;
uint32_t roadTriedGen=0;volatile int roadDiagnostic=0;
bool downloadRoad(uint32_t id,char bound,std::vector<GeoPoint>&points){
 WiFiClientSecure tls;tls.setCACert(mapCA);HTTPClient h;h.setConnectTimeout(6000);h.setTimeout(7000);h.useHTTP10(true);
 String url="https://hkbus.github.io/route-waypoints/"+String(id)+"-"+String(bound)+".json";
 if(!h.begin(tls,url)){roadDiagnostic=-100;return false;}int httpCode=h.GET();if(httpCode!=200){char errorText[1];int tlsCode=tls.lastError(errorText,sizeof(errorText));roadDiagnostic=tlsCode?tlsCode:httpCode;h.end();return false;}auto&stream=h.getStream();stream.setTimeout(7000);
 if(!stream.find("\"coordinates\"")){h.end();return false;}
 RoadSimplifier simplify(points);int depth=0,used=0,field=0;char token[32];float lon=0;bool done=false,bad=false;unsigned long start=millis();int bytes=0;
 while(millis()-start<25000){int ch=stream.read();if(ch<0){if(!h.connected()&&!stream.available())break;delay(1);continue;}if(++bytes>512000){bad=true;break;}if(bytes%256==0)delay(1);
 if(ch=='['){depth++;if(depth==3){used=0;field=0;}continue;}
 if(depth==3&&(ch==','||ch==']')){token[used]=0;char*end;float value=strtof(token,&end);if(end==token||*end||!std::isfinite(value)){bad=true;break;}used=0;if(ch==','){if(field++){bad=true;break;}lon=value;}else{if(field!=1||lon<113||lon>115||value<22||value>23){bad=true;break;}if(!simplify.feed({lon,value})){bad=true;break;}}}
 else if(depth==3&&ch!=' '&&ch!='\n'&&ch!='\r'&&ch!='\t'){if(used>=31){bad=true;break;}token[used++]=(char)ch;}
 if(ch==']'){depth--;if(depth==0){done=true;break;}}
 }
 h.end();return done&&!bad&&simplify.finish()&&points.size()>1;
}
bool cachedStop(const char*id,WindowStop&stop){uint64_t key=strtoull(id,nullptr,16);int l=0,r=stopLabelCount-1;while(l<=r){int m=(l+r)/2;StopLabelEntry entry;memcpy_P(&entry,stopLabels+m,sizeof(entry));if(entry.id==key){stop.point={(float)(entry.lng/1e6),(float)(entry.lat/1e6)};stop.label=entry.offset;return true;}if(entry.id<key)l=m+1;else r=m-1;}return false;}
struct RoadStopCandidate{WindowStop stop;int seq;};
std::vector<RoadStopCandidate> cachedRoadStops;uint32_t roadStopsGen=0;unsigned long roadAttemptAt=0;
void cacheRoadStops(const Config&c,JsonArray rows){cachedRoadStops.clear();for(JsonObject row:rows){int seq=atoi(row["seq"]|"0");if(seq<1||seq>=c.seq)continue;WindowStop stop;if(cachedStop(row["stop"]|"",stop))cachedRoadStops.push_back({stop,seq});}std::sort(cachedRoadStops.begin(),cachedRoadStops.end(),[](const RoadStopCandidate&a,const RoadStopCandidate&b){return a.seq<b.seq;});roadStopsGen=c.gen;}
bool prepareRoad(const Config&c){pendingRoad=RoadData{};pendingRoad.gen=c.gen;if(roadStopsGen!=c.gen){roadDiagnostic=10;return false;}roadDiagnostic=20;GeoPoint anchors[3];for(int i=0;i<c.count;i++)anchors[i]={(float)c.stops[i].lng,(float)c.stops[i].lat};int attempted=0;
 for(int pass=0;pass<2;pass++)for(int i=0;i<mapSourceCount&&attempted<4;i++){MapSource source;memcpy_P(&source,mapSources+i,sizeof(source));if(strcmp(source.route,c.route)||source.bound!=c.bound)continue;bool preferred=c.service==1?source.special==0:source.special!=0;if((pass==0)!=preferred)continue;attempted++;std::vector<GeoPoint> points;/* Grow after TLS connects; eager 16KB reserve can starve the handshake. */if(!downloadRoad(source.id,c.bound,points))continue;points.shrink_to_fit();int end;if(!matchRoadEnd(points.data(),points.size(),anchors,c.count,end)){roadDiagnostic=30;continue;}RoadWindow window=upstreamWindow(points.data(),end,3000);pendingRoad.leadSeq=0;pendingRoad.leadFraction=0;WindowStop visible[maxMapStops];int used=0,lastMatched=0;
 for(const auto&ref:cachedRoadStops){auto stop=ref.stop;float best=1e9;int index=-1;for(int j=lastMatched;j<=end;j++){float dist=geoDistance2(points[j],stop.point);if(dist<best){best=dist;index=j;}}if(index<0||geoMeters(points[index],stop.point)>150)continue;lastMatched=index;if(index<window.first){pendingRoad.leadSeq=ref.seq;pendingRoad.leadFraction=windowFraction(points.data(),window,index);continue;}stop.seq=ref.seq;stop.fraction=windowFraction(points.data(),window,index);if(used>=maxMapStops-1)return false;visible[used++]=stop;}
 visible[used++]={anchors[c.count-1],0xffffffffu,c.seq,1};if(projectWindow(points.data(),window,visible,used,pendingRoad)){roadDiagnostic=0;return true;}roadDiagnostic=50;}
 return false;
}
