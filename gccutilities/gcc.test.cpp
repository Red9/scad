// Copyright 2013 SRLM and Red9
#include <propeller.h>
#include "unity.h"

#include <stdio.h>

#include <stdlib.h>


//TODO: Add test for shadow registers being used (is that possible to test?)
//TODO: Add test for bool sized 4 bytes...


void setUp(void){

}

void tearDown(void){
	for(int i = 1; i < 8; ++i){
		cogstop(i);
	}
}

// -----------------------------------------------------------------------------
// Test Shift operations. Right shift is compiler dependent: it can be either
// arithemetic right shift or logical right shift. It usually (and the tests 
// confirm this) depends on the variable:
// unsigned -> use logical right shift
// signed   -> use arithmetic right shift
// -----------------------------------------------------------------------------

void test_ShiftLeft(void){
	volatile int x = 1;
	x = x << 2;
	TEST_ASSERT_EQUAL_INT(4, x);
}

void test_ShiftRightNegative(void){
	volatile int x = -8;
	x = x >> 1;
	TEST_ASSERT_EQUAL_INT(-4, x);
}

void test_ShiftRightUnsignedNumber(void){
	volatile unsigned int x = 0xFFFFFFFF;
	x = x >> 4;
	TEST_ASSERT_EQUAL_HEX32(0x0FFFFFFF, x);
}

//TODO(SRLM): Add tests:
// -Right shift positive signed
// -Left shift unsigned


// -----------------------------------------------------------------------------
// Propeller.h lock tests
// Suggested rules for locks: Propeller Manual v1.2 pg 123
// -----------------------------------------------------------------------------

void test_lockretReturnsLock(void){
	int num_first = locknew();
	lockret(num_first);
	int num_second = locknew();
	TEST_ASSERT_EQUAL_INT(num_first, num_second);
	lockret(num_second);
}

void test_locknewFirstLockIs1(void){
	int num = locknew();
	
	TEST_ASSERT_EQUAL_INT(1, num);
	
	lockret(num);
}

void test_locknewSevenAvailableLocks(void){
	for(int i = 1; i < 8; i++){
		TEST_ASSERT_EQUAL_INT(i, locknew());
	}
	
	TEST_ASSERT_EQUAL_INT(-1, locknew());
	
	for(int i = 1; i < 8; i++){
		lockret(i);
	}
}
	
void test_locksetAndlockclr(void){
	int lock = locknew();
	TEST_ASSERT_TRUE(lock != -1);
		
	TEST_ASSERT_FALSE(lockset(lock));
	TEST_ASSERT_TRUE(lockset(lock));

	lockret(lock);
}

void test_ReturnSetLock(void){
	int lock = locknew();
	TEST_ASSERT_TRUE(lock != -1);
	
	lockset(lock);
	lockret(lock);
	int lock2 = locknew();
	TEST_ASSERT_EQUAL_INT(lock, lock2);
	
	lockret(lock2);

}

// -----------------------------------------------------------------------------
// Type sizes
// -----------------------------------------------------------------------------
void test_SizeofInt(void){
	TEST_ASSERT_EQUAL_INT(4, sizeof(int));
}

void test_SizeofUnsignedInt(void){
	TEST_ASSERT_EQUAL_INT(4, sizeof(unsigned int));
}

void test_SizeofChar(void){
	TEST_ASSERT_EQUAL_INT(1, sizeof(char));
}

void test_SizeofUnsignedChar(void){
	TEST_ASSERT_EQUAL_INT(1, sizeof(unsigned char));
}

void test_SizeofBool(void){
	TEST_ASSERT_EQUAL_INT(1, sizeof(bool));
}

void test_SizeofShort(void){
	TEST_ASSERT_EQUAL_INT(2, sizeof(short));
}

void test_SizeofUnsignedShort(void){
	TEST_ASSERT_EQUAL_INT(2, sizeof(unsigned short));
}

void test_SizeofIntPointer(void){
	TEST_ASSERT_EQUAL_INT(4, sizeof(int *));
}

void test_SizeofShortPointer(void){
	TEST_ASSERT_EQUAL_INT(4, sizeof(short *));
}

void test_SizeofCharPointer(void){
	TEST_ASSERT_EQUAL_INT(4, sizeof(char *));
}

