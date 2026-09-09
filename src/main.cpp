#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_ST7789.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>
#include "smooth_font.h"
#include "kmb_ca.h"
#include "eta_logic.h"
#include "eta_animation.h"
#include "setup_page.h"
#include "buffered_display.h"
SPIClass lcdSPI(HSPI),touchSPI(VSPI);
Adafruit_ST7789 panel(&lcdSPI,15,2,-1);
BufferedDisplay lcd;
#include "plate_canvas.h"
PlateCanvas plate(lcd);
#include "hardware.inc"
Preferences prefs;WebServer web(80);
#include "daylight.h"
uint16_t BG=0xFF7A,INK=0x21E6,MAP=0xD71D,PANEL=0xEF18,FEATURE=0xF6B8,FIELD=0xFFFE,ROAD=0xFFDD,EDGE=0xCE35;
constexpr uint16_t RED=0xCA07,PAPER=0xFFFE,PLATE_INK=0x21E6;
bool nightMode=false;
int themeMode=0; // 0 solar auto, 1 day, 2 night. Stored separately from route config.
int clockMinute=-1;
struct Stop{char id[17];char name[100];double lat,lng;int seq;};
struct Config{char route[8]="92";char bound='O';int service=1,seq=8;char stop[17]="5089C69E080B7A43";char destination[100]="鑽石山站";Stop stops[3];int count=3,threshold=5,brightness=190;bool armed=false;uint32_t gen=1;};
Config cfg;String wifiName,wifiPassword,pin,serialLine,lastMessage="請在 Mac 開啟設定頁";
struct Request{Config c;int kind;};struct Result{uint32_t gen;int code;time_t eta,stamp;int kind;};
#include "road_fetch.inc"
QueueHandle_t requests,results;bool requestBusy=false,settings=false,alarmOn=false,needsDraw=true;
volatile int apiDiagnostic=0;
int etaCode=0;time_t etaEpoch=0,dataStamp=0;unsigned long lastPoll=0,lastRender=0,lastStatus=0,lastTouch=0;int failures=0;
float calibration[6];bool calibrated=false;int calStep=0;float rawCal[3][2];bool held=false;
int touchX=0,touchY=0;
ApproachMotion motion;
bool mapActualRoad=false,markerPainted=false;
int mapX[3],mapY[3],markerX=0,markerY=0;
unsigned long lastAnimation=0;
void drawMap();
void mapGrid();
void drawDownloadedMap();
void updateAnimation();
void updateTheme(int mode=-1){
 int selected=mode<0?themeMode:mode;time_t now=time(nullptr);time_t hk=now+8*3600;tm local={};gmtime_r(&hk,&local);
 bool dark=selected==2;
 if(selected==0&&now>1700000000){auto stop=cfg.stops[cfg.count-1];auto sun=solarMinutes(local.tm_yday+1,stop.lat,stop.lng);int minute=local.tm_hour*60+local.tm_min;dark=minute<sun.rise||minute>=sun.set;}
 if(dark!=nightMode){nightMode=dark;needsDraw=true;markerPainted=false;}
 BG=dark?0x10E3:0xFF7A;INK=dark?0xFF7A:0x21E6;MAP=dark?0x224C:0xD71D;PANEL=dark?0x29E8:0xEF18;FEATURE=dark?0x4289:0xF6B8;FIELD=dark?0x1924:0xFFFE;ROAD=dark?0x7C0E:0xFFDD;EDGE=dark?0x4269:0xCE35;
}
uint16_t color(uint8_t r,uint8_t g,uint8_t b){return ((r>>3)<<11)|((g>>2)<<5)|(b>>3);}
uint32_t nextCode(const char*& p){unsigned char c=*p++;if(c<128)return c;uint32_t r=c&((c<224)?31:15);int n=c<224?1:2;while(n--&&*p)r=(r<<6)|(*p++&63);return r;}
Adafruit_GFX* painter=&lcd;
uint16_t textBG=BG;
int aaIndex(uint32_t c,int size){if(size>20)return c>=32&&c<127?c-32:-1;int l=0,r=aaCount-1;while(l<=r){int m=(l+r)/2;uint16_t k=pgm_read_word(aaCodes+m);if(k==c)return m;if(k<c)l=m+1;else r=m-1;}return -1;}
const uint8_t* aaWidths(int z){return z==12?aaWidths12:z==16?aaWidths16:z==20?aaWidths20:z==39?aaWidths39:aaWidths67;}
const uint8_t* aaData(int z){return z==12?aaData12:z==16?aaData16:z==20?aaData20:z==39?aaData39:aaData67;}
int textWidth(const char*s,int size){int w=0;while(*s){uint32_t c=nextCode(s);int i=aaIndex(c,size);w+=i<0?size:pgm_read_byte(aaWidths(size)+i);}return w;}
uint16_t blend565(uint16_t fg,uint16_t bg,int a){int r=(((fg>>11)&31)*a+((bg>>11)&31)*(15-a)+7)/15,g=(((fg>>5)&63)*a+((bg>>5)&63)*(15-a)+7)/15,b=((fg&31)*a+(bg&31)*(15-a)+7)/15;return (r<<11)|(g<<5)|b;}
void label(int x,int y,const char*s,int size=16,uint16_t fg=INK,int maxWidth=320){int start=x;uint16_t row[67];while(*s){uint32_t c=nextCode(s);int i=aaIndex(c,size),advance=i<0?size:pgm_read_byte(aaWidths(size)+i);if(x+advance-start>maxWidth)break;if(i>=0){const uint8_t* data=aaData(size)+i*((size*size+1)/2);for(int yy=0;yy<size;yy++){for(int xx=0;xx<advance;xx++){int p=yy*size+xx;uint8_t packed=pgm_read_byte(data+p/2);int a=p%2?(packed&15):(packed>>4);row[xx]=blend565(fg,textBG,a);}painter->drawRGBBitmap(x,y+yy,row,advance,1);}}x+=advance;}}
void centered(int x,int y,int width,const char*s,int size,uint16_t fg=INK){label(x+max(0,(width-textWidth(s,size))/2),y,s,size,fg,width);}
void bigNumber(const char*s,int x,int y,int width,int height,uint16_t fg=INK){int sz=height>=58?67:39;if(textWidth(s,sz)>width)sz=39;if(textWidth(s,sz)>width)sz=20;label(x+(width-textWidth(s,sz))/2,y+(height-sz)/2,s,sz,fg,width);}
String stationName(){String s=cfg.stops[cfg.count-1].name;int pos=s.indexOf('(');if(pos>=0)s=s.substring(0,pos);s.trim();return s;}
void drawPlate(){plate.fillScreen(BG);plate.fillCircle(56,56,51,RED);painter=&plate;textBG=RED;centered(7,16,98,"巴士站",12,PAPER);plate.fillRect(16,33,80,43,0xFFFE);textBG=0xFFFE;bigNumber(cfg.route,16,33,80,43,PLATE_INK);textBG=RED;String name=stationName();String shown;const char*rest=name.c_str();while(*rest){const char*start=rest;nextCode(rest);String ch;while(start<rest)ch+=*start++;if(textWidth((shown+ch+(*rest?"…":"")).c_str(),12)>72){shown+="…";break;}shown+=ch;}centered(20,80,72,shown.c_str(),12,PAPER);painter=&lcd;textBG=BG;
 const float co=.9986295348f,si=.0523359562f;uint16_t line[120];
 for(int y=0;y<120;y++){for(int x=0;x<120;x++){float u=(x-59.5f)*112/120.f,v=(y-59.5f)*112/120.f;float sx=co*u-si*v+55.5f,sy=si*u+co*v+55.5f;int ix=floorf(sx),iy=floorf(sy);if(ix<0||ix>=111||iy<0||iy>=111){line[x]=BG;continue;}int ax=roundf((sx-ix)*15),ay=roundf((sy-iy)*15);line[x]=blend565(blend565(plate.pixelColor(ix+1,iy+1),plate.pixelColor(ix,iy+1),ax),blend565(plate.pixelColor(ix+1,iy),plate.pixelColor(ix,iy),ax),ay);}lcd.drawRGBBitmap(6,y+1,line,120,1);}}
