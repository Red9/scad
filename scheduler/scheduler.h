#ifndef SRLM_PROPGCC_SCHEDULER_H_
#define SRLM_PROPGCC_SCHEDULER_H_
/**
 *
  warning: This class does not guarentee a minimum seperation between true Run()
  calls. Instead, it guarentees that, on average, a true Run() will happen with
  the specified frequency. Depending on how often Run() is called, there may be
  some jitter (if it is not called frequently enough). TODO(SRLM): Add more here
 
 
 
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
  * 
  * The frequency is specified in units of 0.1Hz. So, to create a scheduler with
  * a frequency of 150Hz you pass in 1500. To make a scheduler with a frequency
  * of 0.5Hz (once every two seconds), you pass in 5 (0.1Hz * 5 = 0.5Hz). A
  * scheduler with a frequency of 1Hz takes a parameter of 10.
  *
  * @warning @a hz must be 1 or more! Setting it less will result in a runtime
  *          error.
  * @param hz The frequency (in 10x true return values per second).
  */
Scheduler(int hz);

/** Check to see if the time period has passed yet.
  *
  * This function is not blocking. This class keeps an internal state that
  * watches the system counter, and if the next "period" has begun returns true
  * the next time that this function is called. It then returns false for the
  * rest of the current period.
  *   
  * This function is useful for scheduling reseting watchdog timers,
  * determining when to poll external sensors, and so on.
  *
  * The function returns true for each time period, so if it is not called for
  * multiple time periods it returns true for each (up to hz times).
  *
  * @returns true if the time period has passed, false otherwise.
  */
bool Run();


static unsigned int GetPeriodTicks(int hz);

private:
unsigned int readPeriod;
unsigned int nextReadTime;


unsigned int startCNT;
unsigned int periodTicks;

public:
/** Sets the next read time to a CNT value.
  * @param time the CNT to set to.
  * @warning For testing only! Do not use this function.
  */
//void SetnextReadTime(unsigned int time);

};

#endif // SRLM_PROPGCC_SCHEDULER_H_
