#include <stdio.h>
#include "unity.h"
#include "eeprom.h"


//TODO: Add test to see about the -1 return value for Get

//offset is a randomization feature to make sure that we're not reading from a
//previous successful test (and pass, even if the Put failed).
//Offset is based on the current seconds at compilation.
int addressOffset = ((__TIME__)[6] - '0')*10 + (__TIME__)[7] - '0';
char dataOffset = ((__TIME__)[3] - '0')*5 + (__TIME__)[7] - '0';

void setUp(void){
	
}

void tearDown(void){

}


void test_Warning(void){
	printf("---------------------------------------------\r\n");
	printf("Warning: This test suite mangles your EEPROM!\r\n");
	printf("Current addressOffset: %i\r\n", addressOffset);
	printf("Current dataOffset:    %i\r\n", dataOffset);
	printf("---------------------------------------------\r\n");
}
	

void test_SingleByteReadWrite(void){
	Eeprom mem;
	unsigned short address = 0x8121 + addressOffset;
	char data = 0xA9 + dataOffset;
	TEST_ASSERT_TRUE(mem.Put(address, data));
	TEST_ASSERT_EQUAL_HEX8(data, mem.Get(address));
}


void test_PageWriteSingleByteRead(void){
	Eeprom mem;
	unsigned short address = 0x80F5 + addressOffset;
	int size = 500;
	char data [size];
	char result = dataOffset;
	data[0] = result;
	data[size-1] = result;
	TEST_ASSERT_TRUE(mem.Put(address, data, size));
	TEST_ASSERT_EQUAL_INT(result, mem.Get(address));
	TEST_ASSERT_EQUAL_INT(result, mem.Get(address+size-1));
}

void test_PageReadWrite(void){
	Eeprom mem;
	unsigned short address = 0x80F7 + addressOffset;
	int size = 500;
	char output [size];
	char input  [size];

	output[0] = 0x25+dataOffset;
	output[1] = 0xE9+dataOffset;
	output[size/2] = 0x42+dataOffset;
	output[size-1] = 0xF0+dataOffset;
	
	
	TEST_ASSERT_TRUE(mem.Put(address, output, size));
	mem.Get(address, input, size);
	TEST_ASSERT_EQUAL_MEMORY(output, input, size);
	
}

void test_PageReadDoesntBufferOverflow(void){
	Eeprom mem;
	unsigned short address = 0x8223 + addressOffset;
	int size = 200;
	char output [size];
	char input  [size*2];
	char overflowPattern [size];
	for(int i = 0; i < size; ++i){
		input[i + size] = overflowPattern[i] = 0x83 + dataOffset;
	}
	
	mem.Put(address, output, size);
	mem.Get(address, input, size);
	TEST_ASSERT_EQUAL_MEMORY(overflowPattern, input+size, size);
}

void test_PutGetInt(void){
	Eeprom mem;
	unsigned short address = 0x9B21 + addressOffset;
	int size = 4;
	int bytes = 5367+dataOffset;
	
	mem.Put(address, bytes, size);
	TEST_ASSERT_EQUAL_INT(bytes, mem.Get(address, size));	

}

void test_PutGetShort(void){
	Eeprom mem;
	unsigned short address = 0x965D + addressOffset;
	int size = 2;
	int bytes = 525+dataOffset;
	
	mem.Put(address, bytes, size);
	TEST_ASSERT_EQUAL_INT(bytes, mem.Get(address, size));	
}




















