#include "unity.h"
#include "stopwatch.h"

unsigned int CNT;
unsigned int CLKFREQ = 80000000;

void setUp(void)
{

}

void tearDown(void)
{

}

//TODO(SRLM): Add test for GetStarted();

// -----------------------------------------------------------------------------


void test_short_basic_case(void){
	const int milliseconds = 10000;
	CNT = 0;
	Stopwatch sw;
	sw.Start();
	CNT = CLKFREQ/1000 * milliseconds;
	TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
}


void test_long_basic_case(void){
	const int milliseconds = 50000;
	CNT = 0;
	Stopwatch sw;
	sw.Start();
	CNT = CLKFREQ/1000 * milliseconds;
	TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
}

void test_short_rollover_case(void){
	const int milliseconds = 5000;
	CNT = 0xFfffFfff;
	Stopwatch sw;
	sw.Start();
	CNT = CNT + CLKFREQ/1000 * milliseconds;
	TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
}

void test_long_rollover_case(void){
	const int milliseconds = 50000;
	CNT = 0xFfffFfff;
	Stopwatch sw;
	sw.Start();
	CNT = CNT + CLKFREQ/1000 * milliseconds;
	TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
}

