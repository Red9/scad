#include "gpsparser.h"

GPSParser::GPSParser(int Rx_pin, int Tx_pin, int Rate)
{
//TODO(SRLM): Should we call gps.Stop() ?
    head = 0;
    gps.Start(Rx_pin, Tx_pin, Rate);
}

GPSParser::~GPSParser(){
	gps.Stop();
}

void GPSParser::Put(char character){
	gps.Put(character);
}

char * GPSParser::Get(){
	return Get(buffer);
}

//TODO(SRLM): What about <LF> at the end of the sentence?
//TODO(SRLM): It should not start recording a string until it finds a start character
//TODO(SRLM): It should check to make sure that a string that is too long is ignored.
char * GPSParser::Get(char s[], const int maxBytes)
{
    for(;;)
    {
        int byte = gps.Get(0);//GetCCheck();
        if(byte == -1) return NULL;

        //Have a valid byte, now need to add to buffer
        s[head++] = byte;

        //Check if it's the end of a string.
        //or if it's too big.
        if (byte == '\n' || (int)head-(int)s+1 >= maxBytes)
        {
            s[head] = 0; //Null terminator
            head = 0;         //Reset head
            return s;   //Return pointer
        }
    }
}
