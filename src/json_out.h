#pragma once
#include <Arduino.h>
#include <cmath>
// Streams one flat JSON object straight to a Print target. State telemetry is
// emitted every few seconds, often during HTTPS work, so it uses no heap document.
class JsonOut{
 Print&out;bool first=true;
 void key(const char*k){out.print(first?'{':',');first=false;out.print('"');out.print(k);out.print("\":");}
 public:
 explicit JsonOut(Print&p):out(p){}
 void put(const char*k,const char*v){key(k);out.print('"');for(const unsigned char*p=(const unsigned char*)(v?v:"");*p;p++){if(*p=='"'||*p=='\\'){out.print('\\');out.print((char)*p);}else if(*p<0x20)out.printf("\\u%04x",*p);else out.print((char)*p);}out.print('"');}
 void put(const char*k,const String&v){put(k,v.c_str());}
 void put(const char*k,bool v){key(k);out.print(v?"true":"false");}
 void put(const char*k,long long v){key(k);out.printf("%lld",v);}
 void put(const char*k,unsigned long long v){key(k);out.printf("%llu",v);}
 void put(const char*k,int v){put(k,(long long)v);}
 void put(const char*k,long v){put(k,(long long)v);}
 void put(const char*k,unsigned v){put(k,(unsigned long long)v);}
 void put(const char*k,unsigned long v){put(k,(unsigned long long)v);}
 void put(const char*k,double v){key(k);if(std::isfinite(v))out.printf("%.7g",v);else out.print("null");}
 void put(const char*k,float v){put(k,(double)v);}
 void end(){out.print(first?"{}":"}");first=true;}
};
