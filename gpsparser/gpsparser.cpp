#include "gpsparser.h"

GPSParser::GPSParser(int Rx_pin, int Tx_pin, int Rate) {
    gps.Stop();
    nextCharacterPosition = 0;
    gps.Start(Rx_pin, Tx_pin, Rate);

    recordingSentence = false;
}

GPSParser::~GPSParser() {
    gps.Stop();
}


char * GPSParser::Get() {
    return Get(internalBuffer);
}

char * GPSParser::TerminateString(char s[]) {
    s[nextCharacterPosition] = 0; //Null terminator
    nextCharacterPosition = 0; //Reset nextCharacterPosition
    recordingSentence = false;
    return s; //Return pointer

}

void GPSParser::CheckForPGTOP(char s[]) {
    if (s[1] == 'P' &&
            s[2] == 'G' &&
            s[3] == 'T' &&
            s[4] == 'O' &&
            s[5] == 'P') {
        nextCharacterPosition = 0;
        recordingSentence = false;
    }
}

char * GPSParser::Get(char s[], const int maxBytes) {
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
