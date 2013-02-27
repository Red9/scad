#ifndef SRLM_PROPGCC_SCHEDULER_H_
#define SRLM_PROPGCC_SCHEDULER_H_
/**
 *
 * @author SRLM
 * @date   2013-01-21
 */

#ifndef UNIT_TEST
#include <propeller.h>
#else
extern volatile unsigned int CNT;
extern volatile unsigned int CLKFREQ;
#endif

class Scheduler{
public:

/** Create a schedule keeper that returns true with the specified frequency.
  * @todo Make this function work for something like 0.1 Hz (1 cycle every 10 seconds...)
  * @warning @a hz must be 1 or more! Setting it less will result in a runtime
  *          error.
  * @param hz The frequency (in true return values per second).
  */
Scheduler(int hz);

/** Check to see if the time period has passed yet.
  *
  * This function is not blocking. This class keeps an internal state that
  * watches the system counter, and if the next "period" has begun returns true
  * the next time that this function is called. It then returns false for the
  * rest of the current period.
  *   
  * This function is useful for scheduling reseting watchdog timers, polling
  * extrenal sensors, and so on.
  *
  * The function returns true for each time period, so if it is not called for
  * multiple time periods it returns true for each (up to hz times).
  *
  * @returns true if the time period has passed, false otherwise.
  */
bool Run();

private:
unsigned int readPeriod;
unsigned int nextReadTime;

public:
/** Sets the next read time to a CNT value.
  * @param time the CNT to set to.
  * @warning For testing only! Do not use this function.
  */
void SetnextReadTime(unsigned int time);

};

#endif // SRLM_PROPGCC_SCHEDULER_H_
