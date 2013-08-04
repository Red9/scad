#ifndef SRLM_PROPGCC_L3GD20_H_
#define SRLM_PROPGCC_L3GD20_H_

#ifndef UNIT_TEST
#include "i2c.h"
#else
#include "i2cMOCK.h"
#endif


/** Provides an interface the L3GD20 gyroscope.

"Output Data Rate, in digital-output accelerometers, defines the rate at which
data is sampled. Bandwidth is the highest frequency signal that can be sampled
without aliasing by the specified Output Data Rate. Per the Nyquist sampling
criterion, bandwidth is half the Output Data Rate." -Analog Devices

@author SRLM (srlm@srlmproductions.com)
@date   2013-01-01
@version 1.1

Version History
	+ 1.1 - Updated to use boolean return values.
	+ 1.0 - Initial release


*/
class L3GD20{

public:

/**
Set the control registers of the device:

Returns true if successful, false otherwise.
	CTRL_REG1
	-ODR = 760 Hz
	-Cut-Off = 100
	-Normal power
	-XYZ enabled

	CTRL_REG4:
	-Continuous block data update (default)
	-Data LSb at lower address (default)
	-2000+- degrees per second
	-(no functionality)
	-00 (no functionality)
	-SPI interface mode off

@param i2cbus the I2C bus that the gyro is connected to.
@return true when the gyro is successfully initialized.
*/
	bool Init(i2c * i2cbus){
            	bus = i2cbus;

	//Check to make sure the gyro is actually there.
	status = bus->Ping(device);
	if(status == false){
		return false;
	}

	bus->Put(device, kCTRL_REG1, 0b11111111);
	bus->Put(device, kCTRL_REG4, 0b00110000);
	
	return true;
        }
	
/**
Get the current rotation readings from the gyroscope.

If there is an error then x, y, and z will be set to zero and false will be
returned.

@param x The gyroscope x axis value. Will be overwritten.
@param y The gyroscope y axis value. Will be overwritten.
@param z The gyroscope z axis value. Will be overwritten.
@return true if all is successful, false otherwise. If false, try reinitilizing
*/
	bool ReadGyro(int& x, int& y, int& z){
            	char data[6];
	
	if(status == false){
		x = y = z = 0;
		return false;
	}
	
	if(bus->Get(device, kOUT_X_L, data, 6) == false){
		x = y = z = 0;
		return false;
	}
	x = ((data[0] | (data[1] << 8)) << 16) >> 16;
	y = ((data[2] | (data[3] << 8)) << 16) >> 16;
	z = ((data[4] | (data[5] << 8)) << 16) >> 16;

	return true;
        }

private:
	i2c * bus;
	
	int status;

	const static unsigned char device = 0b11010110;	
	const static unsigned char kCTRL_REG1 = 0x20;
	const static unsigned char kCTRL_REG4 = 0x23;
	const static unsigned char kOUT_X_L =   0x28  | 0x80; //(turn on auto increment)
}; 

#endif // SRLM_PROPGCC_L3GD20_H_
