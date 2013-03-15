/**


Summary of supported sentences (note: order changed):
	* GLL - Lat, long, time, status
	* RMC - Lat, long, time, status, speed, true heading, date, magnetic variation
	* GGA - Lat, long, time, status (enhanced), #sats, horizontal precision, altitude, geoidal seperation, differentialGPS age, differentialGPS referenceID
	* VTG - true heading, magnetic heading, speed knots, speed km/hr	
	* GSA - Satelite IDs
	* GSV - Satelite SNR in dB data
*/



#ifndef SRLM_PROPGCC_MTK3339_H_
#define SRLM_PROPGCC_MTK3339_H_

#include "gpsparser.h"
#include <propeller.h>
class MTK3339 : public GPSParser{

public:

/**
Send configuration packets:

220 PMTK_SET_NMEA_UPDATERATE 100 (milliseconds, 10Hz)
251 PMTK_SET_NMEA_BAUDRATE   115200 (baud)
341 PMTK_API_SET_NMEA_OUTPUT
	GLL 0 (disabled)
	RMC 1 (10 Hz)
	VTG	0 (disabled)
	GGA 1 (10 Hz)
	GSA 5 (2 Hz)
	GSV 5 (2 Hz)
	

Requires that the previous baud be either 9600 (factory default) or 115200.
Other baud rates are not supported, and will result in @a GetStatus() == false.


*/

MTK3339(int rxPin, int txPin, int ppsPin);
bool GetStatus(void);

//TODO(SRLM): Add enter standby mode
// PMTK_CMD_STANDBY_MODE

~MTK3339();

private:

/** Tests the input stream for the correct baud.

Tests the input by checking to make sure that each character is a valid ASCII
char. It does this 50 times. There is a 1 in 1,125,899,900,000,000 (2^50) chance
that an invalid baud rate will make it through.

@warning Has a 100ms delay in the function, plus however long it takes to
         process the input stream (a ~41 ms for 9600).

@return true if the baud produces printable characters, false otherwise.
*/
bool CheckBaud(void);

bool gpsStatus;

static constexpr const char * kPMTK_SET_NMEA_UPDATE_10HZ     = "$PMTK220,100*2F\r\n";
static constexpr const char * kPMTK_SET_NMEA_BAUDRATE_115200 = "$PMTK251,115200*1F\r\n";
static constexpr const char * kPMTK_API_SET_NMEA_OUTPUT      = "$PMTK314,0,1,0,1,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n";




};

#endif // SRLM_PROPGCC_MTK3339_H_
