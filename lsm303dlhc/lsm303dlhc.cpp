#include "lsm303dlhc.h"
#include <stdio.h>
bool LSM303DLHC::Init(i2c * i2cbus)
{
	bus = i2cbus;

	//Check to make sure the LSM303DLHC is actually there.
	status = bus->Ping(deviceMagn);
	if(status == bus->kNak)
		return false;
	status = bus->Ping(deviceAccl);
	if(status == bus->kNak)
		return false;
		
	
	//Initialize Magn
	bus->Put(deviceMagn, kCRA_REG_M, 0b10011100);
	bus->Put(deviceMagn, kCRB_REG_M, 0b11100000);
	bus->Put(deviceMagn, kMR_REG_M, 0b00000000);
	
	//Initialize Accl
	bus->Put(deviceAccl, kCTRL_REG1_A, 0b10010111);
	bus->Put(deviceAccl, kCTRL_REG4_A, 0b00111000);
	
	

	return true;
}


bool LSM303DLHC::ReadAccl(int& x, int& y, int& z)
{
	char data[6];
	
	//TODO(SRLM): Do we want this? It might be better to ping the bus, and see if there is a response (rather than blindly returning)
	if(status == bus->kNak)
		return false;
	
	if(bus->Get(deviceAccl, kOUT_X_L_A, data, 6) == bus->kNak)
		return false;
	
	//16 + 4 = 20, or the 12 bit data given by the LSM303DLHC.
	x = ((data[0] | (data[1] << 8)) << 16) >> 20;
	y = ((data[2] | (data[3] << 8)) << 16) >> 20;
	z = ((data[4] | (data[5] << 8)) << 16) >> 20;

	return true;

}

bool LSM303DLHC::ReadMagn(int& x, int& y, int& z)
{
	char data[6];
	
	//TODO(SRLM): Do we want this? It might be better to ping the bus, and see if there is a response (rather than blindly returning)
	if(status == bus->kNak)
		return false;
	
	if(bus->Get(deviceMagn, kOUT_X_H_M, data, 6) == bus->kNak)
		return false;
	
//	for(int i = 0; i < 6; ++i)
//	{
//		printf("\ndata[%i] = 0x%X  ", i, data[i]);
//	}
	
	//Warning: the LSM303DLHC datasheet lists the magnetometer high registers
	//first, instead of the low (backwards compared to the accel and L3GD20).
	//16 + 4 = 20, or the 12 bit data given by the LSM303DLHC.
	x = (((int)(data[1]) | ((int)(data[0]) << 8)) << 16) >> 20;
	y = ((data[3] | (data[2] << 8)) << 16) >> 20;
	z = ((data[5] | (data[4] << 8)) << 16) >> 20;

	return true;

}


