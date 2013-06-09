#ifndef SRLM_PROPGCC_EEPROM_H_
#define SRLM_PROPGCC_EEPROM_H_

#include "i2cbase.h"


/** EEPROM access class.

Valid EEPROM data addresses are in the range 0 to 0xFFFF for 64KB EEPROMS, and 0
to 0x7FFF for 32KB EEPROMs.

This driver was written to work with the AT24C512C EEPROM from Atmel.

@author SRLM
@date   2013-01-21
*/
class Eeprom {
public:

/** Start an @a Eeprom instance.

@warning Requires that the A0, A1, and A2 pins of the EEPROM to be tied to ground.

@todo(SRLM): Does this class interfere with other I2C drivers?
@param scl The I2C SCL pin. Defaults to the Propeller default SCL pin.
@param sda The I2C SDA pin. Defaults to the Propeller default SDA pin.
*/


void Start(int scl = 28, int sda = 29);

/** Put a byte into the EEPROM.
@param  address the two byte address to write to.
@param  byte    the single byte of data to write
@return         success or failure
*/

bool Put(unsigned short address, char byte);

/** Write up to 4 bytes to a memory location. Stores them in little endian
order.

This function is useful for storing an int (long, 4 bytes) of data to the
EEPROM. It can also be used to store a short (word, 2 bytes).

@param  address the two byte address to write to.
@param  bytes   the set of bytes to store. If length is less than 4, @a Put
                stores the bytes begining with the LSB.
@param  length  the number of bytes to store, up to 4.
@return         success or failure
*/
bool Put(unsigned short address, int bytes, int length);

/** Write a block of data to the EEPROM. There are no restrictions on page
alignment.

For optimal efficiency, the data should be aligned in 128 byte blocks
starting with an address whose lowest 7 bits are 0000000. The is a slight (one 
time) performance cost to not aligning the data this way.

@param  address the two byte address to write to.
@param  bytes[] the array of bytes to write.
@param  size    the number of bytes to write.
@return         success or failure
*/
bool Put(unsigned short address, char bytes [], int size);

/** Get a single byte from the EEPROM.

@param  address the two byte address to read from.
@return the byte read on success, or -1 on timeout or other failure.
*/
int Get(unsigned short address);

/** Get a block of bytes from the EEPROM.

Note: This funtion takes care of the page reads from the EEPROM.

@param address the two byte address to read from.
@param bytes   the memory location to store the data.
@param length  the number of bytes to read.
@return 0 on success, or -1 on timeout or other failure.
*/
int Get(unsigned short address, char bytes [], int length);

/** Get up to 4 bytes from the EEPROM and concatenate them into a single int.

This function is useful for retrieving an integer or a short stored in EEPROM. 
Bytes must be stored in little endian order in the EEPROM.

@param address the two byte address to read from.
@param length  the number of bytes to read.
@returns       the number read from the EEPROM. If length is < 4, then the
               upper bytes are set to 0.
*/
int Get(unsigned short address, int length);

private:


bool PollForAcknowledge();

i2cBase base;
static const unsigned char device = 0b10100000;

};

#endif // SRLM_PROPGCC_EEPROM_H_
