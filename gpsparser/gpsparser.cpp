#include "gpsparser.h"

GPSParser::GPSParser(int Rx_pin, int Tx_pin, int Rate) {
    //TODO(SRLM): Should we call gps.Stop() ?
    head = 0;
    gps.Start(Rx_pin, Tx_pin, Rate);

    recordingSentence = false;
}

GPSParser::~GPSParser() {
    gps.Stop();
}

void GPSParser::Put(char character) {
    gps.Put(character);
}

char * GPSParser::Get() {
    return Get(buffer);
}

//TODO(SRLM): It should not start recording a string until it finds a start character
//TODO(SRLM): It should check to make sure that a string that is too long is ignored.
//TODO(SRLM): Add test for discarding PGTOP sentences.
char * GPSParser::Get(char s[], const int maxBytes) {
    for (;;) {
        int byte = gps.Get(0); //GetCCheck();
        if (byte == -1) return NULL;

        //Check if it's the end of a string.
        //or if it's too big.
        if (recordingSentence == false && byte != sentenceStart) {
            /* Do nothing */
        } else if (byte == '\r' || byte == '\n' || (int) head - (int) s + 1 >= maxBytes) {
            s[head] = 0; //Null terminator
            head = 0; //Reset head
            recordingSentence = false;

            //Discard all $PGTOP strings:
            if (s[1] == 'P' &&
                    s[2] == 'G' &&
                    s[3] == 'T' &&
                    s[4] == 'O' &&
                    s[5] == 'P') {
                return NULL;
            } else {
                return s; //Return pointer
            }

        } else {
            //Have a valid byte, now need to add to buffer
            recordingSentence = true;
            s[head++] = byte;
        }
    }
}
