#include <cassert>
#include <cstdio>
#include <cstring>
#include "../src/heal_policy.h"
constexpr uint32_t big=49140;constexpr int64_t epoch=1790000000;
HealRecord none(){HealRecord r;memset(&r,0,sizeof(r));return r;}
int main(){
 const HealRecord empty=none();
 // Soft recovery waits for three consecutive transport failures.
 {HealPolicy p;assert(p.decide(0,big,true,epoch,empty)==HEAL_NONE);p.onEta(false,true,1000);assert(p.decide(1000,big,true,epoch,empty)==HEAL_NONE);p.onEta(false,true,61000);assert(p.decide(61000,big,true,epoch,empty)==HEAL_NONE);p.onEta(false,true,181000);assert(p.streak==3);assert(p.decide(181000,big,true,epoch,empty)==HEAL_SOFT);assert(p.softAttempts==1&&p.softTotal==1);assert(p.decide(181001,big,true,epoch,empty)==HEAL_NONE);}
 // Server/data failures and any accepted response end the episode.
 {HealPolicy p;p.onEta(false,true,0);p.onEta(false,true,1);p.onEta(false,false,2);assert(p.streak==0);p.onEta(false,true,3);assert(p.streak==1&&p.firstFailMs==3);p.onEta(false,true,4);p.onEta(false,true,5);assert(p.decide(5,big,true,epoch,empty)==HEAL_SOFT);p.onEta(true,false,6);assert(p.streak==0&&p.softAttempts==0&&p.everOk&&p.lastOkMs==6);assert(p.decide(7,big,true,epoch,empty)==HEAL_NONE);}
 // Cooldown, Wi-Fi STA restart from the second attempt, bounded attempts.
 {HealPolicy p;for(int i=0;i<3;i++)p.onEta(false,true,i);assert(p.decide(10,big,true,epoch,empty)==HEAL_SOFT);p.onEta(false,true,20);assert(p.decide(179999+10-1,big,true,epoch,empty)==HEAL_NONE);assert(p.decide(180010,big,true,epoch,empty)==HEAL_SOFT_STA);assert(p.decide(360009,big,true,epoch,empty)==HEAL_NONE);assert(p.decide(360010,big,true,epoch,empty)==HEAL_SOFT_STA);assert(p.exhausted());assert(p.decide(540010,big,true,epoch,empty)==HEAL_NONE);assert(p.softTotal==3);}
 // Low idle byte-heap block lets one transport failure start soft recovery; no failure means no action.
 {HealPolicy p;assert(p.decide(0,4000,true,epoch,empty)==HEAL_NONE);p.onEta(false,true,0);assert(p.decide(0,8192,true,epoch,empty)==HEAL_NONE);assert(p.decide(0,8191,true,epoch,empty)==HEAL_SOFT);}
 // Restart only after soft recovery is exhausted, its cooldown passed and 25 minutes of failure.
 {HealPolicy p;for(int i=0;i<3;i++)p.onEta(false,true,0);uint32_t t=0;assert(p.decide(t,big,true,epoch,empty)==HEAL_SOFT);t+=180000;assert(p.decide(t,big,true,epoch,empty)==HEAL_SOFT_STA);t+=180000;assert(p.decide(t,big,true,epoch,empty)==HEAL_SOFT_STA);
  assert(p.decide(t+179999,big,true,epoch,empty)==HEAL_NONE);assert(p.decide(1499999,big,true,epoch,empty)==HEAL_NONE);
  assert(p.decide(1500000,big,false,epoch,empty)==HEAL_NONE&&p.lastBlock==HEAL_BLOCK_CLOCK);
  assert(p.decide(1500000,big,true,epoch,empty)==HEAL_RESTART);}
 // Restart is never chosen while soft attempts remain, even after a long outage.
 {HealPolicy p;p.onEta(false,true,0);p.onEta(false,true,0);assert(p.decide(3000000,big,true,epoch,empty)==HEAL_NONE);p.onEta(false,true,3000000);assert(p.decide(3000000,big,true,epoch,empty)==HEAL_SOFT);assert(p.decide(3000001,big,true,epoch,empty)==HEAL_NONE);}
 // Boot-loop guard: once per six hours, synced clock, garbage or future records handled.
 {HealRecord r=none();assert(!healRecordValid(r));assert(restartGuard(r,true,epoch)==HEAL_ALLOWED);assert(restartGuard(r,false,epoch)==HEAL_BLOCK_CLOCK);
  healRecordNote(r,epoch);assert(healRecordValid(r)&&r.restarts==1&&r.pending==1&&r.lastRestart==epoch);
  assert(restartGuard(r,true,epoch+60)==HEAL_BLOCK_SPACING);assert(restartGuard(r,true,epoch+21599)==HEAL_BLOCK_SPACING);assert(restartGuard(r,true,epoch+21600)==HEAL_ALLOWED);
  assert(restartGuard(r,true,epoch-301)==HEAL_BLOCK_FUTURE);assert(restartGuard(r,true,epoch-300)==HEAL_BLOCK_SPACING);
  healRecordNote(r,epoch+21600);assert(r.restarts==2&&healRecordValid(r));
  r.pending=0;assert(!healRecordValid(r));healRecordSeal(r);assert(healRecordValid(r));
  HealRecord junk;memset(&junk,0x5A,sizeof(junk));assert(!healRecordValid(junk));assert(restartGuard(junk,true,epoch)==HEAL_ALLOWED);healRecordNote(junk,epoch);assert(junk.restarts==1&&healRecordValid(junk));
  HealRecord tampered=r;tampered.lastRestart-=21600;assert(!healRecordValid(tampered));}
 // A recorded restart blocks the next one inside the window (dry-run style re-entry).
 {HealPolicy p;HealRecord r=none();for(int i=0;i<3;i++)p.onEta(false,true,0);p.decide(0,big,true,epoch,r);p.decide(180000,big,true,epoch,r);p.decide(360000,big,true,epoch,r);assert(p.decide(1500000,big,true,epoch,r)==HEAL_RESTART);healRecordNote(r,epoch);assert(p.decide(1500001,big,true,epoch+1,r)==HEAL_NONE&&p.lastBlock==HEAL_BLOCK_SPACING);assert(p.decide(1500002,big,true,epoch+21600,r)==HEAL_RESTART);}
 // millis() wrap-around keeps durations correct.
 {HealPolicy p;const uint32_t start=0xFFFFF000u;for(int i=0;i<3;i++)p.onEta(false,true,start);assert(p.decide(start,big,true,epoch,empty)==HEAL_SOFT);assert(p.decide(start+180000u,big,true,epoch,empty)==HEAL_SOFT_STA);assert(p.decide(start+360000u,big,true,epoch,empty)==HEAL_SOFT_STA);assert(p.decide(start+1499999u,big,true,epoch,empty)==HEAL_NONE);assert(p.decide(start+1500000u,big,true,epoch,empty)==HEAL_RESTART);}
 puts("Self-heal thresholds, cooldowns, restart guard and wrap-around checks passed");
}
