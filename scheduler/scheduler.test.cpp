#include "unity.h"
#include "scheduler.h"


unsigned volatile int CNT;
unsigned volatile int CLKFREQ;



void setUp(void)
{
	CNT = 0;
	CLKFREQ = 80000000;
}

void tearDown(void)
{

}


// -----------------------------------------------------------------------------

void test_SchedulerReadPeriodIncremented(void)
{
	int hz = 100;
	Scheduler scheduler(hz*10);
	
	CNT += 1;
	
	TEST_ASSERT_FALSE(scheduler.Run());

	//At 6000 cycles and 100hz it should loop CNT around.
	//2^32 / (80000000/100) = 5368
	for(int i = 0; i < 6000; i++)
	{
		CNT += CLKFREQ/hz/2;
		TEST_ASSERT_FALSE(scheduler.Run());
		CNT += CLKFREQ/hz/2;
		TEST_ASSERT_TRUE(scheduler.Run());
	}

}


// -----------------------------------------------------------------------------
// Test the various relative CNT and nextReadTime conditions
// -----------------------------------------------------------------------------
void test_SchedulerLowBoth(void)
{
	CNT = 0x0;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0x10);
	
	TEST_ASSERT_FALSE(scheduler.Run());
}

void test_SchedulerLowBothOpposite(void)
{
	CNT = 0x10;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0x0);
	
	TEST_ASSERT_TRUE(scheduler.Run());
}

void test_SchedulerHighBoth(void)
{
	CNT = 0xFFFFFFF0;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0xFFFFFFFF);
	
	TEST_ASSERT_FALSE(scheduler.Run());
}

void test_SchedulerHighBothOpposite(void)
{
	CNT = 0xFFFFFFFF;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0xFFFFFFF0);
	
	TEST_ASSERT_TRUE(scheduler.Run());
}

void test_SchedulerRollover(void)
{
	CNT = 0xFFFFFFFF;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0x0);
	
	TEST_ASSERT_FALSE(scheduler.Run());
}

void test_SchedulerRolloverOpposite(void)
{
	CNT = 0x0;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0xFFFFFFFF);
	
	TEST_ASSERT_TRUE(scheduler.Run());
}

void test_SchedulerMedian(void)
{
	CNT = 0x0FFFFFFF;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0xF0000000);
	
	TEST_ASSERT_FALSE(scheduler.Run());
}

void test_SchedulerMedianOpposite(void)
{
	CNT = 0xF0000000;
	int hz = 100;
	Scheduler scheduler(hz);
	scheduler.SetnextReadTime(0x0FFFFFFF);
	
	TEST_ASSERT_TRUE(scheduler.Run());
}

// -----------------------------------------------------------------------------
void test_SchedulerNotCalledForMultiplePeriods(void)
{
	CNT = 0x0;
	int hz = 100;
	Scheduler scheduler(hz*10);
	TEST_ASSERT_FALSE(scheduler.Run());
	CNT += (CLKFREQ/100 * 4) + 1;
	TEST_ASSERT_TRUE(scheduler.Run());
	TEST_ASSERT_TRUE(scheduler.Run());
	TEST_ASSERT_TRUE(scheduler.Run());
	TEST_ASSERT_TRUE(scheduler.Run());
	TEST_ASSERT_FALSE(scheduler.Run());
}



















