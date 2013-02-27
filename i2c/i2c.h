

#ifndef i2c_Class_Defined__
#define i2c_Class_Defined__

#include <stdint.h>

#include "i2cbase.h"



/**



- All "device" fields should be the 7 bit address of the device, with the low bit
    set to 0 (the 7 addres bits are the upper bits). This applies to both the
    Put (write) and Get (read) cases.

- Put and Get are based on I2C communication specification as described by ST in
  the LSM303DLHC and L3GD20 datasheets.
  
- Terms:

  + ST - Start
  + SAD - Slave Address (device)
  + SAK - Slave Acknowledge
  + SUB - SubAddress (slave register address)
  + SP - Stop
  + +W - plus write (lowest device bit set to 0)
  + +R - plus read (lowest device bit set to 1)
  + NMAK - Master No Acknowledge  
 
- I2C differs based on the device that you use. For functions that might be
  device specific, there is a reference indicator. These references are:
  
  + ST - ST Microelectronics, particularily the LSM303DLHC and L3GD20 devices.
  + MS - Measurement Specialties, particularily the MS5607 and MS5611 devices.
 
- If you're using the multibyte Get and Put with ST based devices, be sure to
  bitwise OR the register address with 0x80 (the MSb to 1) in order to turn on
  the auto-increment function (see datasheet for L3GD20 for example). This is
  not done automatically by this library.

@author SRLM
@date   2013-01-21
*/

class i2c {
public:

i2cBase base;

static const int kAck = 1;
static const int kNak = 0;



/** Setup the DIRA, OUTA, and INA registers for scl and sda.

@param scl The I2C SCL pin. Defaults to the Propeller default SCL pin.
@param sda The I2C SDA pin. Defaults to the Propeller default SDA pin.
@todo(SRLM): Move this function into the class constructor.
@return Nothing important.
*/
int Initialize(int scl = 28, int sda = 29);


/** Test for the Acknowledge of a device by sending start and the slave address.


Useful for polling the bus and seeing what devices are available. Ping uses the
following format:

    +--------+----+-------+-----+----+
    | Master | ST | SAD+W |     | SP |
    | Slave  |    |       | SAK |    |
    +--------+----+-------+-----+----+


@todo(SRLM): make it return a bool instead of int.
@return kAck or kNak.
*/
int Ping(unsigned char device);





/** Put a single byte with the following format:

    +--------+----+-------+-----+-----+-----+------+-----+----+
    | Master | ST | SAD+W |     | SUB |     | BYTE |     | SP |
    | Slave  |    |       | SAK |     | SAK |      | SAK |    |
    +--------+----+-------+-----+-----+-----+------+-----+----+
Reference: ST

@param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
@param address the 8 bit slave register address
@param byte    the 8 bits of data to store at @a address.
@todo(SRLM):   Change the return to bool (and make it meaningful!).
@return       TODO(SRLM): document return.
*/
unsigned char Put(unsigned char device, unsigned char address, char byte);

/**
Get a single byte with the following format: 

    +--------+----+-------+-----+-----+-----+----+-------+-----+------+------+----+
    | Master | ST | SAD+W |     | SUB |     | ST | SAD+R |     |      | NMAK | SP |
    | Slave  |    |       | SAK |     | SAK |    |       | SAK | DATA |      |    |
    +--------+----+-------+-----+-----+-----+----+-------+-----+------+------+----+

Reference: ST

@param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
@param address the 8 bit slave register address
@return       the 8 bits of data read from register @a address.
*/
unsigned char Get(unsigned char device, unsigned char address);






/**
Put multiple bytes with the following format:

                                            |Repeat for # of bytes    |
    +--------+----+-------+-----+-----+-----+------+-----+------+-----+----+
    | Master | ST | SAD+W |     | SUB |     | DATA |     | DATA |     | SP |
    | Slave  |    |       | SAK |     | SAK |      | SAK |      | SAK |    |
    +--------+----+-------+-----+-----+-----+------+-----+------+-----+----+

Reference: ST

@param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
@param address the 8 bit slave register address
@param bytes   the set of bytes to store on @a device, starting at register @a
               address.
@param size    the number of bytes to write
@todo(SRLM):   Change the return to bool (and make it meaningful!).
@return       TODO(SRLM): document return.
*/
unsigned char Put(unsigned char device, unsigned char address, char * bytes, int size);




/**
Get multiple bytes with the following format:

                                                               |Repeat for # of bytes -1 | Last byte   |            
    +--------+----+-------+-----+-----+-----+----+-------+-----+------+-----+------+-----+------+------+----+
    | Master | ST | SAD+W |     | SUB |     | ST | SAD+R |     |      | MAK |      | MAK |      | NMAK | SP |
    | Slave  |    |       | SAK |     | SAK |    |       | SAK | DATA |     | DATA |     | DATA |      |    |
    +--------+----+-------+-----+-----+-----+----+-------+-----+------+-----+------+-----+------+------+----+

Reference: ST

@param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0).
@param address the 8 bit slave register address.
@param bytes   the address to begin storing the read bytes at.
@param size    the number of bytes to read.
@todo(SRLM):   Change the return to bool (and make it meaningful!).
@return       TODO(SRLM): document return.
*/
unsigned char Get(unsigned char device, unsigned char address, char * bytes, int size);



/**
Put a single byte, no register address, on the bus with the following format :

    +--------+----+-------+-----+------+-----+----+
    | Master | ST | SAD+W |     | DATA |     | SP |
    | Slave  |    |       | SAK |      | SAK |    |
    +--------+----+-------+-----+------+-----+----+

@warning Notice the lack of a specifed register!

Reference: MS

@param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
@param byte    the 8 bits of data to send to @a device
@todo(SRLM):   Change the return to bool (and make it meaningful!).
@return       TODO(SRLM): document return.
*/
unsigned char Put(unsigned char device, char byte);


/**
Get multiple bytes, no register address, on the bus with the following format:

                                |Repeat      |
    +--------+----+-------+-----+------+-----+------+------+----+
    | Master | ST | SAD+R |     |      | MAK |      | NMAK | SP |
    | Slave  |    |       | SAK | DATA |     | DATA |      |    |
    +--------+----+-------+-----+------+-----+------+------+----+

@warning Notice the lack of a specifed register!

Reference: MS

@param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
@param bytes   the address to begin storing the read bytes at.
@param size    the number of bytes to read.
@todo(SRLM):   Change the return to bool (and make it meaningful!).
@return       TODO(SRLM): document return.
*/

unsigned char Get(unsigned char device, char * bytes, int size);


/*
Pass through methods to base:
TODO(SRLM): Test pass through methods
*/

/** Passthrough to base. See i2cBase::Start() for more details. */
void Start(){ base.Start(); }

/** Passthrough to base. See i2cBase::Stop() for more details. */
void Stop() { base.Stop();  }

/** Passthrough to base. See i2cBase::SendByte(unsigned char) for more details. */
int SendByte(unsigned char byte)       { return base.SendByte(byte); }

/** Passthrough to base. See i2cBase::ReadByte(int) for more details. */
unsigned char ReadByte(int acknowledge){ return base.ReadByte(    acknowledge); }





};

#endif
