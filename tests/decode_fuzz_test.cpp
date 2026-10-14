// Deterministic fuzz test for the streamed ETA (position_decode.h) and route-stop
// (route_stop_decode.h) decoders. Feeds random, truncated, oversized and malformed
// JSON; same seed => same cases. Build (ArduinoJson v6.21.5 headers via -I):
//   c++ -std=c++17 -g -O1 -fsanitize=address,undefined -fno-sanitize-recover=all
//     -I<ArduinoJson-6.21.5>/src tests/decode_fuzz_test.cpp -o /tmp/kmb-decode-fuzz
//   /tmp/kmb-decode-fuzz <seed> [cases=10000]
#include "../src/position_decode.h"
#include "../src/route_stop_decode.h"
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// Same stream shape as the device adapters in the other decode tests, plus find().
struct Input{std::string s;size_t at=0;int held=-1;bool allowed(){return at<s.size()||held>=0;}int read(){if(held>=0){int c=held;held=-1;return c;}return at<s.size()?(unsigned char)s[at++]:-1;}size_t readBytes(char*b,size_t n){size_t i=0;for(;i<n;i++){int c=read();if(c<0)break;b[i]=c;}return i;}int token(){int c;while((c=read())>=0)if(!std::isspace(c))return c;return -1;}void unread(int c){held=c;}bool find(const char*needle){auto p=s.find(needle,at);if(p==std::string::npos){at=s.size();held=-1;return false;}at=p+strlen(needle);held=-1;return true;}};

// splitmix64; the seed is hashed first so nearby seeds give unrelated streams.
static uint64_t mix(uint64_t z){z=(z^(z>>30))*0xBF58476D1CE4E5B9ull;z=(z^(z>>27))*0x94D049BB133111EBull;return z^(z>>31);}
struct Rng{uint64_t x;explicit Rng(uint64_t seed):x(mix(mix(seed^0xD1B54A32D192ED03ull))){}
 uint64_t next(){return mix(x+=0x9E3779B97F4A7C15ull);}
 int below(int n){return n<=0?0:int(next()%uint64_t(n));}bool chance(int percent){return below(100)<percent;}
 template<class T,size_t N>const T&pick(const T(&a)[N]){return a[below(N)];}};

static const char*kRoute="92";static const char kBound='O';static const int kService=1;static const char*kDest="D";
static const char*kGenerated="2026-09-10T16:01:46+08:00";
static bool clean=false; // when set, generators emit only rows the decoders should accept

