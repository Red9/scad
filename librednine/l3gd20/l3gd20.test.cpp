// Copyright 2013 SRLM and Red9
#include <propeller.h>
#include "unity.h"
#include "l3gd20.h"
#include "c++-alloc.h"

const int kSDAPin = 1;
const int kSCLPin = 2;


i2c * bus;
L3GD20 gyro;


void setUp(void)
{
	bus = new i2c();
	bus->Initialize(kSCLPin, kSDAPin);
	gyro.Init(bus);
}

void tearDown(void)
{
	delete bus;
}

// -----------------------------------------------------------------------------

void test_Init(void)
{
	TEST_ASSERT_EQUAL_INT(0b00110000, bus->GetPutStack());
	TEST_ASSERT_EQUAL_INT(0b11111111, bus->GetPutStack());
	TEST_ASSERT_EQUAL_INT(-1, bus->GetPutStack());
}
void test_ReadGyroPositiveNumbers(void)
{
	char indata[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	bus->SetXYZ(indata, 6);
	int x, y, z;
	TEST_ASSERT_TRUE(gyro.ReadGyro(x, y, z));
	
	TEST_ASSERT_EQUAL_HEX32(0x0201, x);
	TEST_ASSERT_EQUAL_HEX32(0x0403, y);
	TEST_ASSERT_EQUAL_HEX32(0x0605, z);
}

void test_ReadGyroNegativeNumbers(void)
{
	char indata[] = {0x01, 0xF2, 0x03, 0xF4, 0x05, 0xF6};
	bus->SetXYZ(indata, 6);
	int x, y, z;
	TEST_ASSERT_TRUE(gyro.ReadGyro(x, y, z));
	
	TEST_ASSERT_EQUAL_HEX32(0xFFFFF201, x);
	TEST_ASSERT_EQUAL_HEX32(0xFFFFF403, y);
	TEST_ASSERT_EQUAL_HEX32(0xFFFFF605, z);
}
