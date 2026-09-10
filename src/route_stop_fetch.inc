#include "route_stop_decode.h"
// Bounded route-stop parser: establish TLS before allocating one small row.
class RouteStopStream:public Stream{
 Stream&source;unsigned long started;int used=0,held=-1;
 public:explicit RouteStopStream(Stream&s):source(s),started(millis()){setTimeout(1500);}
 bool allowed(){return used<65536&&millis()-started<15000;}
 void unread(int c){held=c;}
 int available()override{return allowed()?source.available()+(held>=0):0;}
 int read()override{if(!allowed())return -1;if(held>=0){int c=held;held=-1;return c;}int c=source.read();if(c>=0)used++;return c;}
 int peek()override{return allowed()?(held>=0?held:source.peek()):-1;}
 void flush()override{}size_t write(uint8_t)override{return 0;}
 int token(){char c;while(allowed()&&readBytes(&c,1)==1)if(c!=' '&&c!='\r'&&c!='\n'&&c!='\t')return (unsigned char)c;return -1;}
};
template<class Sink>bool fetchRouteStops(const char*route,char bound,int service,Sink sink){
 for(int attempt=0;attempt<3;attempt++){WiFiClientSecure tls;tls.setCACert(kmbCA);tls.setHandshakeTimeout(12);tls.setTimeout(7);HTTPClient h;h.setConnectTimeout(6000);h.setTimeout(7000);h.useHTTP10(true);String dir=bound=='O'?"outbound":"inbound";if(!h.begin(tls,"https://data.etabus.gov.hk/v1/transport/kmb/route-stop/"+String(route)+"/"+dir+"/"+service))return false;int code=h.GET();char tlsError[1];int tlsCode=code<0?tls.lastError(tlsError,sizeof(tlsError)):0;apiTlsDiagnostic=tlsCode;if(code<0&&(code==-1||tlsCode==tlsConnectionEof)&&attempt<2){h.end();apiTlsRetries++;delay(500*(attempt+1));continue;}if(code!=200||h.getSize()>65536){apiDiagnostic=code;h.end();return false;}
  RouteStopStream input(h.getStream());int diagnostic=0;bool ok=decodeRouteStops(input,route,bound,service,sink,[](const char*id){return validID(id);},diagnostic);apiDiagnostic=diagnostic;h.end();if(ok){apiDiagnostic=0;return true;}return false;
 }
 return false;
}
