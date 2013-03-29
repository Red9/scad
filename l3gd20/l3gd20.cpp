#include "l3gd20.h"
bool L3GD20::Init(i2c * i2cbus)
{
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


bool L3GD20::ReadGyro(int& x, int& y, int& z)
{
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
