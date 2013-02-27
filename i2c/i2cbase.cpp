#include <propeller.h>
#include "i2cbase.h"



#define i2c_float_scl_high (DIRA &= ~SCLMask)
#define i2c_set_scl_low    (DIRA |= SCLMask)
#define i2c_float_sda_high (DIRA &= ~SDAMask)
#define i2c_set_sda_low    (DIRA |= SDAMask)

void i2cBase::Start()
{
	i2c_float_sda_high;
	i2c_float_scl_high;
	i2c_set_sda_low;
	i2c_set_scl_low;
}

void i2cBase::Stop()
{
	i2c_float_scl_high;
	i2c_float_sda_high;
}

unsigned char i2cBase::ReadByte(int acknowledge)
{
    uint8_t byte = 0;
    int count;

    i2c_float_sda_high;

    for (count = 8; --count >= 0; ) {
        byte <<= 1;
        i2c_float_scl_high;
        waitcnt(CNT + halfCycle);
        byte |= (INA & SDAMask) ? 1 : 0;
        i2c_set_scl_low;
        waitcnt(CNT + halfCycle);
    }

    // acknowledge
    if (acknowledge)
        i2c_set_sda_low;
    else
        i2c_float_sda_high;
    //Send SCL 
    //Clock out the ack bit
    i2c_float_scl_high;
    i2c_set_scl_low;
    
    i2c_set_sda_low;

    return byte;
}

int i2cBase::SendByte(unsigned char byte)
{
    int count, result;

    /* send the byte, high bit first */
    for (count = 8; --count >= 0; ) {
    	//Change byte while clock is low
        if (byte & 0x80)
            i2c_float_sda_high;
        else
            i2c_set_sda_low;
            
        //Send clock high pulse
        i2c_float_scl_high;
        waitcnt(CNT + halfCycle);
		//Prepare byte for next time around
        byte <<= 1;

        i2c_set_scl_low;
        
     } 
  

    /* receive the acknowledgement from the slave */
    //Release SDA line
    i2c_float_sda_high;
  
    //Send clock high pulse and read ACK/NAK bit
    i2c_float_scl_high;
    result = !((INA & SDAMask) != 0);
    i2c_set_scl_low;
    
    //Pull SDA low
    i2c_set_sda_low;

    return result; //Ack or NAK

}

int i2cBase::Initialize(int SCLPin, int SDAPin)
{
	SCLMask = 1 << SCLPin;
	SDAMask = 1 << SDAPin;
	
	
	//Set pins to input
	i2c_float_scl_high;
	i2c_float_sda_high;
	
	
	//Set outputs low
	OUTA &= ~SCLMask;
	OUTA &= ~SDAMask;
	
	halfCycle = 60;
}
