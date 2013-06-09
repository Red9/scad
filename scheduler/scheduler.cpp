#include "scheduler.h"

#ifdef UNIT_TEST
#define CNT unit_CNT
#define CLKFREQ unit_CLKFREQ
extern unsigned int unit_CNT;
extern unsigned int unit_CLKFREQ;
#endif

Scheduler::Scheduler(int deci_hz) {
    
    periodTicks = GetTicksPerPeriod(deci_hz);
    startCNT = CNT;
}

bool Scheduler::Run() {
    if ((CNT - startCNT) >= periodTicks) {
        startCNT += periodTicks;
        return true;
    } else {
        return false;
    }
}

unsigned int Scheduler::GetTicksPerPeriod(int deci_hz) {
    return (CLKFREQ * 10) / deci_hz;
}