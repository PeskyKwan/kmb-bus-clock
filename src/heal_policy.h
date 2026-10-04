#pragma once
#include <cstdint>
// ETA self-heal: soft recovery first, guarded restart only as a last resort.
// Pure logic so thresholds, cooldowns and the boot-loop guard are host-tested.
enum HealAction{HEAL_NONE,HEAL_SOFT,HEAL_SOFT_STA,HEAL_RESTART};
enum HealBlock{HEAL_ALLOWED=0,HEAL_BLOCK_CLOCK,HEAL_BLOCK_SPACING,HEAL_BLOCK_FUTURE};
struct HealLimits{
 int softAfter=3;                 // consecutive transport failures before soft recovery
 uint32_t lowLargest=8192;        // idle byte-heap block below this also allows soft recovery after one failure
 uint32_t softCooldownMs=180000;  // between soft attempts
 int maxSoft=3;                   // soft attempts per failure episode
 int staFrom=2;                   // attempt number that also restarts Wi-Fi STA
 uint32_t restartAfterMs=1500000; // 25 minutes of continuous failure
 int64_t restartSpacingS=21600;   // at most one guarded restart per 6 hours
};
// Kept in RTC memory: survives ESP.restart() but not power loss. Checksum rejects garbage.
struct HealRecord{uint32_t magic;uint32_t restarts;int64_t lastRestart;uint32_t pending;uint32_t check;};
constexpr uint32_t healMagic=0x4B4D4248;
inline uint32_t healChecksum(const HealRecord&r){uint64_t t=(uint64_t)r.lastRestart;return r.magic^(r.restarts*2654435761u)^(uint32_t)t^(uint32_t)(t>>32)^(r.pending*40503u)^0xA5A5A5A5u;}
inline bool healRecordValid(const HealRecord&r){return r.magic==healMagic&&r.check==healChecksum(r);}
inline void healRecordSeal(HealRecord&r){r.magic=healMagic;r.check=healChecksum(r);}
inline void healRecordNote(HealRecord&r,int64_t epoch){if(!healRecordValid(r))r=HealRecord{};r.restarts++;r.lastRestart=epoch;r.pending=1;healRecordSeal(r);}
// Boot-loop guard: needs real time and restartSpacingS since the previous guarded restart.
inline HealBlock restartGuard(const HealRecord&r,bool clockSynced,int64_t epoch,const HealLimits&l=HealLimits{}){
 if(!clockSynced)return HEAL_BLOCK_CLOCK;
 if(!healRecordValid(r)||!r.lastRestart)return HEAL_ALLOWED;
 if(r.lastRestart>epoch+300)return HEAL_BLOCK_FUTURE;
 return epoch-r.lastRestart>=l.restartSpacingS?HEAL_ALLOWED:HEAL_BLOCK_SPACING;
}
class HealPolicy{
 public:
 HealLimits limits;
 int streak=0,softAttempts=0;uint32_t firstFailMs=0,lastOkMs=0,lastSoftMs=0,softTotal=0;bool everOk=false;HealBlock lastBlock=HEAL_ALLOWED;
 // ok: a current KMB response was accepted (forecast or no forecast).
 // transport: the HTTP/TLS client failed; server/data errors are not device faults.
 void onEta(bool ok,bool transport,uint32_t now){
  if(ok){streak=0;softAttempts=0;lastOkMs=now;everOk=true;lastBlock=HEAL_ALLOWED;return;}
  if(!transport){streak=0;softAttempts=0;return;}
  if(!streak)firstFailMs=now;streak++;
 }
 bool exhausted()const{return streak&&softAttempts>=limits.maxSoft;}
 HealAction decide(uint32_t now,uint32_t idleLargest,bool clockSynced,int64_t epoch,const HealRecord&r){
  if(!streak)return HEAL_NONE;
  const bool pressured=streak>=limits.softAfter||idleLargest<limits.lowLargest;
  if(pressured&&softAttempts<limits.maxSoft&&(!softAttempts||now-lastSoftMs>=limits.softCooldownMs)){
   softAttempts++;softTotal++;lastSoftMs=now;return softAttempts>=limits.staFrom?HEAL_SOFT_STA:HEAL_SOFT;
  }
  if(exhausted()&&now-lastSoftMs>=limits.softCooldownMs&&now-firstFailMs>=limits.restartAfterMs){
   lastBlock=restartGuard(r,clockSynced,epoch,limits);if(lastBlock==HEAL_ALLOWED)return HEAL_RESTART;
  }
  return HEAL_NONE;
 }
};
