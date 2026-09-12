#pragma once
#include <ctime>
#include <cmath>
#include <algorithm>
inline bool animationFresh(bool connected,int code,std::time_t now,std::time_t stamp,std::time_t eta){return connected&&code==2&&stamp>0&&std::abs(double(now-stamp))<=120&&eta>=now-30&&eta<=now+10800;}
// Preserve an illustration through one transient empty response, but not
// indefinitely after the provider repeatedly confirms no forecast exists.
struct EtaAvailabilityGate{
 unsigned char consecutiveEmpty=0;
 void reset(){consecutiveEmpty=0;}
 bool update(int code){
  if(code==1){if(consecutiveEmpty<2)consecutiveEmpty++;return consecutiveEmpty>=2;}
  consecutiveEmpty=0;return false;
 }
};
// Rendering only: callers choose matched, approximate or held positions.
struct ApproachMotion{
 bool visible=false;float fraction=0,opacity=0;
 void reset(){visible=false;fraction=opacity=0;}
 void update(bool valid,float position,float seconds,bool continuous=false){seconds=std::max(0.f,std::min(seconds,1.f));
  if(valid){position=std::max(0.f,std::min(position,1.f));if(!visible||position<fraction-.03f||position>fraction+.2f){fraction=position;if(!continuous||!visible)opacity=0;}else{float step=seconds*.2f;fraction+=std::max(-step,std::min(step,position-fraction));}opacity=std::min(1.f,opacity+seconds*2);}
  else opacity=std::max(0.f,opacity-seconds*3);visible=opacity>.01f;
 }
};

// User-authorized illustration fallback, separate from the strict ETA matcher.
// Never used by the official ETA or alerts; the map remains labelled "估算".
struct PersistentApproach{
 bool initialized=false,matchedBefore=false;float fraction=0;std::time_t lastTarget=0;
 enum Mode { NONE, ETA_GUESS, MATCHED, HELD };Mode mode=NONE;
 void reset(){*this=PersistentApproach{};}
 bool update(bool fresh,std::time_t now,std::time_t target,float meters,bool matched,float position){
  // Once the old arrival is due, a substantially later ETA denotes the next
  // illustrated journey. Do not leave that next bus parked at the boarding stop.
  if(fresh&&lastTarget&&lastTarget<=now+30&&target>lastTarget+90)reset();
  if(fresh)lastTarget=target;
  if(matched){fraction=std::max(0.f,std::min(position,1.f));initialized=matchedBefore=true;mode=MATCHED;}
  else if(fresh&&!matchedBefore){
   // Nominal 20 km/h only seeds an illustrative location. Far-away forecasts
   // wait near the map entrance; this is not a measured speed or GPS location.
   const float travel=std::max(60.f,meters/(20.f/3.6f));
   fraction=std::max(.05f,std::min(.98f,1.f-float(target-now)/travel));initialized=true;mode=ETA_GUESS;
  }else if(initialized)mode=HELD;
  else mode=NONE;
  return initialized;
 }
 const char* status()const{return mode==ETA_GUESS?"eta-guess":mode==MATCHED?"matched":mode==HELD?"held":"none";}
};

inline bool standbyIllustration(bool scheduled,PersistentApproach::Mode mode,float fraction){return scheduled&&mode==PersistentApproach::ETA_GUESS&&fraction<=.06f;}

inline void alignIllustratedPair(PersistentApproach&first,PersistentApproach&second,std::time_t now,bool fresh,std::time_t target,bool nextFresh,std::time_t next){
 if(fresh&&first.lastTarget&&first.lastTarget<=now+30&&target>first.lastTarget+90){
  // Promote the known following journey; never leave a duplicate of it behind.
  if(second.initialized&&std::abs(double(target-second.lastTarget))<=90)first=second;
  else first.reset();
  second.reset();
 }
 if(nextFresh&&second.lastTarget&&std::abs(double(next-second.lastTarget))>90)second.reset();
}

// Two 22x26 sprites must remain distinguishable even when both estimates wait
// at the map entrance. Offset only the artwork, not the route fractions.
inline void separateMarkerCenters(int&x1,int&y1,int&x2,int&y2){
 if(std::abs(x1-x2)>=24||std::abs(y1-y2)>=28)return;
 const bool firstLeft=x1<=x2;const int center=std::max(157,std::min(284,(x1+x2)/2));
 x1=center+(firstLeft?-12:12);x2=center+(firstLeft?12:-12);
}
