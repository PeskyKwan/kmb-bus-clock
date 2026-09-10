#include "../src/eta_position.h"
#include "../src/eta_animation.h"
#include <ArduinoJson.h>
#include <cassert>
#include <fstream>
#include <iostream>

int main(){
 std::ifstream file("tests/fixtures/position_1a_2026-09-10.json");
 DynamicJsonDocument doc(65536);assert(!deserializeJson(doc,file));
 EtaPositionTracker tracker;int index=0;time_t lastNow=0,lastTarget=0;float fraction;
 const int expectedSamples[]={1,2,2,2,2,0,1,2};
 const bool expectedVisible[]={false,true,false,true,true,false,false,true};
 for(JsonObject snapshot:doc["frames"].as<JsonArray>()){
  EtaFrame frame;frame.generated=snapshot["generated"].as<time_t>();frame.layout.meters=doc["layout"]["meters"];
  for(auto anchor:doc["layout"]["anchors"].as<JsonArray>())frame.layout.anchors[frame.layout.count++]={anchor[0],anchor[1]};
  for(auto row:snapshot["rows"].as<JsonArray>())frame.add(row[0],{row[2].as<time_t>(),row[3].as<time_t>(),row[4],row[1]});
  auto now=snapshot["now"].as<time_t>(),target=snapshot["target"].as<time_t>();
  tracker.accept(frame,now,target);
  assert(tracker.samples()==expectedSamples[index]);
  assert(tracker.estimate(now,target,fraction)==expectedVisible[index]);
  lastNow=now;lastTarget=target;index++;
 }
 assert(index==8);
 // Between actual polls, two separately confirmed live brackets coexist.
 float first,second;assert(tracker.samples(1)==2);
 assert(tracker.estimate(lastNow+25,lastTarget,first));
 assert(tracker.estimate(1,lastNow+25,second));
 assert(first>second&&first<=1&&second>=0);
 ApproachMotion a,b;for(int i=0;i<10;i++){a.update(true,first,.1f);b.update(true,second,.1f);}
 assert(a.visible&&b.visible&&a.opacity==1&&b.opacity==1);
 // A travelled anchor can expire while the CURRENT bracket and path are fresh.
 assert(tracker.estimate(lastNow+71,lastTarget,first));
 // Path expiry still wins even if the main official ETA remains in the future.
 assert(!tracker.estimate(lastNow+91,lastTarget,first));
 for(int i=0;i<10;i++){a.update(false,0,.1f);b.update(false,0,.1f);}
 assert(!a.visible&&!b.visible);
 std::cout<<"Raw-provider replay: exact identity, conservative ambiguity, real two-bus brackets, expiry/fade PASS\n";
}
