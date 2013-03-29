#include "max17048.h"


MAX17048::MAX17048(i2c * newbus){
	bus = newbus;
	GetStatus();
//	if(bus == NULL){
//		status = false;
//		return;
//	}
	if(!status){
		return;
	}

}

bool MAX17048::GetStatus(void){
	status = bus->Ping(deviceFuel);
	return status;
}

int MAX17048::GetShort(char address){
	char data[2];
	bus->Get(deviceFuel, address, data, 2);
	int result = (data[0] << 8) | data[1];
	return result;
}


int MAX17048::GetVersion(void){
//	char data[2];
//	bus->Get(deviceFuel, kVERSION, data, 2);
//	int result = (data[0] << 8) | data[1];
//	return result;
	return GetShort(kVERSION);
}

int MAX17048::GetVoltage(void){
//	char data[2];
//	bus->Get(deviceFuel, kVCELL, data, 2);
//	int voltage = (data[0] << 8) | data[1];

	int voltage = GetShort(kVCELL);
	
	//Originally units of 78.125uV/LSb,
	//Convert to 1mV/LSb
	voltage = ((voltage * 7812) / 100) / 1000; 
	
	return voltage;
}

int MAX17048::GetStateOfCharge(void){
	int soc = GetShort(kSOC);
	return ((unsigned)soc) >> 8;
}

int MAX17048::GetChargeRate(void){

	//I don't know if this is signed or not, but if it is then sign extend
	//And hopefully, it's not unsigned *and* 16 bits!
	int rate = (GetShort(kCRATE) << 16) >> 16;

	return (rate*208)/100;	
}































	

