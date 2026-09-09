#pragma once
#include <cstring>
inline bool validWifiCredentials(const char* ssid,const char* pass){auto n=strlen(ssid),p=strlen(pass);return n>0&&n<=32&&p<=63&&(p==0||p>=8);}
