// Copyright 2013 SRLM and Red9
#include <propeller.h>
#include "unity.h"
#include "lsm303dlhc.h"
#include "c++-alloc.h"

const int kSDAPin = 1;
const int kSCLPin = 2;


i2c * bus;
LSM303DLHC acclMagn;


void setUp(void)
{
	bus = new i2c();
	bus->Initialize(kSCLPin, kSDAPin);
	acclMagn.Init(bus);
}

void tearDown(void)
{
	delete bus;
}

// -----------------------------------------------------------------------------

void test_Init(void)
{
	TEST_ASSERT_EQUAL_INT(0b00111000, bus->GetPutStack());
	TEST_ASSERT_EQUAL_INT(0b10010111, bus->GetPutStack());
	TEST_ASSERT_EQUAL_INT(0b00000000, bus->GetPutStack());
	TEST_ASSERT_EQUAL_INT(0b11100000, bus->GetPutStack());
	TEST_ASSERT_EQUAL_INT(0b10011100, bus->GetPutStack());
	TEST_ASSERT_EQUAL_INT(-1, bus->GetPutStack());
}

void test_ReadAcclPositiveNumbers(void)
{
	//Notice how the lower 4 bits are removed: LSM303DLHC returns 12 bit numbers!
	char indata[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	bus->SetXYZ(indata, 6);
	int x, y, z;
	TEST_ASSERT_TRUE(acclMagn.ReadAccl(x, y, z));
	
	TEST_ASSERT_EQUAL_HEX32(0x020, x);
	TEST_ASSERT_EQUAL_HEX32(0x040, y);
	TEST_ASSERT_EQUAL_HEX32(0x060, z);
}

void test_ReadAcclNegativeNumbers(void)
{
	//Notice how the lower 4 bits are removed: LSM303DLHC returns 12 bit numbers!
	char indata[] = {0x01, 0xF2, 0x03, 0xF4, 0x05, 0xF6};
	bus->SetXYZ(indata, 6);
	int x, y, z;
	TEST_ASSERT_TRUE(acclMagn.ReadAccl(x, y, z));
	
	TEST_ASSERT_EQUAL_HEX32(0xFFFFFF20, x);
	TEST_ASSERT_EQUAL_HEX32(0xFFFFFF40, y);
	TEST_ASSERT_EQUAL_HEX32(0xFFFFFF60, z);
}

void test_ReadMagnPositiveNumbers(void)
{
	//Notice how the lower 4 bits are removed: LSM303DLHC returns 12 bit numbers!
	char indata[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	bus->SetXYZ(indata, 6);
	int x, y, z;
	TEST_ASSERT_TRUE(acclMagn.ReadMagn(x, y, z));
	
	TEST_ASSERT_EQUAL_HEX32(0x010, x);
	TEST_ASSERT_EQUAL_HEX32(0x030, y);
	TEST_ASSERT_EQUAL_HEX32(0x050, z);
}

void test_ReadMagnNegativeNumbers(void)
{
	//Notice how the lower 4 bits are removed: LSM303DLHC returns 12 bit numbers!
	char indata[] = {0xF1, 0xA2, 0xF3, 0xA4, 0xF5, 0xA6};
	bus->SetXYZ(indata, 6);
	int x, y, z;
	TEST_ASSERT_TRUE(acclMagn.ReadMagn(x, y, z));
	
	TEST_ASSERT_EQUAL_HEX32(0xFFFFFF1A, x);
	TEST_ASSERT_EQUAL_HEX32(0xFFFFFF3A, y);
	TEST_ASSERT_EQUAL_HEX32(0xFFFFFF5A, z);
}

//TODO(SRLM): Add tests for the ReadAccl/ReadMagn bus->Get == bus->kNak case
//Add tests for status being bus->kNak