void emitState(){DynamicJsonDocument d(1024);d["event"]="state";d["app"]="kmb-bus-clock";d["version"]="0.2.0";d["nightMode"]=nightMode;d["themeMode"]=themeMode;d["clockSynced"]=time(nullptr)>1700000000;d["connected"]=WiFi.status()==WL_CONNECTED;d["ip"]=WiFi.localIP().toString();d["ssid"]=wifiName;d["message"]=lastMessage;d["route"]=cfg.route;d["bound"]=String(cfg.bound);d["service"]=cfg.service;d["stop"]=cfg.stop;d["threshold"]=cfg.threshold;d["brightness"]=cfg.brightness;d["armed"]=cfg.armed;d["etaCode"]=etaCode;d["animationVisible"]=motion.visible;d["animationFraction"]=motion.fraction;d["sentTiles"]=lcd.sentTiles;d["presents"]=lcd.presents;d["roadReady"]=activeRoad.valid;d["roadPoints"]=activeRoad.count;d["mapStops"]=activeRoad.stopCount;d["mapMeters"]=activeRoad.meters;d["roadDiagnostic"]=roadDiagnostic;d["heap"]=ESP.getFreeHeap();d["displayReady"]=lcd.ready();d["apiDiagnostic"]=apiDiagnostic;d["largestHeap"]=ESP.getMaxAllocHeap();serializeJson(d,Serial);Serial.println();}
void saveArmed(){prefs.putBool("armed",cfg.armed);}
void defaultStops(){strlcpy(cfg.stops[0].id,"1741D103CB826E93",17);strlcpy(cfg.stops[0].name,"大涌口",100);cfg.stops[0].lat=22.372007;cfg.stops[0].lng=114.260106;cfg.stops[0].seq=6;strlcpy(cfg.stops[1].id,"4823D6EFB3722E64",17);strlcpy(cfg.stops[1].name,"白沙臺",100);cfg.stops[1].lat=22.367542;cfg.stops[1].lng=114.260133;cfg.stops[1].seq=7;strlcpy(cfg.stops[2].id,cfg.stop,17);strlcpy(cfg.stops[2].name,"白沙灣",100);cfg.stops[2].lat=22.364778;cfg.stops[2].lng=114.259413;cfg.stops[2].seq=8;}
bool validID(const char*s){if(strlen(s)!=16)return false;for(int i=0;i<16;i++)if(!isxdigit(s[i]))return false;return true;}
bool applyConfig(JsonDocument&d,bool persist){
 const char*route=d["route"]|"";const char*bound=d["bound"]|"";const char*stop=d["stop"]|"";
 if(strlen(route)<1||strlen(route)>6||strlen(bound)!=1||(bound[0]!='O'&&bound[0]!='I')||!validID(stop))return false;
 for(const char*p=route;*p;p++)if(!isalnum(*p))return false;
 int service=d["service"]|0,seq=d["seq"]|0,threshold=d["threshold"]|0,brightness=d["brightness"]|190;
 JsonArray ss=d["stops"].as<JsonArray>();if(service<1||service>50||seq<1||seq>300||threshold<1||threshold>60||ss.size()<1||ss.size()>3||brightness<20||brightness>255)return false;
 Config next=cfg;strlcpy(next.route,route,8);next.bound=bound[0];strlcpy(next.stop,stop,17);strlcpy(next.destination,d["destination"]|"",100);next.seq=seq;next.service=service;next.threshold=threshold;next.brightness=brightness;next.armed=d["armed"]|false;next.count=ss.size();
 for(int i=0;i<next.count;i++){auto s=ss[i];const char*id=s["id"]|"";double lat=s["lat"]|0.,lng=s["lng"]|0.;int sq=s["seq"]|0;if(!validID(id)||!isfinite(lat)||!isfinite(lng)||lat<22||lat>23||lng<113||lng>115||sq!=seq-next.count+1+i)return false;strlcpy(next.stops[i].id,id,17);strlcpy(next.stops[i].name,s["name"]|"",100);next.stops[i].lat=lat;next.stops[i].lng=lng;next.stops[i].seq=sq;}
 if(strcmp(next.stops[next.count-1].id,stop))return false;
 String newSSID=d["ssid"]|"",newPass=d["password"]|"";if(newSSID.length()>32||newPass.length()>63)return false;
 if(newSSID.length()&&newSSID!=wifiName&&newPass.length()==0&&wifiName.length())return false;
 bool sameMap=!strcmp(cfg.route,next.route)&&cfg.bound==next.bound&&cfg.service==next.service&&!strcmp(cfg.stop,next.stop);cfg=next;cfg.gen++;if(sameMap&&activeRoad.valid){activeRoad.gen=cfg.gen;roadTriedGen=cfg.gen;}else{activeRoad.valid=false;roadTriedGen=0;}motion.reset();markerPainted=false;alarmOn=false;etaEpoch=0;etaCode=0;dataStamp=0;needsDraw=true;lastPoll=0;failures=0;
 if(persist){d.remove("ssid");d.remove("password");d.remove("key");String safe;serializeJson(d,safe);prefs.putString("config",safe);saveArmed();}
 if(newSSID.length()&&(newSSID!=wifiName||newPass.length())){wifiName=newSSID;if(newPass.length())wifiPassword=newPass;if(persist){prefs.putString("ssid",wifiName);prefs.putString("pass",wifiPassword);}WiFi.begin(wifiName.c_str(),wifiPassword.c_str());}
 ledcWrite(0,cfg.brightness);lastMessage="設定已儲存；正在連線及核對路線";emitState();return true;
}
bool api(const String&path,DynamicJsonDocument&out){WiFiClientSecure tls;tls.setCACert(kmbCA);tls.setHandshakeTimeout(12);tls.setTimeout(7);HTTPClient h;h.setConnectTimeout(6000);h.setTimeout(7000);h.useHTTP10(true);if(!h.begin(tls,"https://data.etabus.gov.hk/v1/transport/kmb/"+path))return false;int code=h.GET();bool ok=false;if(code==200&&h.getSize()<65536){DeserializationError err;if(path.startsWith("route-stop/")){StaticJsonDocument<160> filter;filter["data"][0]["stop"]=true;filter["data"][0]["seq"]=true;err=deserializeJson(out,h.getStream(),DeserializationOption::Filter(filter));}else err=deserializeJson(out,h.getStream());apiDiagnostic=(int)err.code();ok=!err;}else apiDiagnostic=code;h.end();return ok;}
void worker(void*){Request req;uint32_t validated=0;for(;;){if(xQueueReceive(requests,&req,portMAX_DELAY)!=pdTRUE)continue;Result result={req.c.gen,-1,0,0};if(req.kind==1){result.kind=1;result.code=prepareRoad(req.c)?1:0;xQueueSend(results,&result,portMAX_DELAY);continue;}DynamicJsonDocument d(8192);
 if(validated!=req.c.gen){String dir=req.c.bound=='O'?"outbound":"inbound";bool found=false;if(api("route-stop/"+String(req.c.route)+"/"+dir+"/"+String(req.c.service),d)){for(JsonObject row:d["data"].as<JsonArray>())if(String(row["stop"]|"")==req.c.stop&&atoi(row["seq"]|"0")==req.c.seq)found=true;}if(!found){result.code=-2;xQueueSend(results,&result,portMAX_DELAY);continue;}cacheRoadStops(req.c,d["data"].as<JsonArray>());validated=req.c.gen;d.clear();}
 if(api("eta/"+String(req.c.stop)+"/"+req.c.route+"/"+String(req.c.service),d)){time_t now=time(nullptr),generated=parseISO(d["generated_timestamp"]|"");result.code=1;
 if(!generated||abs((long)(now-generated))>120)result.code=-3;
 else{for(JsonObject row:d["data"].as<JsonArray>()){if(String(row["route"]|"")!=req.c.route||String(row["dir"]|"")!=String(req.c.bound)||row["service_type"].as<int>()!=req.c.service||row["seq"].as<int>()!=req.c.seq)continue;time_t stamp=parseISO(row["data_timestamp"]|""),eta=parseISO(row["eta"]|"");if(!stamp||abs((long)(now-stamp))>120){result.code=-3;continue;}if(eta>=now-30&&eta<=now+10800&&(result.eta==0||eta<result.eta)){result.eta=eta;result.stamp=stamp;result.code=2;}}}}
 xQueueSend(results,&result,portMAX_DELAY);}}
