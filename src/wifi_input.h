#pragma once
#include <cstring>
inline bool validWifiCredentials(const char* ssid,const char* pass){auto n=strlen(ssid),p=strlen(pass);return n>0&&n<=32&&p<=63&&(p==0||p>=8);}
inline bool sameWifiName(const char* a,const char* b){return a&&b&&*a&&strcmp(a,b)==0;}
inline bool storedWifiName(const char* candidate,const char* current,const char* previous){return sameWifiName(candidate,current)||sameWifiName(candidate,previous);}
inline bool activeWifiName(const char* candidate,bool connected,const char* active){return connected&&sameWifiName(candidate,active);}
