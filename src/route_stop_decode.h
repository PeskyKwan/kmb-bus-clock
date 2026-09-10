#pragma once
#include <ArduinoJson.h>
#include <cstring>
#include <cstdlib>
template<class Input,class Sink,class Valid>bool decodeRouteStops(Input&input,const char*route,char bound,int service,Sink sink,Valid valid,int&diagnostic){
 int rows=0;if(!input.find("\"data\"")||input.token()!=':'||input.token()!='['){diagnostic=-21;return false;}DynamicJsonDocument row(512);StaticJsonDocument<160> filter;for(const char*k:{"route","bound","service_type","seq","stop"})filter[k]=true;
 while(input.allowed()&&rows<256){int next=input.token();if(next!='{'){diagnostic=-21;return false;}input.unread(next);row.clear();DeserializationError err=deserializeJson(row,input,DeserializationOption::Filter(filter));if(err){diagnostic=(int)err.code();return false;}const char*id=row["stop"]|"",*rowRoute=row["route"]|"",*rowBound=row["bound"]|"";int seq=atoi(row["seq"]|"0"),rowService=atoi(row["service_type"]|"0");if(strcmp(rowRoute,route)||strlen(rowBound)!=1||rowBound[0]!=bound||rowService!=service||!valid(id)||seq<1||!sink(id,seq)){diagnostic=-20;return false;}rows++;int delimiter=input.token();if(delimiter==']'){diagnostic=rows>0&&input.token()=='}'?0:-21;return diagnostic==0;}if(delimiter!=','){diagnostic=-21;return false;}}
 diagnostic=-22;return false;
}
