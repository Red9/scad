/**`Interface to the MAX17048 Fuel Chip

This chip montiors a single cell Lithium battery, and calculates metrics of
battery use.

Hardware: The MAX17048 should be connected as described in the datasheet. The
only connections required to the Propeller are via the I2C bus.

One MAX17948 can be connected per I2C bus.

@TODO(SRLM): Add temperature compensation


@author SRLM
@date   2013-02-10

*/
#ifndef SRLM_PROPGCC_MAX17048_H_
#define SRLM_PROPGCC_MAX17048_H_

#include "i2c.h"

class MAX17048{
public:

/**
Create a MAX17048 object.

@param newbus The I2C bus that the MAX17048 is on.
*/
MAX17048(i2c * newbus);

/** Test the presence of the MAX17048 chip by pinging the bus.

@return true if the MAX17048 is found, false otherwise.
*/
bool GetStatus(void);

/** Fetches the hardware version of the MAX17048.

@returns the version
*/
int GetVersion(void);

/** Use the MAX17048 internal ADC, and get the voltage

@returns the current voltage in units of 1mv/LSb
*/
int GetVoltage(void);

/**Get the state of charge from the chip.

The state of charge refers to the overall percentage of the battery remaining.
The MAX17048 chip automatically adjusts for a number of features, including
battery capacity.

@todo Figure out what voltage the 0% SOC is at, and if we can change it (since
we can only run down to about 3.45v with MAX8819 for 3.3v systems.

@returns the current state of charge in units of 1%/LSb
*/
int GetStateOfCharge(void);

/** Fetches the rate of charge (positive or negative)

The charge rate is the change in battery state of charge over time. It should
not be used to calculate current consumption (eg milliamps).

@todo(SRLM): Figure out the sign issues with this.
@returns the charge rate in units of 0.1%/hr. Note: Positive or negive for
*/
int GetChargeRate(void);

private:

i2c * bus;
bool status;




const static unsigned char deviceFuel = 0b01101100;

const static unsigned char kVCELL = 0x02;
const static unsigned char kSOC = 0x04;
const static unsigned char kVERSION = 0x08;
const static unsigned char kCRATE = 0x16;


int GetShort(char address);

};


#endif // SRLM_PROPGCC_MAX17048_H_
