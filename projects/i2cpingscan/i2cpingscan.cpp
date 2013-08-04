#include <stdio.h>
#include <string.h>
#include "i2c.h"


//unsigned int SCLPin = 28;
//unsigned int SDAPin = 29;

//unsigned int SCLPin = 27;
//unsigned int SDAPin = 26;

unsigned int SCLPin = 4;
unsigned int SDAPin = 5;


bool IsSpecialAddress(unsigned char address){
//	Ignore special addresses: http://www.i2c-bus.org/addressing/
	unsigned char special [] = {
		0b0000000, 0b0000001, 0b0000010, 0b0000011, //Assorted addresses
		0b0000100, 0b0000101, 0b0000110, 0b0000111, //High speed master code
		0b1111000, 0b1111001, 0b1111010, 0b1111011, //10-bit slave addressing
		0b1111100, 0b1111101, 0b1111110, 0b1111111};//Reserved for future purposes
	
	int specialSize = 16;
	
	for(int i = 0; i < specialSize; ++i){
		if(address == special[i]){
			return true;
		}
	}
	return false;
}

bool IsKnownAddress(unsigned char address, char * buffer){
	if(address == 0b0011001){
		strcpy(buffer, "LSM303DLHC Acclerometer");
		return true;
	}
	if(address == 0b1101011){
		strcpy(buffer, "L3GD20 Gyroscope");
		return true;
	}
	if(address == 0b0011110){
		strcpy(buffer, "LSM303DLHC Magnetometer");
		return true;
	}
	if(address == 0b1110111){
		strcpy(buffer, "MS5611 Barometer");
		return true;
	}
	if(address == 0b0110110){
		strcpy(buffer, "MAX17048G Fuel Gauge");
		return true;
	}
	if(address == 0b1101000){
		strcpy(buffer, "PCF8523 Real Time Clock");
		return true;
	}
	if(address == 0b1010000){
		strcpy(buffer, "24LC512 EEPROM");
		return true;
	}
	

	return false;
}

int main(void){

	printf("\r\nI2C Ping Scan\r\n");
	printf("*** indicates special address.\r\n");
	printf("ADDR - status\r\n");
	printf("-------------\r\n");
	
	
	i2c bus;
	bus.Initialize(SCLPin, SDAPin);
	
	char buffer[80];
	
	for(unsigned char address = 0; address < 128; address++){
		if(IsSpecialAddress(address) == false){
			//Ping bus
			if(bus.Ping(address << 1)){
				if(IsKnownAddress(address, buffer)){
					printf("0x%X - Found ", address << 1);
					printf(buffer);
					printf("\r\n");
				}else{
					printf("0x%X - Found \r\n", address << 1);
				}
			}else{
				printf("0x%X - \r\n", address << 1);
			}
		}else{
			printf("0x%X - ***\r\n", address << 1);
		}
			

	}
	putchar(0xff);
	putchar(0x00);
	putchar(0);

}
