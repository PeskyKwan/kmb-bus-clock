#include "route_geometry.h"
#include "map_sources.h"
#include "map_ca.h"
#include <vector>
RoadData pendingRoad,activeRoad;
uint32_t roadTriedGen=0;
bool downloadRoad(uint32_t id,char bound,std::vector<GeoPoint>&points){
 WiFiClientSecure tls;tls.setCACert(mapCA);HTTPClient h;h.setConnectTimeout(6000);h.setTimeout(7000);h.useHTTP10(true);
 String url="https://hkbus.github.io/route-waypoints/"+String(id)+"-"+String(bound)+".json";
 if(!h.begin(tls,url)||h.GET()!=200){h.end();return false;}auto&stream=h.getStream();stream.setTimeout(7000);
 if(!stream.find("\"coordinates\"")){h.end();return false;}
 int depth=0,used=0,field=0;char token[32];float lon=0;bool done=false,bad=false;unsigned long start=millis();int bytes=0;
 while(millis()-start<25000){int ch=stream.read();if(ch<0){if(!h.connected()&&!stream.available())break;delay(1);continue;}if(++bytes>512000){bad=true;break;}if(bytes%256==0)delay(1);
 if(ch=='['){depth++;if(depth==3){used=0;field=0;}continue;}
 if(depth==3&&(ch==','||ch==']')){token[used]=0;char*end;float value=strtof(token,&end);if(end==token||*end||!std::isfinite(value)){bad=true;break;}used=0;if(ch==','){if(field++){bad=true;break;}lon=value;}else{if(field!=1||lon<113||lon>115||value<22||value>23||points.size()>=2048){bad=true;break;}points.push_back({lon,value});}}
 else if(depth==3&&ch!=' '&&ch!='\n'&&ch!='\r'&&ch!='\t'){if(used>=31){bad=true;break;}token[used++]=(char)ch;}
 if(ch==']'){depth--;if(depth==0){done=true;break;}}
 }
 h.end();return done&&!bad&&points.size()>1;
}
bool prepareRoad(const Config&c){pendingRoad=RoadData{};pendingRoad.gen=c.gen;GeoPoint stops[3];for(int i=0;i<c.count;i++)stops[i]={(float)c.stops[i].lng,(float)c.stops[i].lat};int attempted=0;
 for(int pass=0;pass<2;pass++)for(int i=0;i<mapSourceCount&&attempted<4;i++){MapSource source;memcpy_P(&source,mapSources+i,sizeof(source));if(strcmp(source.route,c.route)||source.bound!=c.bound)continue;bool preferred=c.service==1?source.special==0:source.special!=0;if((pass==0)!=preferred)continue;attempted++;std::vector<GeoPoint> points;points.reserve(2048);if(downloadRoad(source.id,c.bound,points)&&fitRoad(points.data(),points.size(),stops,c.count,pendingRoad))return true;}
 return false;
}
