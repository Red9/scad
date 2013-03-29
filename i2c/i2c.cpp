

#include <propeller.h>
#include "i2c.h"


void i2c::Initialize(int SCLPin, int SDAPin)
{
	base.Initialize(SCLPin, SDAPin);
}

bool i2c::Ping(unsigned char device)
{
	base.Start();
	bool result = base.SendByte(device);
	base.Stop();
	return result;
}




// -----------------------------------------------------------------------------
// Single Byte
// -----------------------------------------------------------------------------

bool i2c::Put(unsigned char device, unsigned char address, char byte)
{
	bool result;
	
	base.Start();
	result  = base.SendByte(device);
	result &= base.SendByte(address);
	result &= base.SendByte(byte);
	base.Stop();
	
	return result;
}

unsigned char i2c::Get(unsigned char device, unsigned char address)
{
	bool result;
	
	base.Start();
	result &= base.SendByte(device);
	result &= base.SendByte(address);
	
	base.Start();
	result &= base.SendByte(device | 0x01); //Set read bit
	unsigned char dataByte = base.ReadByte(false);
	base.Stop();
	return dataByte;
}

// -----------------------------------------------------------------------------
// Multiple Bytes
// -----------------------------------------------------------------------------

bool i2c::Put(unsigned char device, unsigned char address, char * bytes, int size)
{
	bool result;
	base.Start();
	result   = base.SendByte(device);
	result  &= base.SendByte(address);

	for(int i = 0; i < size; ++i)
	{
		result &= base.SendByte(bytes[i]);
	}
	base.Stop();
	
	return result;
}

bool i2c::Get(unsigned char device, unsigned char address, char * bytes, int size)
{
	bool result;
	base.Start();
	result  = base.SendByte(device);
	result &= base.SendByte(address); //Assert the read multiple bytes bit (ref: L3GD20 datasheet)
	base.Start();
	result &= base.SendByte(device | 0x01);
	
	int i = 0;
	for(; i < size - 1; ++i)
	{
		bytes[i] = base.ReadByte(true); //Send true to keep on reading bytes
	}
	
	bytes[i] = base.ReadByte(false); //Trailing NAK
	base.Stop();
	
	return result;
	
}


// -----------------------------------------------------------------------------
// Single Byte payload
// -----------------------------------------------------------------------------
bool i2c::Put(unsigned char device, char byte)
{
	//Warning: this method is not unit tested! (it's run, but the MS5611 does
	//not have a register that can be written to and read from).
	
	base.Start();
	bool result = base.SendByte(device);
	result     &= base.SendByte(byte);
	base.Stop();
	
	return result;
}

bool i2c::Get(unsigned char device, char * bytes, int size)
{
	base.Start();
	bool result = base.SendByte(device | 0x01);
	int i = 0;
	for(; i < size - 1; ++i)
	{
		bytes[i] = base.ReadByte(true);
	}
	bytes[i] = base.ReadByte(false);
	base.Stop();
	
	return result;
}






