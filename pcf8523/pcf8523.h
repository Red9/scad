/** Interface to the PCF8523 Real Time Clock (RTC).
@author SRLM
@date   2013-01-24
*/

#ifndef SRLM_PROPGCC_PCF8523_H_
#define SRLM_PROPGCC_PCF8523_H_

#include <propeller.h>
#include "i2c.h"


class PCF8523{
public:

/**


Set the control registers of the device:


   - CONTROL_1 (set to 0b10000000):
     + Set to 12.5pF capacitor
     + 0 (no functionality)
     + Set to RTC time circuits running
     + No software reset
     + 24 Hour mode
     + second interrupt disabled
     + alrarm interrupt disabled
     + no correction input generated
   - CONTROL_2 (set to 0b00000000):
     + 0 (no functionality on write)
     + 0 (no functionality on write)
     + 0 (no functionality on write)
     + 0 (no functionality on write)
     + 0 (no functionality on write)
     + Disable watchdog timer A interrupt.
     + Disable countdown timer A interrupt.
     + Disable countdown timer B interrupt.
   - CONTROL_3 (set to 0b00000000):
     + Battery switch over (standard mode), batery low detection enabled
     + 0 (no functionality)
     + Reset battery switchover interupt (whether set or not)
     + 0 (no functionality on write)
     + Set to no interrupt when battery switch over flag is set
     + Set to no interrupt when battery low flag is set
   

@param newbus The i2c bus to use.
@param newkPIN_SQW The square wave pin. If not used leave out or set to -1.
*/
PCF8523(i2c * newbus, int newkPIN_SQW = -1);
bool GetStatus();
bool SetClock(int year, int month, int day,
                       int hour, int minute, int second, int weekday = 0);
bool GetClock(int & year, int & month, int & day,
                       int & hour, int & minute, int & second, int & weekday);
bool GetClock(int & year, int & month, int & day,
                       int & hour, int & minute, int & second);


private:
i2c * bus;
bool status;
int kPIN_SQW;




const static unsigned char deviceRtc = 0b11010000;

const static unsigned char kCONTROL_1 = 0x00;
const static unsigned char kCONTROL_2 = 0x01;
const static unsigned char kCONTROL_3 = 0x02;
const static unsigned char kSECONDS   = 0x03;
	
public:
/** Convert an integer representation of a number into it's Binary Coded Digit
equivalent.

@warning Converts at most 2 binary coded digits (up to a value of 99).
@warning Undefined operation with negative numbers
@warning Should not be used publicly: made public for use with testing.

@param number the number to convert.
@returns the 8 bits (2 digits) of binary coded number.
*/
char ConvertToBCD(int number);
	
int ConvertFromBCD(unsigned char bcd);	
};


#endif // SRLM_PROPGCC_PCF8523_H_
