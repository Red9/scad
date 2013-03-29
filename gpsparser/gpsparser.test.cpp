#include <string.h>

#include "unity.h"
#include "c++-alloc.h"

#include "serial.h"
#include "gpsparser.h"

const int kPIN_USER_1 = 18;
const int kPIN_USER_2 = 19;

/*

All tests requires that pins 18 and 19 be connected by a resistor.

*/



GPSParser * sut;

void setUp(void){
	sut = new GPSParser(kPIN_USER_1, kPIN_USER_2, 9600);
}

void tearDown(void){
	delete sut;
}

const char * FillBuffer(const char * string){
	for(int i = 0; string[i] != '\0'; i++){
		sut->Put(string[i]);
	}
	
	return string;
}



//void test_fail(void){
//	TEST_FAIL();
//}

void test_GetIncompleteString(void){
	//const char * line = 
	FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.");
	TEST_ASSERT_EQUAL_INT(NULL, sut->Get());
}

void test_GetCompleteString(void){
	const char * line =
		FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");
	TEST_ASSERT_EQUAL_MEMORY(line, sut->Get(), strlen(line)-2);
}
	
void test_GetMultipleStringsNoWait(void){
	const char * line0 =
		FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n" );
	const char * line1 =
		FillBuffer("$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32\r\n");
	const char * line2 =
		FillBuffer("$PGTOP,11,2*6E\r\n");
		
	TEST_ASSERT_EQUAL_MEMORY(line0, sut->Get(), strlen(line0)-2);
	TEST_ASSERT_EQUAL_MEMORY(line1, sut->Get(), strlen(line1)-2);
	TEST_ASSERT_EQUAL_MEMORY(line2, sut->Get(), strlen(line2)-2);
}

void test_NullAfterString(void){
	const char * line =
		FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");
	
	TEST_ASSERT_EQUAL_MEMORY(line, sut->Get(), strlen(line)-2);
	TEST_ASSERT_EQUAL_INT(NULL, sut->Get());
}

void test_GetPartialStringAtBeginning(void){
	//const char * line0 = 
		FillBuffer(".00,N,0.00,K,N*32\r\n");
	const char * line1 =
		FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");
		
	TEST_ASSERT_EQUAL_MEMORY(line1, sut->Get(), strlen(line1)-2);
	TEST_ASSERT_EQUAL_INT(NULL, sut->Get());
	
}
	
	



//void test_getGPS(void){
////Routine to echo GPS received data to terminal.
//	const int kPIN_GPS_TX = 24; //Tx from the Propeller
//	const int kPIN_GPS_RX = 25; //Rx to the Propeller
//	const int kPIN_GPS_FIX = 26;
//	Serial gps;
//	gps.Start(kPIN_GPS_RX, kPIN_GPS_TX, 9600);
//	for(;;){
//		printf("%c", gps.Get());
//	}
//}











/*
Sample GPS Data, no Fix:

$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32
$PGTOP,11,2*6E
$GPGGA,180253.087,,,,,0,0,,,M,,M,,*4A
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPRMC,180253.087,V,,,,,0.00,0.00,290113,,,N*47
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32
$PGTOP,11,2*6E
$GPGGA,180254.087,,,,,0,0,,,M,,M,,*4D
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPRMC,180254.087,V,,,,,0.00,0.00,290113,,,N*40
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32
$PGTOP,11,2*6E
$GPGGA,180255.087,,,,,0,0,,,M,,M,,*4C
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPGSV,1,1,00*79
$GPRMC,180255.087,V,,,,,0.00,0.00,290113,,,N*41
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32

*/
