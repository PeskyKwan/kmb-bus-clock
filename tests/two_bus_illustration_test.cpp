#include "../src/eta_animation.h"
#include "../src/eta_logic.h"
#include <cassert>
#include <iostream>

int main(){
 const time_t now=100000;time_t next=0,stamp=0;
 considerFollowingEta(now,now+100,now+100,now,next,stamp);assert(!next);
 considerFollowingEta(now,now+100,now+400,now-121,next,stamp);assert(!next);
 considerFollowingEta(now,now+100,now+500,now,next,stamp);
 considerFollowingEta(now,now+100,now+150,now,next,stamp);assert(next==now+150);
 considerFollowingEta(now,now+100,now+180,now,next,stamp);assert(next==now+150);
 considerFollowingEta(now,now+100,now+10801,now,next,stamp);assert(next==now+150);
 // Scheduled/ambiguous matching is irrelevant to two official-ETA illustrations.
 PersistentApproach first,second;
 first.update(true,now,now+900,3000,false,0);
 second.update(true,now,now+1500,3000,false,0);
 assert(first.initialized&&second.initialized&&first.fraction==.05f&&second.fraction==.05f);
 first.update(true,now,now+900,3000,true,.8f);
 second.update(true,now,now+1500,3000,true,.3f);
 first.update(false,now+200,0,3000,false,0);
 second.update(false,now+200,0,3000,false,0);
 assert(first.mode==PersistentApproach::HELD&&second.mode==PersistentApproach::HELD);
 assert(first.fraction==.8f&&second.fraction==.3f);
 ApproachMotion a,b;for(int i=0;i<10;i++){a.update(true,.8f,.1f,true);b.update(true,.3f,.1f,true);}
 second.update(true,now+201,now+1500,3000,true,.65f);b.update(true,second.fraction,.1f,true);
 assert(b.visible&&b.opacity==1&&b.fraction==.65f);
 // First arrival rolls over to the previous second: promote, then seed third.
 alignIllustratedPair(first,second,now+901,true,now+1500,true,now+2100);
 assert(first.initialized&&first.fraction==.65f&&!second.initialized);
 first.update(true,now+901,now+1500,3000,false,0);
 second.update(true,now+901,now+2100,3000,false,0);
 assert(first.mode==PersistentApproach::HELD&&second.mode==PersistentApproach::ETA_GUESS);
 // A rollover with only one supplied forecast must not invent a second bus.
 alignIllustratedPair(first,second,now+1501,true,now+2100,false,0);
 assert(first.initialized&&!second.initialized);
 assert(!second.update(false,now+1501,0,3000,false,0));
 first.reset();second.reset();assert(!first.initialized&&!second.initialized);
 // Even at the same point or near an edge, two full sprites cannot cover each other.
 for(int x1=145;x1<=296;x1+=7)for(int x2=145;x2<=296;x2+=7)
 for(int y1=48;y1<=161;y1+=13)for(int y2=48;y2<=161;y2+=13){
  int a=x1,b=y1,c=x2,d=y2;separateMarkerCenters(a,b,c,d);
  assert(a>=145&&a<=296&&c>=145&&c<=296&&b==y1&&d==y2);
  assert(std::abs(a-c)>=24||std::abs(b-d)>=28);
 }
 std::cout<<"Two ETA illustrations, persistence/recovery, arrival promotion, no invented second and sprite separation PASS\n";
}
