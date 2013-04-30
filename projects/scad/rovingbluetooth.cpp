#include "rovingbluetooth.h"


Bluetooth::Bluetooth(int rxpin, int txpin, int ctspin, int connectpin)
	:connection(connectpin){
	connection.input();
	baud = 460800;
	
	bool result = serial.Start(rxpin, txpin, baud, ctspin);
	if(result == false){
		//TODO(SRLM): What to do here? ignore for now...	
	}
	
//	connection = Pin(connectpin);

}

void Bluetooth::Put(char character){
	if(connection.get()){
		serial.Put(character);
	}

}

int Bluetooth::Put(char * buffer_pointer, int count){
	if(connection.get()){
		serial.Put(buffer_pointer, count);
	}

}



