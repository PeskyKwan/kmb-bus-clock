#include "../src/position_decode.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>
struct Input{std::string s;size_t at=0;int held=-1;bool allowed(){return at<s.size()||held>=0;}int read(){if(held>=0){int c=held;held=-1;return c;}return at<s.size()?(unsigned char)s[at++]:-1;}size_t readBytes(char*b,size_t n){size_t i=0;for(;i<n;i++){int c=read();if(c<0)break;b[i]=c;}return i;}int token(){int c;while((c=read())>=0)if(!isspace(c))return c;return -1;}void unread(int c){held=c;}};
int main(int argc,char**argv){setenv("TZ","UTC0",1);tzset();EtaFrame f;f.layout.count=3;for(int i=0;i<3;i++)f.layout.anchors[i]={i+6,i*.5f};f.layout.meters=600;
 std::string row="{\"route\":\"92\",\"dir\":\"O\",\"service_type\":1,\"seq\":8,\"eta_seq\":2,\"eta\":\"2026-09-10T16:07:54+08:00\",\"data_timestamp\":\"2026-09-10T16:01:30+08:00\",\"dest_tc\":\"D\",\"rmk_en\":\"Scheduled Bus\"}";
 std::string prefix="{\"generated_timestamp\":\"2026-09-10T16:01:46+08:00\", \"data\" : [";time_t now=parseISO("2026-09-10T16:01:46+08:00");
 Input a{prefix+row+"]}"};assert(decodePosition(a,f,"92",'O',1,"D",now));assert(f.counts[2]==1&&f.rows[2][0].scheduled&&f.rows[2][0].rank==2);
 Input b{prefix+row+",]}"};assert(!decodePosition(b,f,"92",'O',1,"D",now));Input c{prefix+row+"]"};assert(!decodePosition(c,f,"92",'O',1,"D",now));Input d{prefix+row+"]}"};assert(!decodePosition(d,f,"92",'O',1,"D",now+121));Input e{prefix+row+"]}"};assert(decodePosition(e,f,"92",'I',1,"D",now)&&f.counts[2]==0);
 if(argc>1){std::ifstream file(argv[1]);std::stringstream text;text<<file.rdbuf();Input live{text.str()};DynamicJsonDocument meta(1024);auto end=live.s.find(",\"data\"");assert(end!=std::string::npos);std::string hdr=live.s.substr(0,end)+"}";assert(!deserializeJson(meta,hdr));time_t t=parseISO(meta["generated_timestamp"]|"");assert(decodePosition(live,f,"92",'O',1,"",t));assert(f.counts[2]>0);}
 std::cout<<"Streamed route decode, variant filter, scheduled flags, truncated/trailing/stale input PASS\n";
}
