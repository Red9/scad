#include "a6281.h"

A6281::A6281 (const unsigned int pin_data, const unsigned int pin_latch,
		   const unsigned int pin_enable, const unsigned int pin_clock)
		   :data(pin_data), latch(pin_latch),
		   clock(pin_clock), enable(pin_enable){
	data.low();
	latch.low();
	clock.low();
	enable.low();

}

void A6281::Output(unsigned int value){
	value &= 0x3fffFfff; //Mask the upper 2 bits
	for(int i = 0; i < 32; i++){
		if((value & 0x80000000) != 0){
			data.high();
		}else{
			data.low();
		}
		value <<= 1;
		clock.high();
		clock.low();
	}
	data.low();
	latch.high();
	clock.high();
	latch.low();
	clock.low();

}

void A6281::Output(int red, int green, int blue){
	unsigned int value = (blue  & 0x3FF) << 20
	                    |(red   & 0x3FF) << 10
	                    |(green & 0x3FF) << 0;
	Output(value);

}
