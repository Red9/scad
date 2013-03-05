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
char * GPSParser::Get(char s[])
{
    for(;;)
    {
        int byte = gps.Get(0);//GetCCheck();
        if(byte == -1) return NULL;

        //Have a valid byte, now need to add to buffer
        s[head++] = byte;

        //Check if it's the end of a string.
        if (byte == '\n')
        {
            s[head] = 0; //Null terminator
            head = 0;         //Reset head
            return s;   //Return pointer
        }
    }
}
