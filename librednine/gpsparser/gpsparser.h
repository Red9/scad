/** Parses NMEA0183 GPS streams into C strings.

@warning The PGTOP sentences can't have any "$" characters in them. The parsing
 * would treat that as a new NMEA string. While technically not correct, it's a
 * bit easier to implement.

@author Cody Lewis (srlm@srlmproductions.com)
 */

#ifndef SRLM_PROPGCC_GPSPARSER_H__
#define SRLM_PROPGCC_GPSPARSER_H__

#include "serial.h"

class GPSParser {
public:

    /** Create the parser and launch a new cog.

    Requires 1 cog to operate.

    @param rxPin the serial pin to receive data from the GPS.
    @param txPin the pin to transmit data to the GPS. If not used, set to -1.
    @param baud  the baud rate to use for tranmission and receiving.

     */
    GPSParser(int rxPin, int txPin, int baud) {
        gps.Stop();
        nextCharacterPosition = 0;
        gps.Start(rxPin, txPin, baud);

        recordingSentence = false;
    }

    /** Stop the GPS parsing, and the cog that was started.
     */
    ~GPSParser() {
        gps.Stop();
    }

    /** Gets a NMEA string. Note that the returned string address is valid (will 
     * not be overwritten) until the next time GetStr() is called.
     * 
     * The returned string includes all characters from the GPS except for the 
     * \r and \n (<CR><LF>) at the end.
     * 
     * Partial sentences may be thrown away (if a sentence is not being 
     * currently recorded).
     * 
     * Ignores the PGTOP type sentence.
     * 
     * @returns NULL pointer if no string, null terminated string otherwise
     */
    char * Get() {
        return Get(internalBuffer);
    }

    /** Same as @a Get(), but with the option of specifying a buffer to use 
     * instead of the internal buffer.

    
     * @warning If you want to switch between buffers, you must not switch 
     * until immediately  after gps.Get(s) != NULL (ie, right after it returns a 
     * string). Otherwise, part of the string will be stored in one buffer, and 
     * part of the string will be stored in the other.

    
     * @param s The buffer to use. Must be at least 85 characters long (the 
     * NMEA string length).
     * @param maxBytes The maximum number of bytes to record in this string. 
     * Defaults to maximum NMEA sentence length.
     * @returns NULL pointer if no string, null terminated string otherwise (in
     * buffer @a s).
     */
    char * Get(char s[], const int maxBytes = kNmeaMaxLength) {
        for (;;) {

            int byte = gps.Get(0);
            if (byte == -1) return NULL;

            if (nextCharacterPosition == 6) {
                CheckForPGTOP(s);
            }


            if (recordingSentence == false && byte != kSentenceStartCharacter) {
                /* Do nothing */
            } else if (byte == '\r' || byte == '\n') {
                return TerminateString(s);
            } else {
                //Have a valid byte, now need to add to buffer
                recordingSentence = true;
                s[nextCharacterPosition++] = byte;
            }

            if (nextCharacterPosition >= maxBytes) {
                return TerminateString(s);
            }
        }
    }

    /** Get the underlying serial object.
     * @warning This function is for testing only!
     * @returns A pointer to the underlying serial object.
     */
    Serial * getSerial(void) {
        return &gps;
    }

protected:
    Serial gps;

private:
    static const int kNmeaMaxLength = 85;
    static const int kBufferSize = kNmeaMaxLength;
    const static char kSentenceStartCharacter = '$';

    int nextCharacterPosition;
    char internalBuffer[kBufferSize]; //Holds 1 NMEA string
    bool recordingSentence;

    char * TerminateString(char s[]) {
        s[nextCharacterPosition] = 0; //Null terminator
        nextCharacterPosition = 0; //Reset nextCharacterPosition
        recordingSentence = false;
        return s; //Return pointer
    }

    void CheckForPGTOP(char s[]) {
        if (s[1] == 'P' &&
                s[2] == 'G' &&
                s[3] == 'T' &&
                s[4] == 'O' &&
                s[5] == 'P') {
            nextCharacterPosition = 0;
            recordingSentence = false;
        }
    }
};



#endif // SRLM_PROPGCC_GPSPARSER_H__
