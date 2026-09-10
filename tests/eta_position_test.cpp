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
 // Two ordered, independently supported buses can be shown after two provider updates.
 tracker.reset();a=frame(t);a.add(6,{t+60,t,false,2});a.add(7,{t+210,t,false,2});a.add(8,{t+360,t,false,2});assert(!tracker.accept(a,t,t+160));b=a;b.generated=t+30;for(int i=0;i<3;i++)for(int j=0;j<b.counts[i];j++)b.rows[i][j].stamp=t+30;assert(tracker.accept(b,t+30,t+160));float second=0;assert(tracker.count()==2&&tracker.samples(1)==2);assert(tracker.estimate(t+80,t+160,p));assert(tracker.estimate(1,t+80,second));assert(p>.5f&&second>0&&second<.5f);
 // A scheduled second journey never becomes a second marker.
 tracker.reset();a=frame(t);a.add(6,{t+60,t,true,2});a.add(7,{t+210,t,true,2});a.add(8,{t+360,t,true,2});tracker.accept(a,t,t+160);b=a;b.generated=t+30;for(int i=0;i<3;i++)for(int j=0;j<b.counts[i];j++)b.rows[i][j].stamp=t+30;assert(tracker.accept(b,t+30,t+160));assert(tracker.count()==1&&!tracker.estimate(1,t+80,second));
 // Normalized replay of two live 2A inbound chains across an eleven-minute window.
 tracker.reset();EtaFrame dense;dense.generated=t;dense.layout.count=8;dense.layout.meters=3000;int firstEta[8]={-565,-412,-318,-223,-128,-40,68,173},secondEta[8]={-24,129,222,327,421,528,648,753};for(int i=0;i<8;i++){dense.layout.anchors[i]={17+i,i/7.f};dense.add(17+i,{t+firstEta[i],t,false,1});dense.add(17+i,{t+secondEta[i],t,false,2});}assert(!tracker.accept(dense,t,t+173));auto dense2=dense;dense2.generated=t+30;for(int i=0;i<8;i++)for(int j=0;j<dense2.counts[i];j++)dense2.rows[i][j].stamp=t+30;assert(tracker.accept(dense2,t+30,t+173));assert(tracker.count()==2&&tracker.estimate(t+45,t+173,p)&&tracker.estimate(1,t+45,second));assert(p>second);
 puts("Multi-stop matching, two-bus ordering, warm-up, rank changes, schedule/ambiguity/staleness gates and history PASS");}
