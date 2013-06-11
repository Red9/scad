#include <stdio.h>
#include <propeller.h>
#include "mtk3339.h"


const int kPIN_GPS_TX = 24; //Tx from the Propeller
const int kPIN_GPS_RX = 25; //Rx to the Propeller
const int kPIN_GPS_FIX = 26;

int main(void){
	printf("Hello, MTK World!");

	MTK3339 mtk(kPIN_GPS_RX, kPIN_GPS_TX, kPIN_GPS_FIX);
	
	
	if(mtk.GetStatus() == true){
		printf("GPS Configured.");
		while(true){	
			char * string = NULL;
			while((string = mtk.Get()) == NULL){ /*do nothing*/}
			printf(string);
		}	
	}else{
		printf("GPS Misconfigured.");
	}
	
}
