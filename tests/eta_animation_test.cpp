#include "../src/eta_animation.h"
#include <cassert>
#include <cstdio>
int main(){const time_t t=100000;assert(animationFresh(true,2,t,t-20,t+240));assert(!animationFresh(false,2,t,t,t+240));assert(!animationFresh(true,1,t,t,0));assert(!animationFresh(true,2,t,t-121,t+240));assert(!animationFresh(true,2,t,t+121,t+240));assert(!animationFresh(true,2,t,t,t-31));assert(!animationFresh(true,2,t,t,t+10801));ApproachMotion m;m.update(true,.4f,.1f);assert(m.visible&&m.fraction==.4f&&m.opacity<1);m.update(true,.41f,.1f);assert(m.fraction>.4f);for(int i=0;i<10;i++)m.update(false,0,.1f);assert(!m.visible);m.update(true,.8,.1);m.update(true,.3,.1);assert(m.fraction==.3f&&m.opacity<=.21f);m.reset();assert(!m.visible);
 PersistentApproach p;assert(!p.update(false,t,0,3000,false,0));
 assert(p.update(true,t,t+1200,3000,false,0)&&p.mode==PersistentApproach::ETA_GUESS&&p.fraction==.05f);
 assert(p.update(true,t+900,t+1200,3000,false,0)&&p.fraction>.4f);
 assert(p.update(true,t+900,t+1200,3000,true,.75f)&&p.mode==PersistentApproach::MATCHED);
 assert(p.update(true,t+930,t+1200,3000,false,0)&&p.fraction==.75f&&p.mode==PersistentApproach::HELD);
 assert(p.update(false,t+1000,0,3000,false,0)&&p.fraction==.75f);
 assert(p.update(false,t+5000,0,3000,false,0)&&p.fraction==.75f);
 assert(p.update(true,t+1001,t+1200,3000,true,.3f)&&p.fraction==.3f);
 for(int i=0;i<10;i++)m.update(true,.75f,.1f,true);
 m.update(true,p.fraction,.1f,true);assert(m.visible&&m.opacity==1&&m.fraction==.3f);
 assert(p.update(true,t+1201,t+1800,3000,false,0)&&p.mode==PersistentApproach::ETA_GUESS&&p.fraction==.05f);
 p.reset();assert(!p.update(false,t+1201,0,3000,false,0));
 puts("Freshness, smoothing/fade, persistent ETA guess/hold/recovery/next-bus/reset PASS");}
