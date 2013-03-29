#include "unity.h"

#include "c++-alloc.h"

#include "pcf8523.h"


const int kPIN_I2C_SCL = 0;
const int kPIN_I2C_SDA = 1;
const int kPIN_PCF8523_SQW = 27;

PCF8523 * sut;
i2c * bus;


int startYear, startMonth, startDay, startHour, startMinute, startSecond;


void setUp(void){
	bus = new i2c;
	bus->Initialize(kPIN_I2C_SCL, kPIN_I2C_SDA);
	sut = new PCF8523(bus, kPIN_PCF8523_SQW);
}

void tearDown(void){
	delete bus;
	delete sut;
}

//TODO(SRLM): What's wrong with this test?
//void test_PassNullBuss(void){
//	PCF8523 rtc(NULL);
//	TEST_ASSERT_FALSE(rtc.GetStatus());
//}

void test_SaveCurrentClock(void){
	//Saves the current clock so that we don't have to keep putting the time back in.
	//It's ok if it get's off by a second or two.
	sut->GetClock(startYear, startMonth, startDay, startHour, startMinute, startSecond);
	TEST_IGNORE();
}


void test_GetStatusPass(void){
	TEST_ASSERT_TRUE(sut->GetStatus());
}
void test_ConvertToBCDUnitsOnly(void){
	TEST_ASSERT_EQUAL_HEX8(0b00000010, sut->ConvertToBCD(2));
}
void test_ConvertToBCDTensOnly(void){
	TEST_ASSERT_EQUAL_HEX8(0b01010000, sut->ConvertToBCD(50));
}
void test_ConvertToBCDBothUnitsAndTens(void){
	TEST_ASSERT_EQUAL_HEX8(0b01001001, sut->ConvertToBCD(49));
}
void test_ConvertFromBCDUnitsOnly(void){
	TEST_ASSERT_EQUAL_INT(2, sut->ConvertFromBCD(0b00000010));
}
void test_ConvertFromBCDTensOnly(void){
	TEST_ASSERT_EQUAL_INT(50, sut->ConvertFromBCD(0b01010000));
}
void test_ConvertFromBCDUnitsAndTens(void){
	TEST_ASSERT_EQUAL_INT(49, sut->ConvertFromBCD(0b01001001));
}

void test_SetGetClock(void){
	
	int clock[] = {12, 1, 24, 13, 59, 12, 4};
	int result[7];
	TEST_ASSERT_TRUE(sut->SetClock(clock[0], clock[1], clock[2], clock[3], clock[4], clock[5], clock[6]));
	TEST_ASSERT_TRUE(sut->GetClock(result[0], result[1], result[2], result[3], result[4], result[5], result[6]));
	
	TEST_ASSERT_EQUAL_INT_ARRAY(clock, result, 7);
	

}

void test_SetGetClockNoWeekday(void){
	
	int clock[] = {12, 1, 24, 13, 59, 12};
	int result[7];
	TEST_ASSERT_TRUE(sut->SetClock(clock[0], clock[1], clock[2], clock[3], clock[4], clock[5]));
	TEST_ASSERT_TRUE(sut->GetClock(result[0], result[1], result[2], result[3], result[4], result[5]));
	
	TEST_ASSERT_EQUAL_INT_ARRAY(clock, result, 6);
	

}

void test_RestoreCurrentClock(void){
	sut->SetClock(startYear, startMonth, startDay, startHour, startMinute, startSecond);
	TEST_IGNORE();
}