// -----------------------------------------------------------------------------

void test_WhatIsTrue(void){
	TEST_ASSERT_EQUAL_INT(1, true);
}

void test_WhatIsFalse(void){
	TEST_ASSERT_EQUAL_INT(0, false);
}

void test_OnlyTrueEqualsTrue(void){
	TEST_ASSERT_TRUE(2 != true);
}

void test_AnyNonZeroNumberIsTrue(void){
	TEST_ASSERT_TRUE(1);
	TEST_ASSERT_TRUE(2);
	TEST_ASSERT_TRUE(200);
	TEST_ASSERT_TRUE(-1);
	TEST_ASSERT_TRUE(-200);
}

void test_ZeroIsFalse(void){
	TEST_ASSERT_FALSE(0);
}

// -----------------------------------------------------------------------------

void test_WritingAnIntToACharWillTruncate(void){
	char data []= {0,0,0,0,0,0,0,0};
	data[4] = 0xFFFFFFFF;
	TEST_ASSERT_EQUAL_HEX8(0, data[0]);
	TEST_ASSERT_EQUAL_HEX8(0, data[1]);
	TEST_ASSERT_EQUAL_HEX8(0, data[2]);
	TEST_ASSERT_EQUAL_HEX8(0, data[3]);
	TEST_ASSERT_EQUAL_HEX8(0xFF, data[4]);
	TEST_ASSERT_EQUAL_HEX8(0, data[5]);
	TEST_ASSERT_EQUAL_HEX8(0, data[6]);
	TEST_ASSERT_EQUAL_HEX8(0, data[7]);
}

void test_InitializingACharWith16BitsWillTruncate(void){
	char data = 0xABCD;
	TEST_ASSERT_EQUAL_INT(0xCD, data);
}
	
// -----------------------------------------------------------------------------


void FunctionThatEnds(void){
	waitcnt(CLKFREQ/10 + CNT);
}

void FunctionThatEndsWithCogstop(void){
	waitcnt(CLKFREQ/10 + CNT);
	cogstop(cogid());
}

void test_WhatHappensWhenACogReachesTheEnd(void){
	int stacksize = sizeof(_thread_state_t)+sizeof(int)*3 + sizeof(int)*100;
	
	int * stackA = (int*) malloc(stacksize);		
	int cogA = cogstart(FunctionThatEnds, NULL, stackA, stacksize);
	
	int * stackB = (int*) malloc(stacksize);		
	int cogB = cogstart(FunctionThatEnds, NULL, stackB, stacksize);
	
	waitcnt(CLKFREQ/10 + CNT);
	
	int * stackC = (int*) malloc(stacksize);		
	int cogC = cogstart(FunctionThatEnds, NULL, stackC, stacksize);

	TEST_ASSERT_EQUAL_INT(1, cogA);
	TEST_ASSERT_EQUAL_INT(2, cogB);
	TEST_ASSERT_EQUAL_INT(3, cogC);
	
	//cleanUp
	for(int i = 1; i < 8; ++i){
		cogstop(i);
	}	
	
	free(stackA);
	free(stackB);
	free(stackC);
}
		
void test_WhatHappensWhenACogReachesTheEndWithCogstop(void){
	int stacksize = sizeof(_thread_state_t)+sizeof(int)*3 + sizeof(int)*100;
	
	int * stackA = (int*) malloc(stacksize);		
	int cogA = cogstart(FunctionThatEndsWithCogstop, NULL, stackA, stacksize);
	
	int * stackB = (int*) malloc(stacksize);		
	int cogB = cogstart(FunctionThatEnds, NULL, stackB, stacksize);
	
	waitcnt(CLKFREQ/5 + CNT);
	
	int * stackC = (int*) malloc(stacksize);		
	int cogC = cogstart(FunctionThatEndsWithCogstop, NULL, stackC, stacksize);

	TEST_ASSERT_EQUAL_INT(1, cogA);
	TEST_ASSERT_EQUAL_INT(2, cogB);
	TEST_ASSERT_EQUAL_INT(1, cogC);
	
	//cleanUp
	for(int i = 1; i < 8; ++i){
		cogstop(i);
	}	
	free(stackA);
	free(stackB);
	free(stackC);
}




































