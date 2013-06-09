/** Interface to the PCF8523 Real Time Clock (RTC).
 * 
 * Hardware connections: The PCF8523 should be connected to an I2C bus. If you 
 * want the clock to keep ticking after power is removed, make sure to attach a 
 * battery to the batt pin. The SQW pin can optionally be connected. At this 
 * time, this class does not use that pin for any purpose.
 * 
 * Possible improvements:
 *   + Use the OS bit to keep track of oscillator state.
 *   + Make use of the SQW pin.
 * 
 * @author SRLM
 */

#ifndef SRLM_PROPGCC_PCF8523_H_
#define SRLM_PROPGCC_PCF8523_H_

#include <propeller.h>
#include "i2c.h"

class PCF8523 {
public:

    /**    
     * Set the control registers of the device:
       - CONTROL_1 (set to 0b10000000):
         + Set to 12.5pF capacitor (matches crystal)
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
     * @param newbus The i2c bus to use.
     * @param newkPIN_SQW The square wave pin. If not used leave out or set to -1.
     */
    PCF8523(i2c * newbus, int newkPIN_SQW = -1);

    /** Test to see if the device is present and readable. Sends a ping on the 
     * i2c bus.
     * 
     * @return present (true) or missing (false).
     */
    bool GetStatus();

    /** Sets the time on the RTC. The various limits (ie, month less than 12) 
     * are not checked.
     * 
     * If the RTC chip has a battery backup, then the time will persist across 
     * power cycles.
     *
     * @param year    The year to set, in the range 0 to 99.
     * @param month   The month to set, in the range 1 to 12.
     * @param day     The day to set, in the range 1 to 31.
     * @param hour    The hour to set, in the range 0 to 23.
     * @param minute  The minute to set, in the range 0 to 59.    
     * @param second  The second to set, in the range 0 to 59.
     * @param weekday The weekday to set, in the range of 0 to 6. You can set 
     *                 and interpret this anyway that you want: it's simply 
     *                 incremented when the day changes. 
     * @return        true if the clock was successfully set, false otherwise. 
     */
    bool SetClock(int year, int month, int day,
            int hour, int minute, int second, int weekday = 0);

    /**
     * Get the current time from the RTC chip. Note that the parameters are 
     * passed by reference.
     * 
     * @param year    The current year, in the range 0 to 99.
     * @param month   The current month, in the range 1 to 12.
     * @param day     The current day, in the range 1 to 31.
     * @param hour    The current hour, in the range 0 to 23.
     * @param minute  The current second, in the range 0 to 59.
     * @param second  The current second, in the range 0 to 59.
     * @param weekday The current weekday, in the range 0 to 6.
     * @return        true if the RTC chip was successfully read, and false otherwise.
     */
    bool GetClock(int & year, int & month, int & day,
            int & hour, int & minute, int & second, int & weekday);

    /**
     * Get the current time from the RTC chip. Note that the parameters are 
     * passed by reference.
     * 
     * @param year   The current year, in the range 0 to 99.
     * @param month  The current month, in the range 1 to 12.
     * @param day    The current day, in the range 1 to 31.
     * @param hour   The current hour, in the range 0 to 23.
     * @param minute The current second, in the range 0 to 59.
     * @param second The current second, in the range 0 to 59.
     * @return       true if the RTC chip was successfully read, and false otherwise.
     */
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
    const static unsigned char kSECONDS = 0x03;

public:
    /** Convert an integer representation of a number into it's Binary Coded 
     * Digit equivalent.
     * 
     * @warning Converts at most 2 binary coded digits (up to a value of 99).
     * @warning Undefined operation with negative numbers
     * @warning Should not be used publicly: made public for use with testing.
     * 
     * @param number the number to convert.
     * @returns the 8 bits (2 digits) of binary coded number.
     */
    char ConvertToBCD(int number);

    /** Convert the BCD representation of a number into it's integer equivalent.
     * 
     * @warning Converts at most 2 binary coded digits (up to a value of 99).
     * @warning Undefined operation with negative numbers
     * @warning Should not be used publicly: made public for use with testing.
     * 
     * @param bcd The one or two digit number to convert.
     * @return    The converted number, in binary.
     */
    int ConvertFromBCD(unsigned char bcd);
};


#endif // SRLM_PROPGCC_PCF8523_H_
