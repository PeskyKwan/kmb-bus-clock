#include "../src/daylight.h"
#include <cassert>
#include <cstdio>
int main(){auto sep=solarMinutes(252,22.3,114.17);assert(abs(sep.rise-368)<5&&abs(sep.set-1113)<5);auto winter=solarMinutes(355,22.3,114.17);auto summer=solarMinutes(172,22.3,114.17);assert(summer.set-summer.rise>winter.set-winter.rise);for(int d=1;d<=366;d++){auto s=solarMinutes(d,22.36,114.26);assert(s.rise>300&&s.rise<450&&s.set>1000&&s.set<1200);}printf("Solar window PASS: September 9 %02d:%02d-%02d:%02d HK; seasons and leap day covered\n",sep.rise/60,sep.rise%60,sep.set/60,sep.set%60);}
