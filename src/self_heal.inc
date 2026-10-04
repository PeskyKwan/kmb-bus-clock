#include "heal_policy.h"
// ETA self-heal: soft recovery without restart first; guarded restart last.
// Nothing here writes Preferences/NVS or changes the screen layout.
RTC_NOINIT_ATTR HealRecord healRecord;
HealPolicy heal;HealRecord dryRecord;
bool healDryRun=false,healRestartedByPolicy=false;
uint32_t healSkewMs=0,healWouldRestart=0,idleLargest8=UINT32_MAX;
unsigned long healHoldUntil=0,lastHealCheck=0;int healStage=0;
volatile uint32_t staDisconnects=0,staGotIp=0; // Wi-Fi event counters prove STA recovery
uint32_t healNow(){return millis()+healSkewMs;}
bool healHolding(){return (long)(healHoldUntil-millis())>0;}
bool networkIdle(){return !requestBusy&&!nativeBusy&&!nativePending&&!wifiWorkActive();}
void healWifiEvents(){WiFi.onEvent([](arduino_event_id_t){staDisconnects++;},ARDUINO_EVENT_WIFI_STA_DISCONNECTED);WiFi.onEvent([](arduino_event_id_t){staGotIp++;},ARDUINO_EVENT_WIFI_STA_GOT_IP);}
void healBoot(){
 if(!healRecordValid(healRecord)){memset(&healRecord,0,sizeof(healRecord));return;}
 if(healRecord.pending){healRestartedByPolicy=esp_reset_reason()==ESP_RST_SW;healRecord.pending=0;healRecordSeal(healRecord);}
}
void emitHeal(const char*action){Serial.printf("{\"event\":\"heal\",\"action\":\"%s\",\"uptimeS\":%u,\"streak\":%d,\"softAttempts\":%d,\"softRecoveries\":%u,\"failAgeS\":%u,\"largest8\":%u,\"idleLargest8\":%u,\"block\":%d,\"dryRun\":%s}\n",action,uptimeSeconds(),heal.streak,heal.softAttempts,heal.softTotal,heal.streak?(healNow()-heal.firstFailMs)/1000:0,largest8(),idleLargest8==UINT32_MAX?0:idleLargest8,(int)heal.lastBlock,healDryRun?"true":"false");}
// Requests open and close their own clients, so recovery pauses other network
// jobs, drops the route-stop scratch list once the map is built, optionally
// restarts Wi-Fi STA with the saved credentials, then retries ETA promptly.
void softRecover(bool restartSta){
 healStage=restartSta?2:1;healHoldUntil=millis()+90000;
 if(activeRoad.valid&&activeRoad.gen==cfg.gen){std::vector<RoadStopCandidate>().swap(cachedRoadStops);}
 if(restartSta&&wifiName.length()&&!wifiWorkActive()){WiFi.disconnect(false,false);restoreWifi();}
 failures=0;lastPoll=millis()-25000;emitHeal(restartSta?"soft-sta":"soft");
}
void healOnEta(int code,bool transport){heal.onEta(code>=1,transport,healNow());if(code>=1)healStage=0;}
void healCheck(bool force=false){
 if(!force&&millis()-lastHealCheck<5000)return;lastHealCheck=millis();
 if(!networkIdle())return;
 if(!tlsPool.ready())tlsPoolReserve();
 idleLargest8=largest8();
 HealRecord&record=healDryRun?dryRecord:healRecord;time_t now=time(nullptr);
 HealAction action=heal.decide(healNow(),idleLargest8,now>1700000000,now,record);
 if(action==HEAL_SOFT||action==HEAL_SOFT_STA){softRecover(action==HEAL_SOFT_STA);return;}
 if(heal.exhausted())healStage=3;
 if(action!=HEAL_RESTART)return;
 healRecordNote(record,now);
 if(healDryRun){healWouldRestart++;emitHeal("would-restart");return;}
 emitHeal("restart");Serial.flush();delay(200);ESP.restart();
}
void healFields(JsonOut&d){
 const uint32_t now=healNow();const bool valid=healRecordValid(healRecord);
 d.put("etaFailStreak",heal.streak);d.put("etaFailAgeS",heal.streak?(now-heal.firstFailMs)/1000:0);d.put("lastEtaOkAgeS",heal.everOk?(long)((now-heal.lastOkMs)/1000):-1);
 d.put("softRecoveries",heal.softTotal);d.put("softAttempts",heal.softAttempts);d.put("lastSoftAgeS",heal.softTotal?(long)((now-heal.lastSoftMs)/1000):-1);d.put("healStage",healStage);d.put("healHold",healHolding());d.put("idleLargest8",idleLargest8==UINT32_MAX?0:idleLargest8);
 d.put("restartReason",healRestartedByPolicy?"self-heal":resetReasonName(esp_reset_reason()));d.put("healRestarts",valid?healRecord.restarts:0);
 time_t wall=time(nullptr);d.put("lastHealRestartAgoS",valid&&healRecord.lastRestart&&wall>1700000000?(long long)(wall-healRecord.lastRestart):-1);d.put("restartBlock",(int)heal.lastBlock);
 d.put("staDisconnects",staDisconnects);d.put("staGotIp",staGotIp);
 d.put("healDryRun",healDryRun);d.put("wouldRestart",healWouldRestart);d.put("simFailures",healSimFailures);
}
// Hidden USB test hooks: simulate TLS allocation failures, advance the policy
// clock and keep the last-resort restart as a dry run. Never persisted.
void healCommand(JsonDocument&d){
 // Reset also ends the test episode so a rewound clock cannot look like a long outage.
 if(d["reset"]|false){healSimFailures=0;healDryRun=false;healSkewMs=0;heal.streak=0;heal.softAttempts=0;heal.lastBlock=HEAL_ALLOWED;healStage=0;}
 if(d.containsKey("fail"))healSimFailures=constrain(d["fail"].as<int>(),0,50);
 if(d.containsKey("dryRun")){healDryRun=d["dryRun"].as<bool>();if(healDryRun)dryRecord=healRecord;}
 if(d.containsKey("skipS"))healSkewMs+=(uint32_t)constrain(d["skipS"].as<long>(),0L,86400L)*1000UL;
 if(d["clearRecord"]|false)memset(&healRecord,0,sizeof(healRecord));
 if(d["poll"]|false){failures=0;lastPoll=millis()-30001UL;}
 JsonOut out(Serial);out.put("event","healState");out.put("uptimeS",uptimeSeconds());healFields(out);out.end();Serial.println();
}
