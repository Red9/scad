// Copyright 2013 SRLM and Red9

#include <propeller.h>

#include <stdio.h>

#include "unity.h"
#include "serial.h"

#include "c++-alloc.h"

//TODO(SRLM): Add test to check to make sure that the pins are actually connected.

//TODO(SRLM): Add test to make sure that stop actualy stops (ie, check that there is a free cog).

//TODO (SRLM): Add test suite to test for general things (like sizeof(int))
//	printf("\n\n------\n");
//	printf("sizeof(int) = %i\n", sizeof(int));
//	printf("sizeof(char) = %i\n", sizeof(char));
//	printf("sizeof(char*) = %i\n", sizeof(char*));
//	printf("sizeof(bool) = %i\n", sizeof(bool));
//	printf("sizeof('a') = %i\n", sizeof('a'));
//	printf("\n\n------\n");

//TODO (SRLM): Add test for %% sequence

//TODO(SRLM): add test to make sure that the driver sets the IO pins correctly (input, output, etc.)



/**
Required Hardware:
 pins @a rxpin and @a txpin connected by a ~10kOhm resistor
 pins @a rtspin and @a cts pin connected by a ~10kOhm resistor
*/




int32_t rxpin = 18;
int32_t txpin = 19;
//int32_t baud = 115200;
int32_t baud = 460800;



int ctspin = 20; //Input to Propeller
int rtspin = 21; //Output from Propeller (currently not used by driver, but driven.


Serial sut;

int MAXTIME = 10; //maximum time (in milliseconds) to wait for GetCCheck for tests. Prevents hangups.

void setUp(void)
{
	sut.Start(rxpin, txpin, baud);
}

void tearDown(void)
{
	sut.Stop();
}

// -----------------------------------------------------------------------------
void test_Start(void)
{
	sut.Stop();
	int32_t result = sut.Start(rxpin, txpin, baud);
	TEST_ASSERT_TRUE(result);
}


// -----------------------------------------------------------------------------
// Single or Few Character Checks
// -----------------------------------------------------------------------------
void test_PutCGetC(void)
{
	sut.Put('a');
	TEST_ASSERT_EQUAL_INT('a', sut.Get(MAXTIME));
}

void test_PutCGetCLowerByteBound(void)
{
	sut.Put('\0');
	TEST_ASSERT_EQUAL_INT(0, sut.Get(MAXTIME));
}

void test_PutCGetCUpperByteBound(void)
{
	sut.Put(255);
	TEST_ASSERT_EQUAL_INT(255, sut.Get(MAXTIME));
}

void test_GetCCheckNoPutC(void)
{
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
}

void test_GetCCheck(void)
{
	sut.Put('G');
	TEST_ASSERT_EQUAL_INT('G', sut.Get(MAXTIME));
}

void test_GetCCheckNoWait(void)
{
	sut.Put('T');
	TEST_ASSERT_EQUAL_INT('T', sut.Get(1));	
}

void test_GetCCheckTimeTimeout(void)
{
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(1));
}

void test_RxTxNoExtraTxChars(void)
{
	sut.Put('z');
	sut.Get(MAXTIME);
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
}


// TODO(SRLM): add GetCCheck(time) tests that accurately tests the timing of the code.


// -----------------------------------------------------------------------------
// String and Long Buffer Tests
// -----------------------------------------------------------------------------

