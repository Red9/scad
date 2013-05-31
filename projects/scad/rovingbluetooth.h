#ifndef SRLM_PROPGCC_ROVING_BLUETOOTH_H__
#define SRLM_PROPGCC_ROVING_BLUETOOTH_H__

#include "serial.h"
#include "pin.h"

class Bluetooth {

public:
	Bluetooth(int rxpin, int txpin, int ctspin, int connectpin);
	
	
	int Put(const char character);

	int Put(const char * buffer_ptr, const int count);
	int Put(const char * buffer_pointer);
	
	int Get(int timeout = -1);
	int Get(char * buffer, int length, int timeout=-1);
	int Get(char * buffer, char terminator='\n');
	void GetFlush(void);

        int GetCount(void);
        
private:
	int baud;
	Serial serial;
        Serial serialTx;
	Pin connection;
	
};

#endif //SRLM_PROPGCC_ROVING_BLUETOOTH_H__
