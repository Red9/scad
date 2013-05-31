#include "rovingbluetooth.h"

//TODO(SRLM): Remove the second bluetooth!!

Bluetooth::Bluetooth(int rxpin, int txpin, int ctspin, int connectpin)
	:connection(connectpin){
	connection.input();
	baud = 460800;
        //baud = 230400;
        //baud = 115200;
	
	//bool result = serial.Start(rxpin, txpin, baud, ctspin);
        
        
        serial.Start(rxpin, -1, baud);
        serialTx.Start(-1, txpin, baud, ctspin);
        
	//if(result == false){
		//TODO(SRLM): What to do here? ignore for now...	
	//}
	
//	connection = Pin(connectpin);

}

int Bluetooth::Put(const char character){
	if(connection.get()){
		serialTx.Put(character);
                return 1;
	}

        return 0;
}

int Bluetooth::Put(const char * buffer_pointer, const int count){
	if(connection.get()){
		return serialTx.Put(buffer_pointer, count);
	}
        return 0;
}

int Bluetooth::Put(const char * buffer_pointer){
	if(connection.get()){
		return serialTx.PutFormatted(buffer_pointer);
	}
        return 0;
}

int Bluetooth::Get(int timeout){
    return serial.Get(timeout);
}


int Bluetooth::GetCount(void){
    return serial.GetCount();
}
