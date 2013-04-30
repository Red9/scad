#include "eeprom.h"
#include "serial.h"
#include "numbers.h"
#include "c++-alloc.h"

#include "i2c.h"
#include "pcf8523.h"

#include <propeller.h>
/*
Beta 2 Pins
*/

const int kPIN_I2C_SCL = 0;
const int kPIN_I2C_SDA = 1;

const int kPIN_LSM303DLHC_INT1 = 2;
const int kPIN_LSM303DLHC_INT2 = 3;
const int kPIN_LSM303DLHC_DRDY = 4;

const int kPIN_LEDR           = 5;
const int kPIN_MAX8819_EN123  = 6;
const int kPIN_LEDG           = 7;
const int kPIN_BUTTON         = 8;

const int kPIN_SD_CD  = 9;
const int kPIN_SD_DO  = 10;
const int kPIN_SD_CLK = 11;
const int kPIN_SD_DI  = 12;
const int kPIN_SD_CS  = 13;

const int kPIN_MAX8819_CHG = 14;
const int kPIN_MAX8819_CEN = 15;
const int kPIN_MAX8819_DLIM1 = 16;
const int kPIN_MAX8819_DLIM2 = 17;


const int kPIN_USER_1 = 18;
const int kPIN_USER_2 = 19;
const int kPIN_USER_3 = 20;
const int kPIN_USER_4 = 21;
const int kPIN_USER_5 = 22;
const int kPIN_USER_6 = 23;


const int kPIN_GPS_TX = 24; //Tx from the Propeller
const int kPIN_GPS_RX = 25; //Rx to the Propeller
const int kPIN_GPS_FIX = 26;

const int kPIN_PCF8523_SQW = 27;

const int kPIN_EEPROM_SCL = 28;
const int kPIN_EEPROM_SDA = 29;

const int kPIN_USB_TX = 30; //Tx from the Propeller
const int kPIN_USB_RX = 31; //Rx to the Propeller

const int kUSB_BAUD = 115200;
//const int kUSB_BAUD = 460800;



const int kPIN_BLUETOOTH_TX = kPIN_USER_1;
const int kPIN_BLUETOOTH_RX = kPIN_USER_2;


Serial * debug;


const unsigned short kEepromUnitAddress = 0xFFFC;
const unsigned short kEepromBoardAddress = 0xFFF8;
const unsigned short kEepromCanonNumberAddress = 0xFFF4;

const int kBoardAlpha = 0x0000000A;
const int kBoardBeta  = 0x0000000B;
const int kBoardBeta2 = 0x000000B2;
const int kBoardGamma = 0x00000004;


int GetBoardVersion(int oldVersion){
	
	char charBuffer[12];
	debug->PutFormatted("Please select the board version: \r\n");
	debug->PutFormatted(" * - 0x%08x (current version)\r\n", oldVersion);
	debug->PutFormatted(" a - Alpha\r\n");
	debug->PutFormatted(" b - Beta\r\n");
	debug->PutFormatted(" c - Beta 2\r\n");
	debug->PutFormatted(" d - Gamma\r\n");
	debug->PutFormatted(" e - Other\r\n");
	debug->PutFormatted(">>> ");
	debug->GetFlush();
	char choice = debug->Get();	
	debug->PutFormatted("%c\r\n", choice);
	int boardVersion;
	
	if(choice ==  'a'){ boardVersion = kBoardAlpha;}
	else if(choice == 'b'){ boardVersion = kBoardBeta;}
	else if(choice == 'c'){ boardVersion = kBoardBeta2;}
	else if(choice == 'd'){ boardVersion = kBoardGamma;}
	else if(choice == '*'){ boardVersion = oldVersion;}
	else{ 
		debug->PutFormatted("Your choice '0x%08x' isn't supported right now. Keeping current version.\r\n", choice);
		boardVersion = oldVersion;
	}
	
	return boardVersion;
}

int GetUnitNumber(int oldNumber){
	
	debug->PutFormatted("Please enter the unit number or * to keep current number, then <enter>:\r\n>>> ");
	debug->GetFlush();
	
	char buffer[13];
	for(int i = 0;;++i){
		buffer[i] = debug->Get();
		debug->PutFormatted(buffer[i]); //echo
		if(buffer[i] == '\r'){
			buffer[i] = 0;
			debug->PutFormatted('\n');
			break;
		}
	}
	int unitNumber;
	if(buffer[0] == '*'){
		unitNumber = oldNumber;
	}else{
	 	unitNumber = Numbers::Dec(buffer);
	}

	return unitNumber;
		

}