void calibrationView(){lcd.fillScreen(BG);centered(0,80,320,"請按十字校正觸控",20);int x=calStep==1?290:30,y=calStep==2?210:30;lcd.drawLine(x-12,y,x+12,y,RED);lcd.drawLine(x,y-12,x,y+12,RED);}
void drawMain(){textBG=BG;lcd.fillScreen(BG);drawPlate();
 label(134,22,"往",12);label(149,22,cfg.destination,12,INK,159);lcd.fillRoundRect(262,4,54,28,6,INK);textBG=INK;for(int i=0;i<32;i++){float a=i*3.14159265f/16,b=(i+1)*3.14159265f/16;int r=(i%4==1||i%4==2)?11:8,j=(i+1)%32,t=(j%4==1||j%4==2)?11:8;lcd.fillTriangle(289,18,289+roundf(cosf(a)*r),18+roundf(sinf(a)*r),289+roundf(cosf(b)*t),18+roundf(sinf(b)*t),BG);}lcd.fillCircle(289,18,4,INK);textBG=BG;
 const char*message="未接 Wi-Fi";time_t now=time(nullptr);bool fresh=WiFi.status()==WL_CONNECTED&&etaCode==2&&now-dataStamp<=120&&now-etaEpoch<=30;
 if(fresh){char n[8];snprintf(n,sizeof(n),"%d",max(0,(int)ceil((etaEpoch-now)/60.)));bigNumber(n,10,126,108,58);message="分鐘到";}
 else{bigNumber("--",10,126,108,58);if(WiFi.status()==WL_CONNECTED)message=etaCode==1?"未有預報":etaCode==-3?"資料過期":etaCode<0?"更新失敗":"連線中";}
 centered(8,184,116,message,16);
 char clockText[6]="--:--";if(now>1700000000){time_t hk=now+28800;tm t={};gmtime_r(&hk,&t);snprintf(clockText,sizeof(clockText),"%02d:%02d",t.tm_hour,t.tm_min);}label(134,1,clockText,20);
 drawMap();
 updateAnimation();
 lcd.fillRoundRect(12,204,296,28,7,alarmOn?RED:INK);textBG=alarmOn?RED:INK;centered(12,210,296,alarmOn?"停止":cfg.armed?"取消提醒":"提醒我",16,alarmOn?PAPER:BG);
 textBG=BG;if(alarmOn)lcd.drawRect(0,0,320,240,RED);
}
void drawMap(){markerPainted=false;mapActualRoad=false;if(activeRoad.valid&&activeRoad.gen==cfg.gen){drawDownloadedMap();return;}lcd.fillRoundRect(134,35,174,140,10,MAP);mapGrid();textBG=MAP;centered(134,92,174,roadTriedGen==cfg.gen?"路線圖暫缺":"載入中",16);textBG=BG;lcd.fillRect(134,175,174,27,BG);label(136,185,"3km · 估算",12);}
#include "map_render.inc"
void restoreMarker(){if(markerPainted)drawMap();markerPainted=false;}
void updateAnimation(){
 const unsigned long tick=millis();const float seconds=lastAnimation?(tick-lastAnimation)/1000.0f:.1f;lastAnimation=tick;
 time_t now=time(nullptr);motion.update(activeRoad.valid&&activeRoad.gen==cfg.gen&&animationFresh(WiFi.status()==WL_CONNECTED,etaCode,now,dataStamp,etaEpoch),now,etaEpoch,seconds);
 if(!motion.visible){restoreMarker();return;}
 bool downloaded=activeRoad.valid&&activeRoad.gen==cfg.gen;
 const int count=activeRoad.count;
 auto point=[&](int i,int axis)->float{return axis?activeRoad.points[i].y:activeRoad.points[i].x;};
 float total=0;for(int i=1;i<count;i++)total+=hypotf(point(i,0)-point(i-1,0),point(i,1)-point(i-1,1));
 float distance=motion.fraction*total,x=point(0,0),y=point(0,1);
 for(int i=1;i<count;i++){float dx=point(i,0)-point(i-1,0),dy=point(i,1)-point(i-1,1),len=hypotf(dx,dy);if(len<=.001f)continue;if(distance<=len||i==count-1){float t=constrain(distance/len,0.0f,1.0f);x=point(i-1,0)+dx*t;y=point(i-1,1)+dy*t;break;}distance-=len;}
 const int bob=motion.fraction<.995f?((tick/650)%2):0;
 int nextX=constrain((int)roundf(x),145,296),nextY=constrain((int)roundf(y)+bob,48,161);
 if(markerPainted&&markerX==nextX&&markerY==nextY)return;
 restoreMarker();markerX=nextX;markerY=nextY;
 GFXcanvas16 sprite(22,26);if(!sprite.getBuffer())return;
 for(int y=0;y<26;y++)for(int x=0;x<22;x++)sprite.drawPixel(x,y,lcd.pixelColor(markerX-11+x,markerY-13+y));
 sprite.fillRoundRect(2,0,18,24,6,0xFFDD);sprite.fillRoundRect(3,1,16,22,5,0xD328);
 sprite.fillRoundRect(6,4,10,5,2,0xBEDB);sprite.fillRoundRect(6,11,10,5,2,0xBEDB);
 sprite.fillCircle(6,19,1,0xFFDD);sprite.fillCircle(15,19,1,0xFFDD);
 sprite.drawLine(9,19,10,20,INK);sprite.drawLine(10,20,12,19,INK);
 sprite.fillRect(5,23,3,2,INK);sprite.fillRect(14,23,3,2,INK);
 lcd.drawRGBBitmap(markerX-11,markerY-13,sprite.getBuffer(),22,26);markerPainted=true;
}
#include "wifi_setup.inc"
#include "native_settings.inc"
void onTouch(int rx,int ry){if(!calibrated){rawCal[calStep][0]=rx;rawCal[calStep][1]=ry;calStep++;if(calStep<3){calibrationView();return;}float ux=rawCal[1][0]-rawCal[0][0],uy=rawCal[1][1]-rawCal[0][1],vx=rawCal[2][0]-rawCal[0][0],vy=rawCal[2][1]-rawCal[0][1],det=ux*vy-uy*vx;if(fabs(det)<10000){calStep=0;calibrationView();return;}calibration[0]=260*vy/det;calibration[1]=-260*vx/det;calibration[2]=30-calibration[0]*rawCal[0][0]-calibration[1]*rawCal[0][1];calibration[3]=-180*uy/det;calibration[4]=180*ux/det;calibration[5]=30-calibration[3]*rawCal[0][0]-calibration[4]*rawCal[0][1];prefs.putBytes("cal",calibration,sizeof(calibration));calibrated=true;needsDraw=true;return;}
 int x=calibration[0]*rx+calibration[1]*ry+calibration[2],y=calibration[3]*rx+calibration[4]*ry+calibration[5];if(settings){nativeTouch(x,y);return;}else if(x>=245&&y<=40){openNative();}else if(y>=195){if(alarmOn)alarmOn=false;else cfg.armed=!cfg.armed;saveArmed();}needsDraw=true;}
