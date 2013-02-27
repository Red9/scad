#include "unity.h"
#include "c++-alloc.h"
#include "max17048.h"

MAX17048 * sut;
i2c * bus;

const int kPIN_I2C_SCL = 0;
const int kPIN_I2C_SDA = 1;

void setUp(void){
	bus = new i2c;
	bus->Initialize(kPIN_I2C_SCL, kPIN_I2C_SDA);
	sut = new MAX17048(bus);
}

void tearDown(void){
	delete bus;
	delete sut;
}

void test_GetStatus(void){
	TEST_ASSERT_TRUE(sut->GetStatus());
}

void test_GetVersion(void){
	TEST_ASSERT_EQUAL_HEX32(0x0011, sut->GetVersion());
}

void test_GetVoltage(void){
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, sut->GetVoltage(),
		"Note: Must be manually checked. Fully charged should be about 4200.");
}

void test_GetStateOfCharge(void){
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, sut->GetStateOfCharge(),
		"Note: Must be manually checked. Fully charged should be about 100.");
}

void test_GetChargeRate(void){
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, sut->GetChargeRate(),
		"Note: Must be manually checked. Fully charged should be about 0.");
}





