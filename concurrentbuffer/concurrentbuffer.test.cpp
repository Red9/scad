#include <stdio.h>
#include <propeller.h>

#include "unity.h"
#include "concurrentbuffer.h"

void setUp(void){
	ConcurrentBuffer::ResetHead();
}

void tearDown(void){
	
}

// -----------------------------------------------------------------------------

//TODO(SRLM): Add tests to make sure that it only takes out one lock
//TODO(SRLM): When I used this class, it was locking up when used in multiple
// cogs. To solve this, I made head volatile, and that seemed to fix it. I also
// made buffer volatile just for safety. To Do: add a test that will fail if
// head or buffer is not volatile.


void test_CBResetHead(void){
	ConcurrentBuffer::ResetHead();
	ConcurrentBuffer b;
	b.Put('0');
	ConcurrentBuffer::ResetHead();
	b.Put('1');
	TEST_ASSERT_EQUAL_HEX8('1', b.Get());
}

void test_CBBasicSingleCharacterSingleThread(void){
	ConcurrentBuffer b;
	b.Put('a');
	
	TEST_ASSERT_EQUAL_HEX8('a', b.Get());	
}

void test_CBBasicMultipleCharactersSingleThread(void){

	char data[] = "abcdef";
	int size = 6;
	ConcurrentBuffer b;
	b.Put(data, size);
	
	char result[size];
	result[size] = 0; //Null terminate
	
	for(int i = 0; i < size; i++){
		result[i] = b.Get();
	}
	
	TEST_ASSERT_EQUAL_STRING(data, result);
}

void test_CBBasicPutTimeout(void){
	int timeout = 1000; //1000us = 1 ms
	ConcurrentBuffer b(timeout); 
	
	
	TEST_ASSERT_TRUE(b.Put('a'));
	TEST_ASSERT_FALSE(lockset(ConcurrentBuffer::lock));	//Make sure nothing had
	                                                    //it before the test
	unsigned int startCNT = CNT;
	TEST_ASSERT_FALSE(b.Put('b'));
	unsigned int endCNT = CNT;
	
	//Check to make sure it took less than 2 timeout periods
	TEST_ASSERT_TRUE((endCNT-startCNT) < 2*(CLKFREQ/1000000)*timeout);
	
	lockclr(ConcurrentBuffer::lock);
}

void test_CBAddLargeAmountsOfData(void){
	char buffer[] = "abcdef";
	int size = 6;
	ConcurrentBuffer b;
	
	for(int i = 0; i < 100; ++i){
		b.Put(buffer, size);
		for(int j = 0; j < size; ++j){
			TEST_ASSERT_EQUAL_HEX8(buffer[j], b.Get());
		}
	}
}

void test_CBGetFreeBasic(void){
	char buffer[] = "abcdef";
	int size = 6;
	
	ConcurrentBuffer b;
	
	TEST_ASSERT_EQUAL_INT(b.GetkSize()-1, b.GetFree());
	
	b.Put(buffer, size);
	
	TEST_ASSERT_EQUAL_INT(b.GetkSize()-size-1, b.GetFree());

}

void test_CBGetFreeWrapAround(void){

	ConcurrentBuffer b;
	
	
	//Go to halfway through the buffer
	for(int i = 0; i < (b.GetkSize()/2); ++i){
		b.Put('c');
		b.Get();
	}
	
	//No wrap around
	TEST_ASSERT_EQUAL_INT(b.GetkSize()-1, b.GetFree());
	
	
	//Go for 3/4 the buffer
	for(int i = 0; i < (3*b.GetkSize()/4); ++i){
		b.Put('c');
	}
	
	TEST_ASSERT_EQUAL_INT(b.GetkSize() - (3*b.GetkSize()/4)-1, b.GetFree());
	
	
}











































