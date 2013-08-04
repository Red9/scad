#include "FdSerial.h"
#include <stdio.h>



int main(void){
	FdSerial_start(18,19, 0,230400);
//	pc.Start(31,30, 19200);
//	Serial pin; 
//	pin.Start(18, 19, 115200);
	
	while(true){
		//getchar();
		FdSerial_tx(getchar());
	}
	
	
	
	
	
	return 0;
}