std::string digits(Rng&r,int n){std::string s;for(int i=0;i<n;i++)s+=char('0'+r.below(10));return s;}
// Mostly well-formed ISO-8601 stamps, sometimes out-of-range or broken.
std::string isoStamp(Rng&r){
 if(r.chance(55))return r.chance(50)?"2026-09-10T16:0"+std::to_string(r.below(10))+":"+digits(r,2)+"+08:00":kGenerated;
 static const char*odd[]={"","2026-13-40T25:61:61+08:00","2026-09-10T16:01:46+15:00","2026-09-10T16:01:46+08:60","2026-09-10T16:01:46Z","2026-09-10 16:01:46+08:00","9999-99-99T99:99:99-99:99","0000-00-00T00:00:00+00:00","2026-02-30T00:00:00-14:00","x026-09-10T16:01:46+08:00","2026-09-10T16:01:46+08:000"};
 if(r.chance(60))return r.pick(odd);
 return digits(r,4)+"-"+digits(r,2)+"-"+digits(r,2)+"T"+digits(r,2)+":"+digits(r,2)+":"+digits(r,2)+(r.chance(50)?"+":"-")+digits(r,2)+":"+digits(r,2);
}
std::string junkString(Rng&r,int maxLen){std::string s;int n=r.below(maxLen+1);for(int i=0;i<n;i++){int c=r.below(6);s+=c==0?char(1+r.below(31)):c==1?char(0x80+r.below(128)):c==2?'\\':char(32+r.below(95));}return s;}
std::string quoted(const std::string&s){std::string o="\"";for(char c:s){if(c=='"'||c=='\\')o+='\\';o+=c;}return o+"\"";}
// A random JSON value of bounded depth (used for unexpected field values).
std::string randomValue(Rng&r,int depth){
 switch(r.below(depth>0?9:6)){
  case 0:return "null";case 1:return r.chance(50)?"true":"false";
  case 2:return std::to_string(int64_t(r.next()))+(r.chance(30)?".5e"+std::to_string(r.below(700)-350):"");
  case 3:return quoted(junkString(r,24));case 4:return std::to_string(r.below(12)-2);case 5:return "1e999";
  case 6:{std::string s="[";int n=r.below(4);for(int i=0;i<n;i++)s+=(i?",":"")+randomValue(r,depth-1);return s+"]";}
  case 7:{std::string s="{";int n=r.below(4);for(int i=0;i<n;i++)s+=std::string(i?",":"")+quoted(junkString(r,6))+":"+randomValue(r,depth-1);return s+"}";}
  default:{std::string s;int d=12+r.below(30);for(int i=0;i<d;i++)s+=r.chance(50)?"[":"{\"k\":";return s+"1";}
 }
}
std::string field(Rng&r,const char*key,const std::string&good){return std::string("\"")+key+"\":"+(clean||r.chance(85)?good:randomValue(r,2));}
std::string positionRow(Rng&r){
 static const char*routes[]={"92","92","92","1A","","92X"};static const char*dirs[]={"O","O","I","","OO"};static const char*rmk[]={"","Scheduled Bus","原定班次","Final Bus"};
 std::vector<std::string>f={field(r,"route",quoted(clean?kRoute:r.pick(routes))),field(r,"dir",quoted(clean?"O":r.pick(dirs))),field(r,"service_type",clean||r.chance(80)?"1":std::to_string(r.below(4))),
  field(r,"seq",std::to_string(clean?6+r.below(3):r.below(12))),field(r,"eta_seq",std::to_string(clean?1+r.below(3):r.below(6)-1)),field(r,"dest_tc",quoted(clean||r.chance(80)?"D":"E")),
  field(r,"eta",quoted(isoStamp(r))),field(r,"data_timestamp",quoted(isoStamp(r))),field(r,"rmk_en",quoted(r.pick(rmk))),field(r,"rmk_tc",quoted(r.pick(rmk)))};
 if(!clean&&r.chance(15))f.push_back(quoted(junkString(r,8))+":"+randomValue(r,3));
 for(size_t i=f.size();i>1;i--)if(r.chance(20))std::swap(f[i-1],f[r.below(int(i))]);
 std::string s="{";for(size_t i=0;i<f.size();i++)if(clean||!r.chance(5))s+=(s.size()>1?",":"")+f[i];return s+"}";
}
std::string stopId(Rng&r){static const char hex[]="0123456789ABCDEF";std::string s;int n=clean||r.chance(85)?16:r.below(20);for(int i=0;i<n;i++)s+=clean||r.chance(97)?hex[r.below(16)]:'Z';return s;}
std::string routeStopRow(Rng&r,int seq){
 static const char*routes[]={"92","92","92","1A",""};static const char*bounds[]={"O","O","O","I","","OI"};
 std::vector<std::string>f={field(r,"route",quoted(clean?kRoute:r.pick(routes))),field(r,"bound",quoted(clean?"O":r.pick(bounds))),field(r,"service_type",quoted(clean||r.chance(85)?"1":std::to_string(r.below(4)))),
  field(r,"seq",clean||r.chance(90)?quoted(std::to_string(seq)):std::to_string(seq)),field(r,"stop",quoted(stopId(r)))};
 if(!clean&&r.chance(15))f.push_back(quoted(junkString(r,8))+":"+randomValue(r,3));
 std::string s="{";for(size_t i=0;i<f.size();i++)s+=(i?",":"")+f[i];return s+"}";
}
std::string ws(Rng&r){static const char*w[]={""," ","\n","\t","  \r\n "};return r.chance(70)?"":r.pick(w);}
std::string positionDoc(Rng&r,int rows){std::string s="{\"type\":\"ETA\","+ws(r)+"\"version\":\"1.0\",\"generated_timestamp\":"+quoted(clean||r.chance(85)?kGenerated:isoStamp(r))+","+ws(r)+"\"data\""+ws(r)+":"+ws(r)+"[";
 for(int i=0;i<rows;i++)s+=(i?","+ws(r):"")+positionRow(r);
 return s+ws(r)+"]}";}
