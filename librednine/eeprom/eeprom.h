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


    void Start(int scl = 28, int sda = 29) {
        base.Initialize(scl, sda);
    }

    /** Put a byte into the EEPROM.
    @param  address the two byte address to write to.
    @param  byte    the single byte of data to write
    @return         success or failure
     */

    bool Put(unsigned short address, char byte) {
        char bytes[] = {byte};
        return Put(address, bytes, 1);
    }

    /** Write up to 4 bytes to a memory location. Stores them in little endian
    order.

    This function is useful for storing an int (long, 4 bytes) of data to the
    EEPROM. It can also be used to store a short (word, 2 bytes).

    @param  address the two byte address to write to.
    @param  bytes   the set of bytes to store. If length is less than 4, @a Put
                    stores the bytes beginning with the LSB.
    @param  length  the number of bytes to store, up to 4.
    @return         success or failure
     */
    bool Put(unsigned short address, int bytes, int length) {

        char temp[4];
        //Even if length is < 4, do them all (easier than a loop). Only the used ones will be written.
        temp[3] = (((unsigned) bytes) & 0xFF000000) >> 24;
        temp[2] = (((unsigned) bytes) & 0xFF0000) >> 16;
        temp[1] = (((unsigned) bytes) & 0xFF00) >> 8;
        temp[0] = (((unsigned) bytes) & 0xFF) >> 0;
        return Put(address, temp, length);
    }

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
    bool Put(unsigned short address, char bytes [], int size) {
        //The lower seven bits define an EEPROM page, so we need a bit of magic to make
        //sure that if we go over the boundry, we start a new page.

        int bytesWritten = 0;
        while (bytesWritten < size) {
            if (PollForAcknowledge() == false) {
                return false;
            }
            base.SendByte((address >> 8) & 0xFF); //High address byte
            base.SendByte(address & 0xFF); //Low address byte
            do {
                base.SendByte(bytes[bytesWritten]); //Payload	
                bytesWritten++;
                address++;
            } while ((address & 0b1111111) != 0 && bytesWritten < size); //detect rollover

            base.Stop();
        }

        return true;
    }

    /** Get a single byte from the EEPROM.

    @param  address the two byte address to read from.
    @return the byte read on success, or -1 on timeout or other failure.
     */
    int Get(unsigned short address) {
        char byte[1];
        int result = Get(address, byte, 1);
        if (result < 0) {
            return result;
        } else {
            return byte[0];
        }
    }

    /** Get a block of bytes from the EEPROM.

    Note: This funtion takes care of the page reads from the EEPROM.

    @param address the two byte address to read from.
    @param bytes   the memory location to store the data.
    @param length  the number of bytes to read.
    @return 0 on success, or -1 on timeout or other failure.
     */
    int Get(unsigned short address, char bytes [], int length) {
        int bytesRead = 0;
        while (bytesRead < length) {
            if (PollForAcknowledge() == false) {
                return -1;
            }
            base.SendByte((address >> 8) & 0xFF); //High address byte
            base.SendByte(address & 0xFF); //Low address byte
            base.Start();
            base.SendByte(device | 0x01); //device EEPROM read (w/ read bit set)


            while (((address + 1) & 0b1111111) != 0
                    && bytesRead + 1 < length) {
                bytes[bytesRead] = base.ReadByte(true);
                bytesRead++;
                address++;
            }

            bytes[bytesRead] = base.ReadByte(false);
            bytesRead++;
            address++;

            base.Stop();
        }

        return 0;

    }

    /** Get up to 4 bytes from the EEPROM and concatenate them into a single int.

    This function is useful for retrieving an integer or a short stored in EEPROM. 
    Bytes must be stored in little endian order in the EEPROM.

    @param address the two byte address to read from.
    @param length  the number of bytes to read.
    @returns       the number read from the EEPROM. If length is < 4, then the
                   upper bytes are set to 0.
     */
    int Get(unsigned short address, int length) {
        char temp[4];
        Get(address, temp, length);
        int result = 0;
        
        
        
        /*
        // Commented out because of the compiler loop bug.
        for (int i = length - 1; i >= 0; --i) {
            //debug->Put(" Get_C ");
            result = (result << 8) | temp[i];
        }*/
        
        
        if(length >= 4){
            result = (result << 8) | temp[3];
        }
        if(length >= 3){
            result = (result << 8) | temp[2];
            
        }
        if(length >= 2){
            result = (result << 8) | temp[1];
        }
        if(length >= 1){
            result = (result << 8) | temp[0];
        }
        
        return result;
    }

private:

    bool PollForAcknowledge() {
        base.Start();
        int counter = 0;
        while (base.SendByte(device) == false) { //device EEPROM write
            if (++counter == 100) { //timeout
                return false;
            }
            base.Stop();
            base.Start();
        }
        return true;
    }

    i2cBase base;
    static const unsigned char device = 0b10100000;

};

#endif // SRLM_PROPGCC_EEPROM_H_
