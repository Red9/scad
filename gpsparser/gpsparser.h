/** Parses NMEA0183 GPS streams into C strings.



@author Cody Lewis (srlm@srlmproductions.com)
@date   2013-01-01 
*/

#ifndef GPSPARSER_H_INCLUDED
#define GPSPARSER_H_INCLUDED

#include "serial.h"



class GPSParser
{

public:

/** Create the parser and launch a new cog.

Requires 1 cog to operate.

@param rxPin the serial pin to receive data from the GPS.
@param txPin the pin to transmit data to the GPS. If not used, set to -1.
@param baud  the baud rate to use for tranmission and receiving.

*/
GPSParser(int rxPin, int txPin, int baud);

/** Stop the GPS parsing, and the cog that was started.
*/
~GPSParser();


/** Gets a NMEA string. Note that the returned string address is
valid (will not be overwritten) until the next time GetStr()
is called.

The returned string includes all characters from the GPS, including the '\n' at
the end.

@returns NULL pointer if no string, null terminated string otherwise
*/
char * Get();

/** Same as @a Get(), but with the option of specifying a buffer to use instead
of the internal buffer.

@warning If you want to switch between buffers, you must not switch until immediately 
after gps.Get(s) != NULL (ie, right after it returns a string). Otherwise, part
of the string will be stored in one buffer, and part of the string will be
stored in the other.

@param s The buffer to use. Must be at least 85 characters long (the NMEA
         string length).
@param maxBytes The maximum number of bytes to record in this string. Defaults
         to maximum NMEA sentence length.
@returns NULL pointer if no string, \n and null terminated string otherwise (in buffer
		@a s).
*/
char * Get(char s[], const int maxBytes = kBufferSize);


/** Passthrough to the base @a Serial::Put(char) function.

@warning This function is for testing only! If you need to put something, you
         should subclass this class, and use the serial object directly.
@param character the byte to transmit.
*/
void Put(char character);

//TODO(SRLM): Add string Put pass through method as well.

/** Get the underlying serial object.

@warning This function is for testing only!
@returns A pointer to the underlying serial object.
*/
Serial * getSerial(void){
	return & gps;
}

protected:
Serial gps;

private:
static const int kBufferSize = 85;
int head;
char buffer[kBufferSize]; //Holds 1 NMEA string



};



#endif // GPSPARSER_H_INCLUDED
