#ifndef SRLM_PROPGCC_L3GD20_H_
#define SRLM_PROPGCC_L3GD20_H_

#ifndef UNIT_TEST
#include "i2c.h"
#else
#include "i2cMOCK.h"
#endif


/*
"Output Data Rate, in digital-output accelerometers, defines the rate at which
data is sampled. Bandwidth is the highest frequency signal that can be sampled
without aliasing by the specified Output Data Rate. Per the Nyquist sampling
criterion, bandwidth is half the Output Data Rate." -Analog Devices

*/
class L3GD20{

public:

/**
* Set the control registers of the device:
*
* Returns true if successful, false otherwise.
*	
*		CTRL_REG1
*		-ODR = 760 Hz
*		-Cut-Off = 100
*		-Normal power
*		-XYZ enabled
*
*		CTRL_REG4:
*		-Continuous block data update (default)
*		-Data LSb at lower address (default)
*		-2000+- degrees per second
*		-(no functionality)
*		-00 (no functionality)
*       -SPI interface mode off
*/
	bool Init(i2c * i2cbus);
	
	bool ReadGyro(int& x, int& y, int& z);

private:
	i2c * bus;
	
	int status;

	const static unsigned char device = 0b11010110;	
	const static unsigned char kCTRL_REG1 = 0x20;
	const static unsigned char kCTRL_REG4 = 0x23;
	const static unsigned char kOUT_X_L =   0x28  | 0x80; //(turn on auto increment)
}; 

#endif // SRLM_PROPGCC_L3GD20_H_
