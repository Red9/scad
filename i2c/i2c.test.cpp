// Copyright 2013 SRLM and Red9

#include <propeller.h>
#include "unity.h"
#include "i2c.h"

#include "Pin.h"

//TODO(SRLM): Add test to make sure there are pullups.
//TODO(SRLM): Add test for ping non-existent device.

const int kSDAPin = 1;
const int kSCLPin = 2;


static const unsigned char EEPROM = 0b10100000;
static const unsigned char Accl   = 0b00110010;
static const unsigned char GYRO   = 0b11010110;
static const unsigned char Magn   = 0b00111100;
static const unsigned char BARO   = 0b11101110;
static const unsigned char Fuel   = 0b01101100;

i2c sut;

Pin pin(3);

int registerAddress = 4;
int randomAddress = 0;

int RandomAddress()
{
	registerAddress = registerAddress * 1103515245 + 12345;
	registerAddress = (registerAddress & 0xFFFF0000) & 0x8FFF; //Note: keep this & 0x8FFF so that it's within 32kB address range
	return registerAddress;
}

void setUp(void)
{
	randomAddress = RandomAddress();
	sut.Initialize(kSCLPin, kSDAPin);
}

void tearDown(void)
{

}

// -----------------------------------------------------------------------------


void test_ToggleStartPin()
{
	//Toggles a start pin to indicate the begining of testing to an external logic analyzer
	pin.low();
	waitcnt(CLKFREQ/1000 + CNT);
	pin.high();
	waitcnt(CLKFREQ/1000 + CNT);
	pin.low();
	TEST_ASSERT(true)
}


// -----------------------------------------------------------------------------
// EEPROM
// -----------------------------------------------------------------------------

void test_EEPROMPing(void)
{
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Ping(EEPROM));
}

void test_EEPROMSendByteWrite(void)
{
	int data = 0xAE;
	
	randomAddress = 0b01011010; //&= 0x7FFF; //Set to EEPROM address range
	
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.PutEEPROM(EEPROM, (short)randomAddress, data));
	while(sut.Ping(EEPROM) == sut.kNak);
	TEST_ASSERT_EQUAL_HEX8(data, sut.GetEEPROM(EEPROM, (short)randomAddress));

}


// -----------------------------------------------------------------------------
// "Normal I2C"
// -----------------------------------------------------------------------------

void test_L3GD20Ping(void)
{
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Ping(GYRO));
}


void test_L3GD20ReadWhoAmIRegister(void)
{
	TEST_ASSERT_EQUAL_INT(0b11010100, sut.Get(GYRO, 0b00001111));	
}

void test_L3GD20WriteCtrlReg1(void)
{
	//Test twice so that we *know* that it actually wrote (instead of from a previous test)
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Put(GYRO, 0x20, 0b01111111));
	TEST_ASSERT_EQUAL_HEX8(0b01111111, sut.Get(GYRO, 0x20));
	
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Put(GYRO, 0x20, 0xFF));
	TEST_ASSERT_EQUAL_HEX8(0xFF, sut.Get(GYRO, 0x20));
}

void test_L3GD20WriteMultipleBytes(void)
{
//Note that the address (SUB) is bitwise OR'd with 0x80 (set the MSb high) in
//  order to indicate to the slave device to auto increment the address.
	unsigned char data_address = 0x32 | 0x80; //Interupt threshold registers
	char indata1[] = {0x0F, 0xFA, 0x0E, 0x80, 0x01, 0x22};
	char indata2[] = {0x0E, 0xF9, 0x0D, 0x7F, 0x00, 0x21};
	int data_size = 6;
	char outdata[data_size];
		
	//First Write
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Put(GYRO, data_address, indata1, data_size));
	sut.Get(GYRO, data_address, outdata, data_size);
	TEST_ASSERT_EQUAL_MEMORY(indata1, outdata, data_size);
	
	//Second Write
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Put(GYRO, data_address, indata2, data_size));
	sut.Get(GYRO, data_address, outdata, data_size);
	TEST_ASSERT_EQUAL_MEMORY(indata2, outdata, data_size);

	
	
}

void testL3GD20WriteMultipleBytesButOnlyOne(void)
{
//Note that the address (SUB) is bitwise OR'd with 0x80 (set the MSb high) in
//  order to indicate to the slave device to auto increment the address.
	//Test using the multiple byte reads for just one byte
	unsigned char data_address = 0x32 | 0x80;
	char indata1[] = {0x0F};
	char indata2[] = {0x0E};
	int data_size = 1;
	char outdata[data_size];
	
			
	//First Write
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Put(GYRO, data_address, indata1, data_size));
	sut.Get(GYRO, data_address, outdata, data_size);
	TEST_ASSERT_EQUAL_MEMORY(indata1, outdata, data_size);
	
	//Second Write
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Put(GYRO, data_address, indata2, data_size));
	sut.Get(GYRO, data_address, outdata, data_size);
	TEST_ASSERT_EQUAL_MEMORY(indata2, outdata, data_size);
}

// -----------------------------------------------------------------------------
// MS5611
// -----------------------------------------------------------------------------

void testMS5611PutSingleByteGetMultipleBytes(void)
{
	TEST_IGNORE_MESSAGE("Must confirm function with Logic16");
	//Command read memory address 011
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Put(BARO, 0b10100110));
	
	int data_size = 2;
	char indata1[data_size];
	TEST_ASSERT_EQUAL_INT(sut.kAck, sut.Get(BARO, indata1, data_size));
}




