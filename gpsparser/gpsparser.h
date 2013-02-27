#ifndef GPSPARSER_H_INCLUDED
#define GPSPARSER_H_INCLUDED

#include "serial.h"

class GPSParser
{

public:
GPSParser(int Rx_pin, int Tx_pin, int Baud);
~GPSParser();


/** Gets a NMEA string. Note that the returned string address is
valid (will not be overwritten) until the next time GetStr()
is called.

The returned string includes all characters from the GPS, including the '\n' at
the end.


@returns NULL if no string, null terminated string otherwise
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
@returns NULL if no string, \n and null terminated string otherwise (in buffer
		@a s).
*/
char * Get(char s[]);


/** Passthrough to the base @a Serial::Put() function.
@param character the byte to transmit.
*/
void Put(char character);



private:
Serial gps;
int head;
char buffer[85]; //Holds 1 NMEA string


};



#endif // GPSPARSER_H_INCLUDED
