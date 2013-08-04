#include <propeller.h>

#include "unity.h"
#include "a6281.h"

#include "c++-alloc.h"

A6281 * sut = NULL;

const unsigned int pin_data = 8;
const unsigned int pin_latch = 7;
const unsigned int pin_enable = 6;
const unsigned int pin_clock = 5;

void setUp(void){
	sut = new A6281(pin_data, pin_latch, pin_enable, pin_clock);
}

void tearDown(void){
	delete sut;
}

void test_SetGreen(void){

	sut->Output(0x3FF, 0, 0);
	waitcnt(CLKFREQ + CNT);
	sut->Output(0, 0x3FF, 0);
	waitcnt(CLKFREQ + CNT);
	sut->Output(0, 0, 0x3FF);
	waitcnt(CLKFREQ + CNT);
	
	while(true){
		for(int k = 0x8F; k < 0x3FF; k++){
			sut->Output(0,0,k);
			waitcnt(CLKFREQ/1000 + CNT);
		}
		for(int k = 0x3FF; k >= 0x8F; k--){
			sut->Output(0,0,k);
			waitcnt(CLKFREQ/1000 + CNT);
		}
	}
	
	
	/*
	for(int i = 0; i < 0x3FF; i++){
		for(int j = 0; j < 0x3FF; j++){
			for(int k = 0; k < 0x3FF; k++){
				sut->Output(i,j,k);
				waitcnt(CLKFREQ/10000 + CNT);
			}
			for(int k = 0x3FF; k >= 0; k--){
				sut->Output(i,j,k);
				waitcnt(CLKFREQ/10000 + CNT);
			}
		}
		
		for(int j = 0x3FF; j >= 0; j--){
			for(int k = 0; k < 0x3FF; k++){
				sut->Output(i,j,k);
				waitcnt(CLKFREQ/10000 + CNT);
			}
			for(int k = 0x3FF; k >= 0; k--){
				sut->Output(i,j,k);
				waitcnt(CLKFREQ/10000 + CNT);
			}
		}
		
		
	}
	*/
	
}
