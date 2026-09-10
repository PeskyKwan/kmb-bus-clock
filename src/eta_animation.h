#pragma once
#include <ctime>
#include <cmath>
#include <algorithm>
inline bool animationFresh(bool connected,int code,std::time_t now,std::time_t stamp,std::time_t eta){return connected&&code==2&&stamp>0&&std::abs(double(now-stamp))<=120&&eta>=now-30&&eta<=now+10800;}
// Rendering only: position comes from matched multi-stop forecasts, never a fixed timer.
struct ApproachMotion{
 bool visible=false;float fraction=0,opacity=0;
 void reset(){visible=false;fraction=opacity=0;}
 void update(bool valid,float position,float seconds){seconds=std::max(0.f,std::min(seconds,1.f));
  if(valid){position=std::max(0.f,std::min(position,1.f));if(!visible||position<fraction-.03f||position>fraction+.2f){fraction=position;opacity=0;}else{float step=seconds*.2f;fraction+=std::max(-step,std::min(step,position-fraction));}opacity=std::min(1.f,opacity+seconds*2);}
  else opacity=std::max(0.f,opacity-seconds*3);visible=opacity>.01f;
 }
};
