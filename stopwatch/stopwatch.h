

#ifndef SRLM_PROPGCC_STOPWATCH_H_
#define SRLM_PROPGCC_STOPWATCH_H_



#ifndef UNIT_TEST
#include "propeller.h"
#else
extern unsigned int CNT;
extern unsigned int CLKFREQ;
#endif

class Stopwatch {
public:
    Stopwatch();
    void Reset(void);
    void Start(void);
    /**
    @returns the number of elapsed milliseconds since start.
     */
    int GetElapsed(void);

    bool GetStarted(void);
private:
    unsigned int startCNT;
    bool started;

};




#endif // SRLM_PROPGCC_STOPWATCH_H_
