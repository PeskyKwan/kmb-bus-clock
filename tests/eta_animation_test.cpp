#include "../src/eta_animation.h"
#include <cassert>
#include <cstdio>
int main(){const time_t t=100000;int n=0;
#define CHECK(x) do{assert(x);n++;}while(0)
 CHECK(animationFresh(true,2,t,t-20,t+240));
 CHECK(!animationFresh(false,2,t,t,t+240));
 CHECK(!animationFresh(true,1,t,t,0));
 CHECK(!animationFresh(true,2,t,t-121,t+240));
 CHECK(!animationFresh(true,2,t,t+121,t+240));
 CHECK(!animationFresh(true,2,t,t,t-31));
 CHECK(!animationFresh(true,2,t,t,t+10801));
 CHECK(approachFraction(t,t+1200)==0);
 CHECK(std::abs(approachFraction(t,t+240)-.6f)<.0001f);
 CHECK(approachFraction(t,t)==1);
 CHECK(approachFraction(t,t-20)==1);
 ApproachMotion m;m.update(true,t,t+240,.1);CHECK(m.visible&&std::abs(m.fraction-.6)<.0001);
 m.update(true,t+60,t+240,.1);CHECK(m.fraction>.6f&&m.fraction<.7f);
 m.update(false,t+61,t+240,.1);CHECK(!m.visible&&m.previousEta==0);
 m.update(true,t,t+60,.1);m.update(true,t,t+1800,.1);CHECK(m.fraction==0);
 m.reset();CHECK(!m.visible);
 printf("%d ETA animation checks passed\n",n);
}
