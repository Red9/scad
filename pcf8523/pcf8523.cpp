#include "pcf8523.h"


PCF8523::PCF8523(i2c * newbus, int newkPIN_SQW){
	kPIN_SQW = newkPIN_SQW;
	bus = newbus;
	GetStatus();
	if(!status){// or bus == NULL){ //TODO(SRLM): is this right?
		return; //No device
	}
	
	//Software reset
//Warning: Don't do a software reset! It will(?) clear the clock registers.
//	bus->Put(deviceRtc, kCONTROL_1, 0b01011000);
//	waitcnt(CLKFREQ/500 + CNT); //Wait 2 ms, just in case it's needed.
	
	//Initialize the device
	bus->Put(deviceRtc, kCONTROL_1, 0b10000000);
	bus->Put(deviceRtc, kCONTROL_2, 0b00000000);
	bus->Put(deviceRtc, kCONTROL_3, 0b00000000);
	
}

bool PCF8523::GetStatus(){
	
	status = bus->Ping(deviceRtc);
	return status;
}

char PCF8523::ConvertToBCD(int number){
	int unit = number % 10;
	int tens = (number % 100) / 10;
	
	return (tens << 4) | unit;
}

int PCF8523::ConvertFromBCD(unsigned char bcd){
	return ((bcd >> 4) * 10) + (bcd &0xF);
}


bool PCF8523::GetClock(int & year, int & month, int & day,
                       int & hour, int & minute, int & second){
	int temp;
	return GetClock(year, month, day, hour, minute, second, temp);
}



bool PCF8523::GetClock(int & year, int & month, int & day,
                       int & hour, int & minute, int & second, int & weekday){
	if(!status){
		return false;
	}
	
	char clock[7];
	bus->Get(deviceRtc, kSECONDS, clock, 7);
	second  = ConvertFromBCD(clock[0]);
	minute  = ConvertFromBCD(clock[1]);
	hour    = ConvertFromBCD(clock[2]);
	day     = ConvertFromBCD(clock[3]);
	weekday = ConvertFromBCD(clock[4]);
	month   = ConvertFromBCD(clock[5]);
	year    = ConvertFromBCD(clock[6]);
	
	return true;
		
}
                    

bool PCF8523::SetClock(int year, int month, int day,
                       int hour, int minute, int second, int weekday){

	if(!status){
		return false; //TODO(SRLM): Should we reping the bus?
	}
	
	
//TODO(SRLM): Check OS bit in register seconds

	char clock[7];
	clock[0] = ConvertToBCD(second );
	clock[1] = ConvertToBCD(minute );
	clock[2] = ConvertToBCD(hour   );
	clock[3] = ConvertToBCD(day    );
	clock[4] = ConvertToBCD(weekday);
	clock[5] = ConvertToBCD(month  );
	clock[6] = ConvertToBCD(year   );
	
	clock[0] &= 0b01111111; //Clear OS bit
		
	bus->Put(deviceRtc, kSECONDS, clock, 7);
	
	return true;

}