void setup(){hardwareInit();lcd.begin();prefs.begin("busclock",false);themeMode=constrain(prefs.getInt("theme",0),0,2);defaultStops();wifiName=prefs.getString("ssid","");wifiPassword=prefs.getString("pass","");wifiPreviousName=prefs.getString("prevssid","");wifiPreviousPass=prefs.getString("prevpass","");String saved=prefs.getString("config","");if(saved.length()){DynamicJsonDocument d(4096);if(!deserializeJson(d,saved))applyConfig(d,false);}cfg.armed=prefs.getBool("armed",false);pin=String(100000+esp_random()%900000);calibrated=prefs.getBytesLength("cal")==sizeof(calibration);if(calibrated)prefs.getBytes("cal",calibration,sizeof(calibration));
 WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);if(wifiName.length())WiFi.begin(wifiName.c_str(),wifiPassword.c_str());configTime(0,0,"time.cloudflare.com","pool.ntp.org");setenv("TZ","UTC0",1);tzset();
 requests=xQueueCreate(1,sizeof(Request));results=xQueueCreate(1,sizeof(Result));xTaskCreatePinnedToCore(worker,"eta",12288,nullptr,1,nullptr,0);
 web.on("/",HTTP_GET,[]{web.send_P(200,"text/html; charset=utf-8",setupPage);});web.on("/state",HTTP_GET,[]{DynamicJsonDocument d(500);d["message"]=lastMessage;String s;serializeJson(d,s);web.send(200,"application/json",s);});
 web.on("/config",HTTP_POST,[]{static unsigned long last=0;if(millis()-last<2000){web.send(429,"application/json","{\"error\":\"請稍後再試\"}");return;}last=millis();String body=web.arg("plain");DynamicJsonDocument d(4096);if(body.length()>4000||deserializeJson(d,body)||String(d["key"]|"")!=pin){web.send(403,"application/json","{\"error\":\"設定碼不正確\"}");return;}bool ok=applyConfig(d,true);web.send(ok?200:400,"application/json",ok?"{\"ok\":true}":"{\"error\":\"請檢查路線及設定\"}");});web.begin();nativeInit();
 updateTheme();if(!calibrated)calibrationView();else drawMain();Serial.println("KMB_CLOCK_LIVE_READY");emitState();}
