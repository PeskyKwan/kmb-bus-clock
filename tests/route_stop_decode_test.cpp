#include "../src/route_stop_decode.h"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>
struct Input{std::string s;size_t at=0;int held=-1;bool allowed(){return at<s.size()||held>=0;}int read(){if(held>=0){int c=held;held=-1;return c;}return at<s.size()?(unsigned char)s[at++]:-1;}size_t readBytes(char*b,size_t n){size_t i=0;for(;i<n;i++){int c=read();if(c<0)break;b[i]=c;}return i;}int token(){int c;while((c=read())>=0)if(!std::isspace(c))return c;return -1;}void unread(int c){held=c;}bool find(const char*needle){auto p=s.find(needle,at);if(p==std::string::npos)return false;at=p+strlen(needle);return true;}};
bool valid(const char*s){if(strlen(s)!=16)return false;for(int i=0;i<16;i++)if(!std::isxdigit((unsigned char)s[i]))return false;return true;}
std::string row(const char*route="1A",const char*bound="O",int service=1,int seq=1,const char*stop="A3ADFCDF8487ADB9"){return std::string("{\"route\":\"")+route+"\",\"bound\":\""+bound+"\",\"service_type\":\""+std::to_string(service)+"\",\"seq\":\""+std::to_string(seq)+"\",\"stop\":\""+stop+"\"}";}
int main(){int diagnostic=0;std::vector<int>seqs;auto sink=[&](const char*,int seq){seqs.push_back(seq);return true;};Input good{"{\"type\":\"RouteStop\",\"data\":["+row()+","+row("1A","O",1,2,"8D804CFD9C7B9042")+"]}"};assert(decodeRouteStops(good,"1A",'O',1,sink,valid,diagnostic)&&diagnostic==0&&seqs.size()==2&&seqs[1]==2);Input wrong{"{\"data\":["+row("1A","I")+"]}"};assert(!decodeRouteStops(wrong,"1A",'O',1,sink,valid,diagnostic)&&diagnostic==-20);Input bad{"{\"data\":["+row("1A","O",1,1,"BAD")+"]}"};assert(!decodeRouteStops(bad,"1A",'O',1,sink,valid,diagnostic)&&diagnostic==-20);Input truncated{"{\"data\":["+row()};assert(!decodeRouteStops(truncated,"1A",'O',1,sink,valid,diagnostic));Input trailing{"{\"data\":["+row()+",]}"};assert(!decodeRouteStops(trailing,"1A",'O',1,sink,valid,diagnostic));puts("Streamed route-stop envelope, variant, ID and truncation checks PASS");}
