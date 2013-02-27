#include "scheduler.h"

Scheduler::Scheduler(int hz)
{
	readPeriod = CLKFREQ/hz;
	nextReadTime = CNT + readPeriod;
}

bool Scheduler::Run()
{
	unsigned int currentCNT = CNT;

		//Case CNT = High and nextReadTime = Low
	if( currentCNT>>31==1 && nextReadTime<CLKFREQ )
		return false;
	
		//Case CNT = Low  and nextReadTime = Low
		//Case CNT = High and nextReadTime = High	
	if( currentCNT > nextReadTime 
		//Case CNT = Low  and nextReadTime = High
	   || ( currentCNT < CLKFREQ && nextReadTime>>31==1 ) )
	{
		//Do something
		nextReadTime += readPeriod;
		return true;
	}
	
	return false;

}

void Scheduler::SetnextReadTime(unsigned int time)
{
	nextReadTime = time;
}
