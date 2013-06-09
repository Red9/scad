#ifndef SRLM_PROPGCC_I2CBASE_H__
#define SRLM_PROPGCC_I2CBASE_H__

#include <propeller.h>

//Note: refactoring this into it's own class took all of 8 bytes extra :^)

/** Low level I2C driver. Only does the most basic functions that all I2C
devices implement.

Requires that the SDA and SCL pins have sufficient pullups. These should be
selected based on the capacitance of the devices on the I2C bus, and the
expected clock speed (400kHz currently).


@author SRLM
@date   2013-01-21
@version 1.1

Version History
+ 1.1 Updated the routines to use FCACHE'd inline assembly. It now runs faster
      and more precisely.
+ 1.0 Initial release.
*/
class i2cBase
{
public:


/** Set the IO Pins to float high. Does not require a cog.

Sets the bus speed to 444kHz.

@param scl The I2C SCL pin. Defaults to the Propeller default SCL pin.
@param sda The I2C SDA pin. Defaults to the Propeller default SDA pin.
@returns Nothing, right now...
*/
void Initialize(const int scl = 28, const int sda = 29);

/** Output a start condition on the I2C bus.
*/
void Start();

/** Output a stop condition on the I2C bus.
*/
void Stop();

/** Ouput a byte on the I2C bus.
@param   byte the 8 bits to send on the bus.
@returns      true if the device acknowledges, false otherwise.
*/
bool SendByte(const unsigned char byte);

/** Get a byte from the I2C bus.

@param acknowledge true to acknowledge the byte received, false otherwise.
@returns the byte clocked in off the bus.
*/
unsigned char ReadByte(const bool acknowledge);

private:
unsigned int SCLMask;
unsigned int SDAMask;


	//Clock delay values (@80MHz system clock):
	// 1600 == 25kHz
	//  400 == 100kHz
	//  100 == 400kHz
	//   90 == 444kHz
	//   32 == 1.25MHz
int clockDelay;

};



#endif // SRLM_PROPGCC_I2CBASE_H__
