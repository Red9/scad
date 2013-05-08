#include "stopwatch.h"

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