std::string routeStopDoc(Rng&r,int rows){std::string s="{\"type\":\"RouteStop\",\"version\":\"1.0\",\"generated_timestamp\":\""+std::string(kGenerated)+"\","+ws(r)+"\"data\""+ws(r)+":"+ws(r)+"[";
 for(int i=0;i<rows;i++)s+=(i?","+ws(r):"")+routeStopRow(r,i+1);
 return s+ws(r)+"]}";}

// Byte-level damage: flips, inserts, deletes, duplicated spans, structural characters.
void mutate(Rng&r,std::string&s){int n=1+r.below(8);static const char structural[]="{}[]:,\"\\ 0-.eE";
 for(int k=0;k<n&&!s.empty();k++){size_t at=r.below(int(s.size()));switch(r.below(6)){
  case 0:s[at]=char(r.below(256));break;case 1:s.insert(at,1,structural[r.below(sizeof structural-1)]);break;
  case 2:s.erase(at,1+r.below(8));break;case 3:{size_t len=1+r.below(40);s.insert(r.below(int(s.size())),s.substr(at,len));break;}
  case 4:s[at]=structural[r.below(sizeof structural-1)];break;default:s.insert(at,std::string(1,'\0'));break;}}}

enum Kind{ValidPos,ValidStops,Truncated,Mutated,Oversized,RandomBytes,RandomJson,KindCount};
static const char*kindName[]={"valid-pos","valid-stops","truncated","mutated","oversized","random-bytes","random-json"};
std::string oversized(Rng&r){
 switch(r.below(6)){
  case 0:return positionDoc(r,2049+r.below(40));              // past the 2048-row cap
  case 1:return routeStopDoc(r,257+r.below(20));               // past the 256-row cap
  case 2:{std::string pad(400+r.below(800),' ');std::string s=positionDoc(r,2);s.insert(1,"\"pad\":\""+std::string(pad.size(),'x')+"\",");return s;} // "data" beyond 480-byte header scan
  case 3:{std::string s=positionDoc(r,1);std::string big=quoted(std::string(2000+r.below(20000),'A'));s.insert(s.size()-3,",\"rmk_en\":"+big);return s;}
  case 4:{std::string s=routeStopDoc(r,1);s.insert(s.size()-3,",\"stop\":"+quoted(std::string(1000+r.below(5000),'F')));return s;}
  default:{std::string deep(200+r.below(2000),'[');return (r.chance(50)?"{\"generated_timestamp\":\""+std::string(kGenerated)+"\",\"data\":[{\"route\":":"{\"data\":[{\"route\":")+deep;}
 }
}
std::string makeCase(Rng&r,Kind kind){
 clean=(kind==ValidPos||kind==ValidStops)&&r.chance(40);struct Reset{~Reset(){clean=false;}}reset;
 switch(kind){
  case ValidPos:return positionDoc(r,r.below(12));case ValidStops:return routeStopDoc(r,r.below(12));
  case Truncated:{std::string s=r.chance(50)?positionDoc(r,1+r.below(6)):routeStopDoc(r,1+r.below(6));return s.substr(0,r.below(int(s.size())));}
  case Mutated:{std::string s=r.chance(50)?positionDoc(r,1+r.below(6)):routeStopDoc(r,1+r.below(6));mutate(r,s);if(r.chance(25))s.resize(r.below(int(s.size())+1));return s;}
  case Oversized:return oversized(r);
  case RandomBytes:{std::string s;int n=r.below(600);for(int i=0;i<n;i++)s+=char(r.below(256));if(r.chance(40))s.insert(r.below(int(s.size())+1),r.chance(50)?"\"data\":[":"\"data\"");return s;}
  default:{std::string s="{\"generated_timestamp\":"+quoted(isoStamp(r))+",\"data\":"+randomValue(r,4);if(r.chance(50))s+="}";return s;}
 }
}

