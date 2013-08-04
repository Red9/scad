/** Utility class for timing duration of certain events.
 * 
 * @warning The maximum time that can be recorded is (2^32/CLKFREQ) seconds. At
 * 80Mhz that is a bit over 53 seconds. Longer durations will rollover and make
 * the stopwatch operate incorrectly.
 * 
 * @author srlm (srlm@srlmproductions.com)
 * 
 */

#ifndef SRLM_PROPGCC_STOPWATCH_H_
#define SRLM_PROPGCC_STOPWATCH_H_


#ifndef UNIT_TEST
#include <propeller.h>
#endif

// Yes, there is an compile warning here for redefine...
// It's because Unity includes propeller.h, which has CNT and CLKFREQ defined.
#ifdef UNIT_TEST
#define CNT unit_CNT
#define CLKFREQ unit_CLKFREQ
extern unsigned int unit_CNT;
extern unsigned int unit_CLKFREQ;
#endif

class Stopwatch {
public:

    /**
     * 
     */
    Stopwatch() {
        Start(); //This call suppresses a warning.
        Reset();
    }

    /**
     * 
     */
    void Reset(void) {
        started = false;
    }

    /**
     * 
     */
    void Start(void) {
        startCNT = CNT;
        started = true;
    }

    /**
    @returns the number of elapsed milliseconds since start.
     */
    int GetElapsed(void) {
        if (started == true) {
            return (CNT - startCNT) / (CLKFREQ / 1000);
        } else {
            return 0;
        }
    }

    /**
     * 
     * @return 
     */
    bool GetStarted(void) {
        return started;
    }

private:

    unsigned int startCNT;
    bool started;

};




#endif // SRLM_PROPGCC_STOPWATCH_H_
