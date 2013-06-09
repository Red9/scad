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

class Stopwatch {
public:
    /**
     * 
     */
    Stopwatch();

    /**
     * 
     */
    void Reset(void);
    
    /**
     * 
     */
    void Start(void);
    
    /**
    @returns the number of elapsed milliseconds since start.
     */
    int GetElapsed(void);
    
    /**
     * 
     * @return 
     */
    bool GetStarted(void);
    
private:
    
    unsigned int startCNT;
    bool started;

};




#endif // SRLM_PROPGCC_STOPWATCH_H_
