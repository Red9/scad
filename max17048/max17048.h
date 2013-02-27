
#ifndef SRLM_PROPGCC_MAX17048_H_
#define SRLM_PROPGCC_MAX17048_H_

#include "i2c.h"

class MAX17048{
public:
MAX17048(i2c * newbus);
bool GetStatus(void);


int GetVersion(void);

/**
@returns the current voltage in units of 1mv/LSb
*/
int GetVoltage(void);

/**
@todo Figure out what voltage the 0% SOC is at, and if we can change it (since
we can only run down to about 3.45V.

@returns the current state of charge in units of 1%/LSb
*/
int GetStateOfCharge(void);

/**

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
