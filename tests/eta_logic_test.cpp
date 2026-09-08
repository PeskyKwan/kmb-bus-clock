#include <initializer_list>
#include <cassert>
#include <cstdio>
#include "../src/eta_logic.h"
int main(){setenv("TZ","UTC0",1);tzset();auto now=parseISO("2026-09-09T08:00:00+08:00");assert(now==parseISO("2026-09-09T00:00:00+00:00"));assert(!parseISO(nullptr));assert(!parseISO(""));assert(!parseISO("2026-09-09T08:00:00+88:00"));assert(arrivalAlert(true,true,2,now,now-20,now+300,5));assert(!arrivalAlert(true,true,2,now,now-121,now+300,5));assert(!arrivalAlert(true,true,2,now,now,now+301,5));assert(!arrivalAlert(true,true,1,now,now,0,5));assert(!arrivalAlert(true,false,2,now,now,now+60,5));assert(!arrivalAlert(false,true,2,now,now,now+60,5));assert(!arrivalAlert(true,true,2,now,now,now-1,5));assert(!arrivalAlert(true,true,2,now,now+121,now+60,5));puts("12 ETA/time/alert boundary checks passed");}
