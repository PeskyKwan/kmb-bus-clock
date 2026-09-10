#include "../src/eta_position.h"
#include <cassert>
#include <cstdio>
EtaFrame frame(time_t now){EtaFrame f;f.generated=now;f.layout.count=3;f.layout.meters=600;f.layout.anchors[0]={6,0};f.layout.anchors[1]={7,.5};f.layout.anchors[2]={8,1};f.add(6,{100000-40,now,false});f.add(7,{100000+60,now,false});f.add(8,{100000+160,now,false});return f;}
int main(){const time_t t=100000;float p;EtaPositionTracker tracker;auto a=frame(t);assert(!tracker.accept(a,t,t+160));assert(!tracker.estimate(t,t+160,p));assert(!tracker.accept(a,t+10,t+160));auto b=frame(t+30);assert(tracker.accept(b,t+30,t+160));assert(tracker.estimate(t+30,t+160,p));assert(std::abs(p-.35f)<.001);assert(tracker.estimate(t+70,t+160,p)&&p>.5);assert(!tracker.estimate(t+151,t+160,p));assert(!tracker.estimate(t+30,t+600,p));
 // Rank order changes do not matter; timestamps and compatible intervals do.
 tracker.reset();a=frame(t);a.add(6,{t+1000,t,false});a.add(7,{t+1100,t,false});tracker.accept(a,t,t+160);b=frame(t+30);assert(tracker.accept(b,t+30,t+160));
 // Scheduled-only rows never animate.
 tracker.reset();a=frame(t);for(int i=0;i<3;i++)a.rows[i][0].scheduled=true;assert(!tracker.accept(a,t,t+160));a.generated=t+30;assert(!tracker.accept(a,t+30,t+160));
 // Two plausible cars at the preceding stop are ambiguous.
 tracker.reset();a=frame(t);a.add(7,{t+30,t,false});assert(!tracker.accept(a,t,t+160));
 tracker.reset();a=frame(t);a.rows[2][0].rank=1;a.add(8,{t+160,t,false,2});assert(!tracker.accept(a,t,t+160));
 // A new bus gets a new warm-up rather than driving the old marker backwards.
 tracker.reset();a=frame(t);tracker.accept(a,t,t+160);b=frame(t+30);tracker.accept(b,t+30,t+160);for(int i=0;i<3;i++)b.rows[i][0].eta+=1200;b.generated=t+60;assert(!tracker.accept(b,t+60,t+1360));
 // Recently removed upstream ETA can be bridged ONLY from a matched history.
 tracker.reset();a=frame(t);tracker.accept(a,t,t+160);b=frame(t+30);b.counts[0]=0;assert(tracker.accept(b,t+30,t+160));assert(tracker.estimate(t+30,t+160,p));
 // Never draw an off-map vehicle at the map entrance.
 tracker.reset();a=frame(t);a.layout.anchors[0].fraction=-.5;tracker.accept(a,t,t+160);b=a;b.generated=t+10;for(int i=0;i<3;i++)b.rows[i][0].stamp=t+10;assert(tracker.accept(b,t+10,t+160));assert(tracker.estimate(t+10,t+160,p));assert(!tracker.estimate(t-10,t+160,p));
 puts("Multi-stop matching, warm-up, rank changes, schedule/ambiguity/staleness gates and history PASS");}
