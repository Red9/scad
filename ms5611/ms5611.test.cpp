#include <propeller.h>

#include "unity.h"

#include "ms5611.h"
#include "c++-alloc.h"


const int kPIN_I2C_SCL = 0;
const int kPIN_I2C_SDA = 1;

i2c * bus = nullptr;
MS5611 * sut = nullptr;


void setUp(void){
	bus = new i2c;
	bus->Initialize(kPIN_I2C_SCL, kPIN_I2C_SDA);
	sut = new MS5611(bus);
}

void tearDown(void){
    sut->Reset();

	delete sut;
//	sut = nullptr;
	delete bus;
//	bus = nullptr;
}

void test_GetStatus(void){
	TEST_ASSERT_TRUE(sut->GetStatus());
}


void test_GetPressureTemperatureBasic(void){
//This test method is mostly for testing the timing of the functions

	int pressure = 0;
	int temperature = 0;
	
//	int startCNT = CNT;
//	int endCNT = CNT;
//	int nothingDelta = endCNT-startCNT;

//	while(true){
	//Touch 1
	waitcnt(CLKFREQ/100 + CNT);
//	startCNT = CNT;
	TEST_ASSERT_FALSE(sut->Touch()); //Pressure
//	endCNT = CNT;
	
//	UnityPrint("Touch1 deltaCNT: ");
//	UnityPrintNumber(endCNT - startCNT - nothingDelta);
//	UNITY_OUTPUT_CHAR('\n');
	
	
	//Touch 2
	waitcnt(CLKFREQ/100 + CNT);
//	startCNT = CNT;
	TEST_ASSERT_TRUE(sut->Touch()); //Temperature
//	endCNT = CNT;
	
//	UnityPrint("Touch2 deltaCNT: ");
//	UnityPrintNumber(endCNT - startCNT - nothingDelta);
//	UNITY_OUTPUT_CHAR('\n');
	
	
	//Conversion
//	startCNT = CNT;
	sut->Get(pressure, temperature);
//	endCNT = CNT;
	
//	UnityPrint("Get deltaCNT: ");
//	UnityPrintNumber(endCNT - startCNT - nothingDelta);
//	UNITY_OUTPUT_CHAR('\n');

	
	TEST_ASSERT_TRUE(pressure != 0);
	TEST_ASSERT_TRUE(temperature != 0);
	
//	UnityPrint("Temperature: ");
//	UnityPrintNumber(temperature);
//	UNITY_OUTPUT_CHAR('\t');
//	UnityPrint("Pressure: ");
//	UnityPrintNumber(pressure);
//	UNITY_OUTPUT_CHAR('\n');
//	}
}


void test_CalculateHighTemperature(void){
	sut->SetC(	40127,
				36924,
				23317,
				23282,
				33464,
				28312);
				
	sut->TEST_SetD(9085466, 8569150);
				
	int pressure, temperature;
	sut->Get(pressure, temperature);
	TEST_ASSERT_EQUAL_INT(2007, temperature);
	TEST_ASSERT_EQUAL_INT(100009, pressure);

}

void test_SetGetC(void){
	int C[] = { 40127,
				36924,
				23317,
				23282,
				33464,
				28312};
	int rC[6];
	
				
	sut->SetC(C[0], C[1], C[2], C[3], C[4], C[5]);
	sut->GetC(rC[0], rC[1], rC[2], rC[3], rC[4], rC[5]);
	
	TEST_ASSERT_EQUAL_INT_ARRAY(C, rC, 6);
}

void test_GetRaw(void){

	int D1 = 52352;
	int D2 = 87950;
	sut->TEST_SetD(D1, D2);
	
	int p, t;
	sut->Get(p, t, false);

	TEST_ASSERT_EQUAL_INT(D1, p);
	TEST_ASSERT_EQUAL_INT(D2, t);
}

void test_GetProccessedIsNotRaw(void){

	int D1 = 52352;
	int D2 = 87950;
	sut->TEST_SetD(D1, D2);
	
	int p, t;
	sut->Get(p, t); // default of true

	TEST_ASSERT_FALSE(D1 == p);
	TEST_ASSERT_FALSE(D2 == t);
}


void test_TouchTimeoutEffectWhenCalledQuickly(void){
	for(int i = 0; i < 25; i++){
		for(int j = 0; j < 17; j++){
			waitcnt(CLKFREQ/1000 + CNT);
			TEST_ASSERT_FALSE(sut->Touch());
		}
		waitcnt(CLKFREQ/1000 + CNT);
		TEST_ASSERT_TRUE(sut->Touch());
		TEST_ASSERT_FALSE(sut->Touch());
	}
}

void test_TouchTimeoutHasNoEffectWhenCalledSlowly(void){
	for(int i = 0; i < 100; i++){
		waitcnt(CLKFREQ/100 + CNT);
		TEST_ASSERT_FALSE(sut->Touch());
		waitcnt(CLKFREQ/100 + CNT);
		TEST_ASSERT_TRUE(sut->Touch());
	}
}
































