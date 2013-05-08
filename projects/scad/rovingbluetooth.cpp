#include "rovingbluetooth.h"


Bluetooth::Bluetooth(int rxpin, int txpin, int ctspin, int connectpin)
	:connection(connectpin){
	connection.input();
	//baud = 460800;
        baud = 115200;
	
	bool result = serial.Start(rxpin, txpin, baud, ctspin);
	if(result == false){
		//TODO(SRLM): What to do here? ignore for now...	
	}
	
//	connection = Pin(connectpin);

}

void Bluetooth::Put(const char character){
	if(connection.get()){
		serial.Put(character);
	}

}

int Bluetooth::Put(const char * buffer_pointer, const int count){
	if(connection.get()){
		serial.Put(buffer_pointer, count);
	}

}

int Bluetooth::Put(const char * buffer_pointer){
	if(connection.get()){
		serial.PutFormatted(buffer_pointer);
	}

}

int Bluetooth::Get(int timeout){
    serial.Get(timeout);
}

