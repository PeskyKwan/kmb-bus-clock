#include "../src/eta_animation.h"
#include <cassert>
#include <cstdio>
int main(){const time_t t=100000;assert(animationFresh(true,2,t,t-20,t+240));assert(!animationFresh(false,2,t,t,t+240));assert(!animationFresh(true,1,t,t,0));assert(!animationFresh(true,2,t,t-121,t+240));assert(!animationFresh(true,2,t,t+121,t+240));assert(!animationFresh(true,2,t,t,t-31));assert(!animationFresh(true,2,t,t,t+10801));ApproachMotion m;m.update(true,.4f,.1f);assert(m.visible&&m.fraction==.4f&&m.opacity<1);m.update(true,.41f,.1f);assert(m.fraction>.4f);for(int i=0;i<10;i++)m.update(false,0,.1f);assert(!m.visible);m.update(true,.8,.1);m.update(true,.3,.1);assert(m.fraction==.3f&&m.opacity<=.21f);m.reset();assert(!m.visible);puts("Freshness, data-position smoothing, correction and fade PASS");}