void test_PutBufferFormatted(void)
{
//	sut.GetFlush();
	
	TEST_ASSERT_EQUAL_INT(3, sut.PutFormatted("abc"));
	
	
	
	TEST_ASSERT_EQUAL_INT('a', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT('b', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT('c', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
}

//void test_PutBufferNoSize(void)
//{
//	sut.Put("abc", 0);
//	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
//}

void test_PutBufferNoPointer(void)
{
	TEST_ASSERT_EQUAL_INT(0, sut.PutFormatted((char *)NULL));
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
}

//void test_PutBufferNoWaitActuallyDoesntWait(void)
//{
//	//If this test fails, it's possible because PutBufferNoWait() ends up waiting,
//	//so that the GetFlush empties out all the bytes it will receive. In that
//	//case, there are no bytes left to receive and it times out.
//	sut.PutBufferNoWait("123456789012345678901234567890", 30);
//	sut.GetFlush();
//	int result = sut.Get(MAXTIME);   
//	TEST_ASSERT_TRUE(result != -1);
//}

void test_PutS(void)
{
//	char * volatile temp = new char(6);
//	temp[0] = 'H';
//	temp[1] = 'e';
//	temp[2] = 'l';
//	temp[3] = 'l';
//	temp[4] = 'o';
//	temp[5] = '\0';
//	
//	
//	TEST_ASSERT_EQUAL_INT(5, sut.Put(temp));
//	
//	volatile char message[] = "Hello";
//	sut.PutBuffer(message);	
	
	
	TEST_ASSERT_EQUAL_INT(5, sut.PutFormatted("Hello"));
	TEST_ASSERT_EQUAL_INT('H', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT('e', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT('l', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT('l', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT('o', sut.Get(MAXTIME));
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
	
	
	
}

// -----------------------------------------------------------------------------
// Extra Functions Test
// -----------------------------------------------------------------------------
void test_GetFlush(void)
{
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty before test
	sut.Put('1');
	waitcnt(CLKFREQ/100 + CNT); //In LMM mode, it's too fast to flush...
	sut.GetFlush();
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty after flush
}

void test_GetFlushEmptyBuffer(void)
{
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty before test
	sut.GetFlush();
	TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty after flush
}



// -----------------------------------------------------------------------------
// Baud Checks
// -----------------------------------------------------------------------------
void test_SetBaud(void)
{
	TEST_ASSERT_TRUE(sut.SetBaud(9600));
}

void test_SetBaudTooHigh(void)
{
	TEST_ASSERT_EQUAL_INT(0, sut.SetBaud(1000000));
}

void test_SetBaudToZero(void)
{
	TEST_ASSERT_EQUAL_INT(0, sut.SetBaud(0));
}

void test_SetBaudTransmitAfterBaudChange(void)
{
	sut.SetBaud(9600);
	sut.Put('a');
	TEST_ASSERT_EQUAL_INT('a', sut.Get(MAXTIME));
}

void test_Setbaudclock(void)
{
	TEST_ASSERT_TRUE(sut.SetBaudClock(9600, CLKFREQ));
}
// -----------------------------------------------------------------------------

void test_PutPrintfReturnsWrittenBytes(void){
	int size = 30;
	char inputBuffer[size];

	TEST_ASSERT_EQUAL_INT(17, sut.PutFormatted("My:%i, Your:%i", 123, -531));
	TEST_ASSERT_EQUAL_INT(17, sut.Get(inputBuffer, 17, MAXTIME));
	sut.GetFlush();
}

void test_PutPrintfBasic(void){

	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}

	sut.PutFormatted("My number: %i.", 123);
	sut.Get(inputBuffer, 15, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My number: 123.", inputBuffer);
	

}

void test_PutPrintfMultipleIntegers(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}

	sut.PutFormatted("My:%i, Your:%i", 123, -531);
	sut.Get(inputBuffer, 17, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:123, Your:-531", inputBuffer);
}

void test_PutPrintfNoSpecifiers(void){
	char string[] = "Hello, World.";
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	
	sut.PutFormatted(string);
	sut.Get(inputBuffer, 13, MAXTIME);
	TEST_ASSERT_EQUAL_STRING(string, inputBuffer);
}

void test_PutPrintfHexSpecifiers(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%x, Your:%X", 0xAB, 0xCDE);
	sut.Get(inputBuffer, 15, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:AB, Your:CDE", inputBuffer);
}

void test_PutPrintfDecpad(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%10d", 1234);
	sut.Get(inputBuffer, 13, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:      1234", inputBuffer);
}

void test_PutPrintfDecpadSmaller(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%2d", 1234);
	sut.Get(inputBuffer, 13, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:1234", inputBuffer);
}

void test_PutPrinfHexpad(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%10x", 0x1234);
	sut.Get(inputBuffer, 13, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:      1234", inputBuffer);
}

void test_PutPrinfHexpadTooSmall(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%2x", 0x1234);
	sut.Get(inputBuffer, 13, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:1234", inputBuffer);
}


void test_PutPrinfHexpadZero(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%010x", 0x1234);
	sut.Get(inputBuffer, 13, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:0000001234", inputBuffer);
}

void test_PutPrintfChar(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%c", 'a');
	sut.Get(inputBuffer, 4, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:a", inputBuffer);
}

void test_PutPrintfString(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("My:%s", "World");
	sut.Get(inputBuffer, 8, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("My:World", inputBuffer);
}

void test_PutPrinfAllTogether(void){
	int size = 30;
	char inputBuffer[size];
	for(int i = 0; i < size; i++){
		inputBuffer[i] = 0;
	}
	sut.PutFormatted("%x%i%s%c%03x%4i", 0x23, 32, "hello", 'w', 0xF, 13);
	sut.Get(inputBuffer, 17, MAXTIME);
	TEST_ASSERT_EQUAL_STRING("2332hellow00F  13", inputBuffer);
}

// -----------------------------------------------------------------------------

void test_GetBuffer(void){
	char string[] = "Hello World!";
	int size = 12;
	char inputBuffer[size+1];
	inputBuffer[size] = 0; //null terminate
	sut.PutFormatted(string);
	TEST_ASSERT_EQUAL_INT(size, sut.Get(inputBuffer, size, MAXTIME));
	TEST_ASSERT_EQUAL_STRING(string, inputBuffer);
}
	
void test_GetBufferString(void){
	char string[] = "Hello World!\n";
	int size = 13;
	char buffer[50];
	sut.PutFormatted(string);
	TEST_ASSERT_EQUAL_INT(size, sut.Get(buffer, '\n'));
	TEST_ASSERT_EQUAL_STRING(string, buffer);
}
	
// -----------------------------------------------------------------------------

void transmitAlphabet(void * param){
	// ~60 ms long
	for(int i = 'A'; i <= 'z'; i++){
		sut.Put(i);
		waitcnt(CLKFREQ/1000 + CNT);
	}
	

	cogstop(cogid());
}

void test_CTSPinBasic(void){
	sut.Stop();
	sut.Start(rxpin, txpin, baud, ctspin);
	
	int stacksize = sizeof(_thread_state_t)+sizeof(int)*8;
	int *stack = (int*) malloc(stacksize);
	int cog = cogstart(transmitAlphabet, NULL, stack, stacksize);
	
	waitcnt(CLKFREQ*5/1000 + CNT); // Give it some time to transmit a few chars
	
	// Turn off transmission
	DIRA |= 1<<rtspin;
	OUTA |= 1<<rtspin;
	
	// Flush out buffer
	int current = 0;
	int last = current;
	while(current != -1){
		last = current;
		current = sut.Get(MAXTIME);
	}
	
	//At this point, no more characters.
	
	// Turn on transmission
	OUTA &= ~(1 << rtspin);
	waitcnt(CLKFREQ/10 + CNT); // Give it time to transmit the last of it's characters
	
	for(int i = last + 1; i <= 'z'; i++){
		TEST_ASSERT_EQUAL_INT(i, sut.Get(MAXTIME));
	}

	TEST_ASSERT_EQUAL_INT(-1, sut.Get(0));
}	
	
// -----------------------------------------------------------------------------
void test_PutBuffer(void){
	char data [] = "Hello, long string!";
	const int length = strlen(data) + 1;
	
	char inputbuffer[length];
	
	TEST_ASSERT_EQUAL_INT(length, sut.Put(data, length));
	
	sut.Get(inputbuffer, length, MAXTIME);
	
	TEST_ASSERT_EQUAL_STRING(data, inputbuffer);

}
		
	










































	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

