#ifndef SRLM_PROPGCC_LSM303DLHC_H_
#define SRLM_PROPGCC_LSM303DLHC_H_

#ifndef UNIT_TEST
#include "i2c.h"
#else
#include "i2cMOCK.h"
#endif

/** Provides an interface to the LSM303DLHC accelerometer and magnetometer.


"Output Data Rate, in digital-output accelerometers, defines the rate at which
data is sampled. Bandwidth is the highest frequency signal that can be sampled
without aliasing by the specified Output Data Rate. Per the Nyquist sampling
criterion, bandwidth is half the Output Data Rate." -Analog Devices


@author SRLM (srlm@srlmproductions.com)
@date   2013-01-01
@version 1.1

Version History
	+ 1.1 - Updated to use boolean return values, and the full 16 bits for the
	        acceleration and magnetometer values.
	+ 1.0 - Initial release

*/
class LSM303DLHC{

public:
/**Tests to make sure that the LSM303DLHC is actually on the bus, and returns
false if it is not. Otherwise, sets the registers as follows and returns true.

Set the control registers of the accelerometer:

- CTRL_REG1_A:
 + 1.344kHz output rate
 + Normal power
 + XYZ enabled

- CTRL_REG4_A:
 + Block data continuous update (default)
 + data LSB @ lower address (default)
 + Full scale +-16G
 + High resolution output enable (?TODO(SRLM): what does this mean? datasheet is no help)
 + 00 (no functionality)
 + SPI interface mode (default, not used)
     
Set the control registers of the Magn :

   - CRA_REG_M:
     + Temperature sensor enable true
     + 00 (no functionality)
     + 220Hz Output data rate
   - CRB_REG_M:
     + 8.1+- guass
     + 00000 (no functionality)
   - MR_REG_M:
     + 000000 (no functionality)
     + Continuous conversion mode (TODO: What does this actually mean? Datasheet is no help)
     
@param  i2cbus The bus that the LSM303DLHC is on.
@return        true when both devices are successfully initialized.
  */
	bool Init(i2c * i2cbus);
	
/** Reads the accelerometer at the current settings, and updates the three
    reference values.

If there is an error then x, y, and z will be set to zero and false will be
returned.

@param x The acceleration x axis value. Will be overwritten.
@param y The acceleration y axis value. Will be overwritten.
@param z The acceleration z axis value. Will be overwritten.
@return true if all is successful, false otherwise. If false, try reinitilizing
*/
	bool ReadAccl(int& x, int& y, int& z);
	
/**Reads the magnetometer at the current settings, and updates the three
   reference values.

If there is an error then x, y, and z will be set to zero and false will be
returned.

@param x The magnetometer x axis value. Will be overwritten.
@param y The magnetometer y axis value. Will be overwritten.
@param z The magnetometer z axis value. Will be overwritten.
@return true if all is successful, false otherwise. If false, try reinitilizing
*/
	bool ReadMagn(int& x, int& y, int& z);

private:
	i2c * bus;
	
	bool status;

	const static unsigned char deviceAccl = 0b00110010;
	const static unsigned char deviceMagn = 0b00111100;
	
//For the LSM303DLH Accl
	const static unsigned char kCTRL_REG1_A = 0x20;
	const static unsigned char kCTRL_REG4_A = 0x23;
	const static unsigned char kOUT_X_L_A   = 0x28 | 0x80; //(turn on auto increment)

//For the LSM303DLH Magn
	const static unsigned char kCRA_REG_M = 0x00;
	const static unsigned char kCRB_REG_M = 0x01;
	const static unsigned char kMR_REG_M  = 0x02;
	const static unsigned char kOUT_X_H_M = 0x03 | 0x80; //(turn on auto increment)
};

#endif // SRLM_PROPGCC_LSM303DLHC_H_
