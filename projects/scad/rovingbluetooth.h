#ifndef SRLM_PROPGCC_ROVING_BLUETOOTH_H__
#define SRLM_PROPGCC_ROVING_BLUETOOTH_H__

#include "serial.h"
#include "pin.h"

class Bluetooth {

public:
	Bluetooth(int rxpin, int txpin, int ctspin, int connectpin);
	
	
	void Put(char character);

	int Put(char * buffer_ptr, int count);
	
	
	int Get(int timeout = -1);
	int Get(char * buffer, int length, int timeout=-1);
	int Get(char * buffer, char terminator='\n');
	void GetFlush(void);

private:
	int baud;
	Serial serial;
	Pin connection;
	
};

#endif //SRLM_PROPGCC_ROVING_BLUETOOTH_H__
