#include "serial.h"

#include "pin.h"

int main(void){
	Serial pc;
	pc.Start(31,30, 230400);
	Serial port; 
	port.Start(18, 19, 460800);
	
	Pin led(22);
	led.low();
	
	while(true){
		port.Put(pc.Get());
		if(pc.GetCount() > Serial::kBufferLength>>1){
			led.high();
		}else{
			led.low();
		}
	}
	
	
	
	
	
	return 0;
}
