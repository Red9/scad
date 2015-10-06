#include "eventlog.h"
//#include <stdio.h>
#include <propeller.h>
#include "c++-alloc.h"
#include "unity.h"



EventLog * sut;

const int kPIN_USER_1 = 18;
const int kPIN_USER_2 = 19;

void setUp(void){
	sut = new EventLog(kPIN_USER_1, kPIN_USER_2, 460800);
}

void tearDown(void){
	delete sut;
}

void test_Simple(void){
		sut->LogInfo(EventLog::kError, "Hello.")
}










