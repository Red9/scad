#ifndef SRLM_PROPGCC_A6281_H__
#define SRLM_PROPGCC_A6281_H__

#include "pin.h"
#include <propeller.h>

class A6281{
public:
	A6281 (const unsigned int pin_data, const unsigned int pin_clock,
		   const unsigned int pin_latch, const unsigned int pin_enable);
	void Output(unsigned int value);
	void Output(int red, int green, int blue);
private:
	Pin data;
	Pin latch;
	Pin clock;
	Pin enable;
};








#endif //SRLM_PROPGCC_A6281_H__
