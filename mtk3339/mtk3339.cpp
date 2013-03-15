#include "mtk3339.h"

//#include <stdio.h>

MTK3339::MTK3339(int rxPin, int txPin, int ppsPin)
	: GPSParser(rxPin, txPin, 9600)
{
	gpsStatus = CheckBaud();
	if(gpsStatus == true){ //GPS running at 9600
		//Update to 115200
		gps.Put(kPMTK_SET_NMEA_BAUDRATE_115200);
		waitcnt(CLKFREQ/19 + CNT); //Make sure it's transmitted.
		gps.SetBaud(115200);
	}else{ //GPS may be runnning at 115200
		gps.SetBaud(115200);
		gpsStatus = CheckBaud();
		if(gpsStatus == false){
			return;
		}
	}
	gps.Put(kPMTK_API_SET_NMEA_OUTPUT);
	gps.Put(kPMTK_SET_NMEA_UPDATE_10HZ);
	
}

MTK3339::~MTK3339(){
}

bool MTK3339::CheckBaud(void){
	gps.GetFlush();
	waitcnt(CLKFREQ/10 + CNT);
	gps.GetFlush();
	for(int i = 0; i < 50; i++){
		unsigned char byte = (unsigned char)gps.Get();
		if(byte > (0x7F)){
			return false;
		}
	}
	return true;

}



bool MTK3339::GetStatus(void){
	return gpsStatus;
}