static bool validStop(const char*s){if(strlen(s)!=16)return false;for(int i=0;i<16;i++)if(!std::isxdigit((unsigned char)s[i]))return false;return true;}
struct PosResult{bool ok;std::time_t generated;int counts[positionAnchors];};
PosResult runPosition(const std::string&text,std::time_t now){
 EtaFrame f;f.layout.count=3;for(int i=0;i<3;i++)f.layout.anchors[i]={i+6,i*.5f};f.layout.meters=600;
 Input in{text};
 PosResult res{decodePosition(in,f,kRoute,kBound,kService,kDest,now),f.generated,{}};
 for(int i=0;i<positionAnchors;i++){res.counts[i]=f.counts[i];assert(f.counts[i]>=0&&f.counts[i]<=positionForecasts);if(i>=f.layout.count)assert(f.counts[i]==0);
  for(int j=0;j<f.counts[i];j++)assert(f.rows[i][j].rank>=1&&f.rows[i][j].rank<=3);}
 if(res.ok)assert(f.generated!=0&&std::labs(long(now-f.generated))<=120);
 return res;
}
struct StopResult{bool ok;int diagnostic,sunk;};
StopResult runStops(const std::string&text){
 Input in{text};int diagnostic=12345,sunk=0;
 auto sink=[&](const char*id,int seq){assert(validStop(id)&&seq>=1);sunk++;return true;};
 bool ok=decodeRouteStops(in,kRoute,kBound,kService,sink,validStop,diagnostic);
 assert(diagnostic!=12345);assert(ok==(diagnostic==0));if(ok)assert(sunk>=1&&sunk<=256);
 return {ok,diagnostic,sunk};
}

int main(int argc,char**argv){
 setenv("TZ","UTC0",1);tzset();
 uint64_t seed=argc>1?strtoull(argv[1],nullptr,10):1;int cases=argc>2?atoi(argv[2]):10000;
 Rng r(seed);std::time_t now=parseISO(kGenerated);assert(now>0);
 int byKind[KindCount]={},posOk=0,stopsOk=0,posRowsKept=0;size_t bytes=0;
 for(int i=0;i<cases;i++){
  Kind kind=Kind(r.below(KindCount));std::string text=makeCase(r,kind);byKind[kind]++;bytes+=text.size();
  std::time_t t=r.chance(90)?now:now+r.below(400)-200;
  PosResult a=runPosition(text,t),b=runPosition(text,t);          // decoders must be deterministic
  assert(a.ok==b.ok&&a.generated==b.generated&&!memcmp(a.counts,b.counts,sizeof a.counts));
  StopResult c=runStops(text),d=runStops(text);
  assert(c.ok==d.ok&&c.diagnostic==d.diagnostic&&c.sunk==d.sunk);
  posOk+=a.ok;stopsOk+=c.ok;for(int k=0;k<positionAnchors;k++)posRowsKept+=a.counts[k];
 }
 printf("seed=%llu cases=%d bytes=%zu pos_ok=%d pos_rows=%d stops_ok=%d |",(unsigned long long)seed,cases,bytes,posOk,posRowsKept,stopsOk);
 for(int k=0;k<KindCount;k++)printf(" %s=%d",kindName[k],byKind[k]);
 printf(" | PASS\n");
}
