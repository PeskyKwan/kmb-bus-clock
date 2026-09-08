#pragma once
#include <time.h>
#include <cstring>
#include <cstdlib>
#include <cctype>
inline time_t parseISO(const char*s){
 if(!s||strlen(s)!=25||s[4]!='-'||s[7]!='-'||s[10]!='T'||s[13]!=':'||s[16]!=':'||s[22]!=':'||(s[19]!='+'&&s[19]!='-'))return 0;
 for(int i:{0,1,2,3,5,6,8,9,11,12,14,15,17,18,20,21,23,24})if(!isdigit(s[i]))return 0;
 tm t={};if(!strptime(s,"%Y-%m-%dT%H:%M:%S",&t))return 0;int h=atoi(s+20),m=atoi(s+23);if(h>14||m>59)return 0;
 return mktime(&t)-(s[19]=='+'?1:-1)*(h*3600+m*60);
}
inline bool arrivalAlert(bool armed,bool connected,int code,time_t now,time_t stamp,time_t eta,int threshold){return armed&&connected&&code==2&&stamp>0&&labs(now-stamp)<=120&&eta>=now&&eta-now<=threshold*60;}