void loop(){web.handleClient();pollNative();pollWifiSetup();while(Serial.available()){char ch=Serial.read();if(ch=='\n'){DynamicJsonDocument d(4096);if(!deserializeJson(d,serialLine)){if(String(d["cmd"]|"")=="state")emitState();else if(String(d["cmd"]|"")=="wifi")wifiCommand(d);else if(String(d["cmd"]|"")=="settings")nativeDebug(d);else if(!applyConfig(d,true)){lastMessage="設定無效，未儲存";emitState();}}serialLine="";}else if(serialLine.length()<4000)serialLine+=ch;else serialLine="";}
 bool pressed=digitalRead(36)==LOW;if(pressed&&!held&&millis()-lastTouch>250){touchSPI.beginTransaction(SPISettings(1000000,MSBFIRST,SPI_MODE0));digitalWrite(33,LOW);touchSPI.transfer(0xD0);int rx=touchSPI.transfer16(0)>>3;touchSPI.transfer(0x90);int ry=touchSPI.transfer16(0)>>3;digitalWrite(33,HIGH);touchSPI.endTransaction();lastTouch=millis();if(rx>100&&rx<4000&&ry>100&&ry<4000)onTouch(rx,ry);}held=pressed;
 Result result;if(xQueueReceive(results,&result,0)==pdTRUE){requestBusy=false;if(result.kind==1){if(result.gen==cfg.gen){roadTriedGen=cfg.gen;roadAttemptAt=millis();activeRoad=pendingRoad;needsDraw=true;}}else if(result.gen==cfg.gen){etaCode=result.code;etaEpoch=result.eta;dataStamp=result.stamp;failures=etaCode<0?min(failures+1,4):0;lastMessage=etaCode==2?"已連線，正在顯示真實九巴 ETA":etaCode==1?"已連線，九巴暫時沒有到站預報":etaCode==-2?"路線或車站未能核實，正在重試":"到站資料未能更新，正在重試";needsDraw=true;emitState();}}
 time_t now=time(nullptr);if(WiFi.status()==WL_CONNECTED&&now>1700000000&&!requestBusy&&!nativeBusy&&!nativePending&&!wifiWorkActive()&&(lastPoll==0||millis()-lastPoll>30000UL*(1<<failures))){Request r={cfg};if(xQueueSend(requests,&r,0)==pdTRUE){requestBusy=true;lastPoll=millis();}}
 if(WiFi.status()==WL_CONNECTED&&now>1700000000&&!requestBusy&&!nativeBusy&&!nativePending&&!wifiWorkActive()&&(roadTriedGen!=cfg.gen||(!activeRoad.valid&&millis()-roadAttemptAt>30000))&&(etaCode==1||etaCode==2)){Request mapRequest={cfg,1};if(xQueueSend(requests,&mapRequest,0)==pdTRUE)requestBusy=true;}
 if(arrivalAlert(cfg.armed,WiFi.status()==WL_CONNECTED,etaCode,now,dataStamp,etaEpoch,cfg.threshold)){alarmOn=true;cfg.armed=false;saveArmed();needsDraw=true;}
 if((int)(now/60)!=clockMinute){clockMinute=now/60;if(!settings)updateTheme();needsDraw=true;}
 if(calibrated&&(needsDraw||millis()-lastRender>15000)){if(settings)drawSettings();else drawMain();needsDraw=false;lastRender=millis();}
 if(calibrated&&!settings&&millis()-lastAnimation>=100)updateAnimation();
 if(millis()-lastStatus>7000){lastStatus=millis();if(WiFi.status()!=WL_CONNECTED)lastMessage=wifiName.length()?"Wi-Fi 尚未連上，請確認名稱、密碼及2.4GHz訊號":"請輸入 Wi-Fi 並選擇路線，然後儲存";emitState();}lcd.present(panel);delay(5);}
