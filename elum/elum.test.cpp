#include "unity.h"
#include "elum.h"
#include "c++-alloc.h"

const int kPIN_LEDR           = 5;
const int kPIN_MAX8819_EN123  = 6;
const int kPIN_LEDG           = 7;
const int kPIN_BUTTON         = 8;


Elum * sut;


void setUp(void){
	sut = new Elum(kPIN_LEDR, kPIN_LEDG, kPIN_BUTTON);
}

void tearDown(void){
	delete sut;
}

//void test_Pattern(void){
//	sut->Pattern(Elum::kJitterFast);
//	
//	
//}

void test_LEDs(void){
	sut->On(Elum::RED);
	waitcnt(CLKFREQ*2 + CNT);
	sut->Fade(10);
	waitcnt(CLKFREQ*2 + CNT);
	sut->Pattern(Elum::kManyFast);
	waitcnt(CLKFREQ*2 + CNT);

}
