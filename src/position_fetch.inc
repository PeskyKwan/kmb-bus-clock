#include "position_decode.h"
EtaFrame pendingFrame;
EtaLayout positionLayout(const RoadData&road){EtaLayout out;out.meters=road.meters;if(road.leadSeq){out.anchors[out.count++]={road.leadSeq,road.leadFraction};}const int room=positionAnchors-out.count,n=std::min(room,road.stopCount);for(int j=0;j<n;j++){int i=n>1?(int)std::lround(j*(road.stopCount-1)/float(n-1)):road.stopCount-1;if(road.stopSeq[i]>0)out.anchors[out.count++]={road.stopSeq[i],road.stopFractions[i]};}return out;}
// Bounded streaming row parser: never retain the whole route response during TLS.
class PositionStream:public Stream{
 Stream&source;unsigned long started;int used=0,held=-1;
 public:explicit PositionStream(Stream&s):source(s),started(millis()){setTimeout(1500);}
 bool allowed(){return used<262144&&millis()-started<25000;}
 void unread(int c){held=c;}
 int available()override{return allowed()?source.available()+(held>=0):0;}
 int read()override{if(!allowed())return -1;if(held>=0){int c=held;held=-1;return c;}int c=source.read();if(c>=0)used++;return c;}
 int peek()override{return allowed()?(held>=0?held:source.peek()):-1;}
 void flush()override{}
 size_t write(uint8_t)override{return 0;}
 int token(){char c;while(allowed()&&readBytes(&c,1)==1){if(c!=' '&&c!='\r'&&c!='\n'&&c!='\t')return (unsigned char)c;}return -1;}
};
bool fetchPosition(const Request&req){
 pendingFrame=EtaFrame{};pendingFrame.layout=req.layout;
 WiFiClientSecure tls;tls.setCACert(kmbCA);tls.setHandshakeTimeout(10);HTTPClient http;http.setConnectTimeout(5000);http.setTimeout(5000);http.useHTTP10(true);
 if(!http.begin(tls,"https://data.etabus.gov.hk/v1/transport/kmb/route-eta/"+String(req.c.route)+"/"+String(req.c.service)))return false;
 if(http.GET()!=200||http.getSize()>262144){http.end();return false;}
 PositionStream stream(http.getStream());bool ok=decodePosition(stream,pendingFrame,req.c.route,req.c.bound,req.c.service,req.c.destination,time(nullptr));
 http.end();return ok;
}
