#ifndef i2cbase_Class_Defined__
#define i2cbase_Class_Defined__

#include <propeller.h>


//Note: refactoring this into it's own class took all of 8 bytes extra :^)

/** Low level I2C driver. Only does the most basic functions that all I2C
  * devices implement.
  *
  * Requires that the SDA and SCL pins have sufficient pullups. These should be
  * selected based on the capacitance of the devices on the I2C bus, and the
  * expected clock speed (400kHz currently).
  *
  * @todo(SRLM): rewrite the I2C sections to use inline assembly, and have more
  *              reliable (and clock changable) timings.
  *
  *
  * @author SRLM
  * @date   2013-01-21
  * @version 1.1
  
  Version History
  + 1.1 Updated the routines to use FCACHE'd inline assembly. It now runs faster
        and more precisely.
  + 1.0 Initial release.
  */
class i2cBase
{
public:

/** Acknowledge value. */
static const int kAck = 1;
/** No Acknowledge value.*/
static const int kNak = 0;

/** Set the IO Pins to float high. Does not require a cog.
  * @param scl The I2C SCL pin. Defaults to the Propeller default SCL pin.
  * @param sda The I2C SDA pin. Defaults to the Propeller default SDA pin.
  * @todo(SRLM): Move this function into the class constructor.
  * @returns Nothing, right now...
  */
int Initialize(int scl = 28, int sda = 29);

/** Output a start condition on the I2C bus.
  */
void Start();

/** Output a stop condition on the I2C bus.
  */
void Stop();

/** Ouput a byte on the I2C bus.
  * @param   byte the 8 bits to send on the bus.
  * @todo(SRLM): change the return to a bool instead of an int.
  * @returns      the acknowledge (or not) from the devices on the bus.
  */
int SendByte(unsigned char byte);

/** Input a byte from the I2C bus.
  * @todo(SRLM): change the parameter to a bool instead of int.
  * @param acknowledge true to acknowledge the byte received, false otherwise.
  * @returns the byte clocked in off the bus.
  */
unsigned char ReadByte(bool acknowledge);

private:
unsigned int SCLMask;
unsigned int SDAMask;


	
	
	//Clock delay values:
	// 1600 == 25kHz
	//  400 == 100kHz
	//  100 == 400kHz
	//   90 == 444kHz
	//   32 == 1.25MHz
	
int clockDelay;

};



#endif
