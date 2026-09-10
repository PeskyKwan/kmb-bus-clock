#pragma once
#include <ArduinoJson.h>
#include <string>
#include "eta_position.h"
#include "eta_logic.h"
template<class Input> bool decodePosition(Input&stream,EtaFrame&out,const char*route,char bound,int service,const char*destination,std::time_t now){char direction[2]={bound,0};out.generated=0;for(int i=0;i<positionAnchors;i++)out.counts[i]=0;
std::string prefix;prefix.reserve(512);bool found=false;char c;
 while(prefix.size()<480&&stream.readBytes(&c,1)==1){prefix+=c;if((prefix.size()>=6&&prefix.compare(prefix.size()-6,6,"\"data\"")==0)){found=true;break;}}
 bool ok=false;
 if(found&&stream.token()==':'&&stream.token()=='['){
  DynamicJsonDocument meta(768);std::string header=prefix+":[]}";
  if(!deserializeJson(meta,header.c_str())){out.generated=parseISO(meta["generated_timestamp"]|"");}
  if(out.generated&&std::abs(double(now-out.generated))<=120){
   DynamicJsonDocument row(1536);StaticJsonDocument<256> filter;for(const char*k:{"route","dir","service_type","seq","eta_seq","dest_tc","eta","rmk_en","rmk_tc","data_timestamp"})filter[k]=true;
   int rows=0;
   while(stream.allowed()&&rows<2048){int next=stream.token();
    if(next==']'){ok=rows==0&&stream.token()=='}';break;}
    if(next!='{')break;stream.unread(next);
    row.clear();if(deserializeJson(row,stream,DeserializationOption::Filter(filter)))break;rows++;
    if(strcmp(row["route"]|"",route)==0&&strcmp(row["dir"]|"",direction)==0&&row["service_type"].as<int>()==service&&(!strlen(destination)||strcmp(row["dest_tc"]|"",destination)==0)){
     const char*en=row["rmk_en"]|"",*tc=row["rmk_tc"]|"";bool scheduled=strstr(en,"Scheduled")||strstr(tc,"原定");int rank=row["eta_seq"]|0;if(rank>=1&&rank<=3)out.add(row["seq"]|0,{parseISO(row["eta"]|""),parseISO(row["data_timestamp"]|""),scheduled,rank});
    }
    int delimiter=stream.token();if(delimiter==']'){ok=stream.token()=='}';break;}if(delimiter!=',')break;
   }
  }
 }
 return ok;
}