int GetCanonNumber(int oldNumber){
	debug->PutFormatted("Please enter the canon number, or * to keep current number, then <enter>:\r\n>>> ");
	debug->GetFlush();
	
	char buffer[13];
	for(int i = 0;;++i){
		buffer[i] = debug->Get();
		debug->PutFormatted(buffer[i]); //echo
		if(buffer[i] == '\r'){
			buffer[i] = 0;
			debug->PutFormatted('\n');
			break;
		}
	}
	int canonNumber;
	if(buffer[0] == '*'){
		canonNumber = oldNumber;
	}else{
	 	canonNumber = Numbers::Dec(buffer);
	}

	return canonNumber;
}


int GetTwoDigits(const char * caption){
	
	char num[3];
	debug->PutFormatted(caption);
	num[0] = debug->Get();
	debug->PutFormatted(num[0]);
	num[1] = debug->Get();
	debug->PutFormatted(num[1]);
	num[2] = 0;
	return Numbers::Dec(num);
}

void SetTime(PCF8523 * rtc){
	debug->PutFormatted("Set time?\r\n(y)es\r\n*no\r\n>>> ");
	debug->GetFlush();
	char choice = debug->Get();
	debug->PutFormatted("%c\r\n", choice);
	
	if(choice != 'y'){
		return;
	}
	
	
	
	int year, month, day, hour, minute, second;
	
	debug->PutFormatted("Type all numbers as 2 digits (pad with zeros if needed).\r\n");
	
	year   = GetTwoDigits("\r\nYear: 20");
	month  = GetTwoDigits("\r\nMonth: ");
	day    = GetTwoDigits("\r\nDay: ");
	hour   = GetTwoDigits("\r\nHour: ");
	minute = GetTwoDigits("\r\nMinute: ");
	second = GetTwoDigits("\r\nSecond: ");  

	debug->PutFormatted("\r\nSelected 20%d-%d-%d at %d:%d:%d\r\n", year, month, day, hour, minute, second);

	rtc->SetClock(year, month, day, hour, minute, second);

}


int main(void){
	
	debug = new Serial;
	debug->Start(kPIN_USB_RX, kPIN_USB_TX, kUSB_BAUD);
	
	
//	Serial bluetooth = new Serial;
//	bluetooth.Start(kPIN_BLUETOOTH_RX, kPIN_BLUETOOTH_TX, kBLUETOOTH_BAUD);
//	debug->PutFormatted("SU,46");
//	debug->SetBaud(460800);
	
		
	debug->PutFormatted("SCAD Configuration Utility!\r\n");
	
	Eeprom eeprom;
	
	char temp[4];
	
	int boardVersion = 0;
	int unitNumber = 0;
	int canonNumber = 0;
	
	i2c * rtcBus = new i2c;
	rtcBus->Initialize(kPIN_I2C_SCL, kPIN_I2C_SDA);
	PCF8523 * rtc = new PCF8523(rtcBus, kPIN_PCF8523_SQW);
	
	for(;;){
		

		unitNumber = eeprom.Get(kEepromUnitAddress, 4);
		boardVersion = eeprom.Get(kEepromBoardAddress, 4);
		canonNumber = eeprom.Get(kEepromCanonNumberAddress, 4);
	
		int year, month, day, hour, minute, second;
		rtc->GetClock(year, month, day, hour, minute, second);
	
	
		debug->PutFormatted("\r\n--------------------------\r\n\r\n");
		debug->PutFormatted("Current unit number:   %d\r\n", unitNumber);		
		debug->PutFormatted("Current board version: 0x%x\r\n", boardVersion);
		debug->PutFormatted("Current Canon file:    %d\r\n", canonNumber);
		debug->PutFormatted("Current RTC clock is: 20%d-%d-%d at %d:%d:%d\r\n", year, month, day, hour, minute, second);
		debug->PutFormatted("\r\n--------------------------\r\n\r\n");
	
		
		boardVersion = GetBoardVersion(boardVersion);
		eeprom.Put(kEepromBoardAddress, boardVersion, 4);
		
		unitNumber = GetUnitNumber(unitNumber);
		eeprom.Put(kEepromUnitAddress, unitNumber, 4);
		
		canonNumber = GetCanonNumber(canonNumber);
		eeprom.Put(kEepromCanonNumberAddress, canonNumber, 4);
		
		SetTime(rtc);
	}
	
	
	delete rtc;
	delete rtcBus;
	delete debug;
	
	
	
	

}

































