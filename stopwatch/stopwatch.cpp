#include "stopwatch.h"

#ifdef UNIT_TEST
#define CNT unit_CNT
#define CLKFREQ unit_CLKFREQ
extern unsigned int unit_CNT;
extern unsigned int unit_CLKFREQ;
#endif

Stopwatch::Stopwatch(void) {
    Reset();
}

void Stopwatch::Reset(void) {
    started = false;
}

void Stopwatch::Start(void) {
    startCNT = CNT;
    started = true;
}

int Stopwatch::GetElapsed(void) {
    if (started == true) {
        return (CNT - startCNT) / (CLKFREQ / 1000);
    } else {
        return 0;
    }
}

bool Stopwatch::GetStarted(void) {
    return started;
}