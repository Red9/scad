#include "scheduler.h"

Scheduler::Scheduler(int hz)
{
    /*
	readPeriod = (CLKFREQ*10)/hz;
	nextReadTime = CNT + readPeriod;
     */
    periodTicks = GetPeriodTicks(hz);
    startCNT = CNT;
}

bool Scheduler::Run()
{
	unsigned int currentCNT = CNT;
        /*
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
        */
        
        
        if((currentCNT - startCNT) >= periodTicks){
            startCNT += periodTicks;
            return true;
        }else{
            return false;
        }
            
}

unsigned int Scheduler::GetPeriodTicks(int hz){
	return (CLKFREQ*10)/hz;
}

//void Scheduler::SetnextReadTime(unsigned int time)
//{
//	//nextReadTime = time;
//	startCNT = time - periodTicks;
//}
