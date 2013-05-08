// Copyright 2013 SRLM and Red9
#include <propeller.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



#include "unity.h"

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

// @WARNING for all locks: Must lockclr the lock after setting it!



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
	
	lockclr(lock);
	lockret(lock);
}

void test_ReturnSetLock(void){
	int lock = locknew();
	TEST_ASSERT_TRUE(lock != -1);
	
	lockset(lock);
	lockclr(lock);
	lockret(lock);
	int lock2 = locknew();
	TEST_ASSERT_EQUAL_INT(lock, lock2);
	
	lockret(lock2);

}

void test_locksetIsEqualToCTrueFalseConstant(void){
	int lock = locknew();
	TEST_ASSERT_TRUE(lock != -1);

	//lockclr(lock);
	
	TEST_ASSERT_TRUE(lockset(lock) == false);
	TEST_ASSERT_TRUE(lockset(lock) != false);
	
	lockclr(lock);
	lockret(lock);

}

void test_dummy(void){
// To test the lock "true" value
//	int lock = locknew();
//	
//	if(lockset(lock) == false){
//		printf("Lock was false!\n");
//	}
//	if(lockset(lock) == true){
//		printf("Lock was true!\n");
//	}
//	if(lockset(lock) != false){
//		printf("Lock was not false!\n");
//	}
//	printf("lockset 'true' value == 0x%X\n", lockset(lock));
//	
//	lockclr(lock);
//	lockret(lock);
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

void test_BooleanAndIsSameAsBitwiseAnd(void){
	TEST_ASSERT_TRUE((true  && false) == (true  & false));
	TEST_ASSERT_TRUE((false && true ) == (false & true ));
	TEST_ASSERT_TRUE((false && false) == (false & false));
	TEST_ASSERT_TRUE((true  && true ) == (true  & true ));
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

// -----------------------------------------------------------------------------

void test_64bitIntegerBasic(void){
	volatile int64_t a = 0x1;
	
	a = a << 32;
	a = a >> 32;
	TEST_ASSERT_EQUAL_INT(0x1, a);
}

void test_64bitIntegerAdd(void){
	volatile int64_t a = 0x100000000;
	a = a + a; //== 0x200000000;

#ifndef UNITY_SUPPORT_64	
	a = a >> 32; //== 0x2;
	TEST_ASSERT_EQUAL_INT(0x2, a);
#else
	TEST_ASSERT_EQUAL_HEX64(0x200000000, a);
#endif
}

void test_64bitIntegerSubtract(void){
	volatile int64_t a = 0x500000000;
	a = a - 0x100000000;
	
#ifndef UNITY_SUPPORT_64
	a = a >> 32;
	TEST_ASSERT_EQUAL_INT(0x4, a);
#else
	TEST_ASSERT_EQUAL_HEX64(0x400000000, a);
#endif
}

void test_64bitIntegerMultiply(void){
	volatile int64_t a = 0x3;
	volatile int64_t b = 0x300000000;
	a = a * b;
	
#ifndef UNITY_SUPPORT_64
	a = a >> 32;	
	TEST_ASSERT_EQUAL_INT(0x9, a);
#else
	TEST_ASSERT_EQUAL_HEX64(0x900000000, a);
#endif
}

void test_64bitIntegerDivide(void){
	volatile int64_t a = 0x3;
	volatile int64_t b = 0x900000000;
	a = b / a;

#ifndef UNITY_SUPPORT_64
	a = a >> 32;
	TEST_ASSERT_EQUAL_INT(0x3, a);
#else
	TEST_ASSERT_EQUAL_HEX64(0x300000000, a);
#endif
}

void test_64bitIntegerAddSpeed(void){

	//Nothing
	int startCNT = CNT;
	int endCNT = CNT;
	int nothingDelta = endCNT - startCNT;

	//64 bit
	volatile int64_t a64 = 0x500000000;
	startCNT = CNT;
	a64 = a64 + a64;
	endCNT = CNT;
	UnityPrint("64bit add deltaCNT == ");
	UnityPrintNumber(endCNT - startCNT - nothingDelta);
	UNITY_OUTPUT_CHAR('\n');

	//32 bit
	volatile int32_t a32 = 0x50000;
	startCNT = CNT;
	a32 = a32 + a32;
	endCNT = CNT;
	UnityPrint("32bit add deltaCNT == ");
	UnityPrintNumber(endCNT - startCNT - nothingDelta);
	UNITY_OUTPUT_CHAR('\n');
	
	
	TEST_ASSERT_EQUAL_INT(0xA0000, a32);	
#ifndef UNITY_SUPPORT_64
	a64 = a64 >> 32;
	TEST_ASSERT_EQUAL_INT(0xA, a64);
#else
	TEST_ASSERT_EQUAL_HEX64(0xA00000000, a64);
#endif


}


void test_64bitIntegerDivideSpeed(void){
	//Nothing
	int startCNT = CNT;
	int endCNT = CNT;
	int nothingDelta = endCNT - startCNT;


	//64 bit
	volatile int64_t a64 = 0x600000000;
	startCNT = CNT;
	a64 = a64 / 3;
	endCNT = CNT;	
	UnityPrint("64bit divide deltaCNT == ");
	UnityPrintNumber(endCNT - startCNT - nothingDelta);
	UNITY_OUTPUT_CHAR('\n');
	
	//32 bit
	volatile int32_t a32 = 0x60000;
	startCNT = CNT;
	a32 = a32 / 3;
	endCNT = CNT;
	UnityPrint("32bit divide deltaCNT == ");
	UnityPrintNumber(endCNT - startCNT - nothingDelta);
	UNITY_OUTPUT_CHAR('\n');
	
	TEST_ASSERT_EQUAL_INT(0x20000, a32);
#ifndef UNITY_SUPPORT_64
	a64 = a64 >> 32;
	TEST_ASSERT_EQUAL_INT(0x2, a64);
#else
	TEST_ASSERT_EQUAL_HEX64(0x200000000, a64);
#endif


}


void test_64bitIntegerMultiplySpeed(void){
	
	//Nothing
	int startCNT = CNT;
	int endCNT = CNT;
	int nothingDelta = endCNT - startCNT;
	
	//64 bit
	volatile int64_t a64 = 0x600000000;
	startCNT = CNT;
	a64 = a64 * 0x30;
	endCNT = CNT;
	UnityPrint("64bit multiply deltaCNT == ");
	UnityPrintNumber(endCNT - startCNT - nothingDelta);
	UNITY_OUTPUT_CHAR('\n');
	
	
	//32 bit
	volatile int32_t a32 = 0x60000;
	startCNT = CNT;
	a32 = a32 * 0x30;
	endCNT = CNT;
	UnityPrint("32bit multiply deltaCNT == ");
	UnityPrintNumber(endCNT - startCNT - nothingDelta);
	UNITY_OUTPUT_CHAR('\n');
		
	TEST_ASSERT_EQUAL_HEX32(0x1200000, a32);
#ifndef UNITY_SUPPORT_64
	a64 = a64 >> 32;
	TEST_ASSERT_EQUAL_HEX32(0x120, a64);
#else
	TEST_ASSERT_EQUAL_HEX64(0x12000000000, a64);
#endif


}

// -----------------------------------------------------------------------------

void test_FloatVariableToInt(void){
	// Get the bits of a float number into an int
	const float floatNum = 0.01f;
	int number = *(int *)&floatNum;
	TEST_ASSERT_EQUAL_HEX32(0x3C23D70A, number);
}

// -----------------------------------------------------------------------------

void test_SignedVsUnsignedComparison(void){
	int sA = 0xFfffFfff;
	int sB = 0x0fffFfff;
	
	TEST_ASSERT_TRUE(sB > sA);

	unsigned int uA = 0xFfffFfff;
	unsigned int uB = 0x0fffFfff;
	
	TEST_ASSERT_TRUE(uA > uB);
}

void test_UnsignedReverseRolloverSubtraction(void){
	unsigned int A = 0xF;
	unsigned int B = 0x10;
	unsigned int result = A - B;
	TEST_ASSERT_EQUAL_HEX32(0xFfffFfff, result);
	

}

// -----------------------------------------------------------------------------





















