

#include <propeller.h>
#include "i2c.h"


int i2c::Initialize(int SCLPin, int SDAPin)
{
	return base.Initialize(SCLPin, SDAPin);
}

int i2c::Ping(unsigned char device)
{
	int result = 0;
	base.Start();
	result = base.SendByte(device);
	base.Stop();
	return result;
}




// -----------------------------------------------------------------------------
// Single Byte
// -----------------------------------------------------------------------------

unsigned char i2c::Put(unsigned char device, unsigned char address, char byte)
{
	base.Start();
	base.SendByte(device);
	base.SendByte(address);
	//TODO(SRLM): Ack?
	base.SendByte(byte);
	base.Stop();
	
	return kAck;
}

unsigned char i2c::Get(unsigned char device, unsigned char address)
{
	base.Start();
	base.SendByte(device);
	base.SendByte(address);
	//TODO(SRLM): Ack?
	
	base.Start();
	base.SendByte(device | 0x01); //Set read bit
	unsigned char result = base.ReadByte(false);
	base.Stop();
	return result;
}

// -----------------------------------------------------------------------------
// Multiple Bytes
// -----------------------------------------------------------------------------

unsigned char i2c::Put(unsigned char device, unsigned char address, char * bytes, int size)
{
	base.Start();
	base.SendByte(device);
	base.SendByte(address);
	//TODO(SRLM): Ack?
	for(int i = 0; i < size; ++i)
	{
		base.SendByte(bytes[i]);
	}
	base.Stop();
	
	return kAck;
}

unsigned char i2c::Get(unsigned char device, unsigned char address, char * bytes, int size)
{
	base.Start();
	base.SendByte(device);
	base.SendByte(address); //Assert the read multiple bytes bit (ref: L3GD20 datasheet)
	base.Start();
	base.SendByte(device | 0x01);
	
	int i = 0;
	for(; i < size - 1; ++i)
	{
		bytes[i] = base.ReadByte(true); //Send true to keep on reading bytes
	}
	
	bytes[i] = base.ReadByte(false); //Trailing NAK
	base.Stop();
	
	return kAck;
	
}


// -----------------------------------------------------------------------------
// Single Byte payload
// -----------------------------------------------------------------------------
unsigned char i2c::Put(unsigned char device, char byte)
{
	//Warning: this method is not unit tested! (it's run, but the MS5611 does
	//not have a register that can be written to and read from.
	
	base.Start();
	base.SendByte(device);
	base.SendByte(byte);
	base.Stop();
	
	return kAck;
}

unsigned char i2c::Get(unsigned char device, char * bytes, int size)
{
	base.Start();
	base.SendByte(device | 0x01);
	int i = 0;
	for(; i < size - 1; ++i)
	{
		bytes[i] = base.ReadByte(true);
	}
	bytes[i] = base.ReadByte(false);
	base.Stop();
	
	return kAck;
}






