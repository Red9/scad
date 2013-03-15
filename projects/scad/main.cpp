#include <string.h>
#include <propeller.h>

#include "serial.h"
#include "i2c.h"
#include "l3gd20.h"
#include "lsm303dlhc.h"
#include "c++-alloc.h"
#include "securedigitalcard.h"
#include "max8819.h"
#include "elum.h"
#include "numbers.h"
#include "scheduler.h"
#include "concurrentbuffer.h"
#include "eeprom.h"
#include "max17048.h"
#include "pcf8523.h"
//#include "gpsparser.h"
#include "mtk3339.h"

#include "pin.h"

#include "stdlib.h"

//TODO(SRLM): Add tests to shut off when battery voltage is too low.

//TODO(SRLM): Change ELUM to something more generic...

//TODO(SRLM): check pointers for null, and so on (be safe!).

//TODO(SRLM): Add date/time to file creation.

//TODO(SRLM): Check that R element is logged correctly.

/**

Cog Usage:
0: Main (inc. datalog controller)
1: GPS
2: I2C Getter
3: SD driver
4: Serial (USB/Bluetooth)
5:
6:
7:
*/

//#define EXTERNAL_IMU


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


/*
Redifinitions of Pins
*/
#ifdef EXTERNAL_IMU
const int kPIN_I2C_SCL_2 = kPIN_USER_5;
const int kPIN_I2C_SDA_2 = kPIN_USER_6;
#endif

const int kPIN_BLUETOOTH_TX = kPIN_USER_1;
const int kPIN_BLUETOOTH_RX = kPIN_USER_2;



/*
Other constants
*/
//const int kUSB_BAUD = 115200;
const int kUSB_BAUD = 460800;

const int kBLUETOOTH_BAUD = 115200;

const int kGPS_BAUD = 9600;


const unsigned short kEepromUnitAddress = 0xFFFC;
const unsigned short kEepromBoardAddress = 0xFFF8;

const int kBoardAlpha = 0x0000000A;
const int kBoardBeta  = 0x0000000B;
const int kBoardBeta2 = 0x000000B2;
const int kBoardGamma = 0x00000004;



Serial * serial;
Max8819 * pmic;
Elum * elum;
i2c * bus;
LSM303DLHC * lsm;
L3GD20 * l3g;
Eeprom * eeprom;
PCF8523 * rtc;
MAX17048 * fuel;
MTK3339 * gps;
SecureDigitalCard * sd;

#ifdef EXTERNAL_IMU
i2c * bus2;
LSM303DLHC * lsm2;
L3GD20 * l3g2;
#endif

Pin * led;


/*
Status Variables
*/
volatile bool datalogging = false;
volatile bool sdPresent = false;

volatile int SDBufferFree = 999999; //large number for before we start...

volatile int fuel_soc = 0;
volatile int fuel_rate = 0;
volatile int fuel_voltage = 0;

volatile int year, month, day, hour, minute, second;

volatile int gyro_x, gyro_y, gyro_z;
volatile int accl_x, accl_y, accl_z;
volatile int magn_x, magn_y, magn_z;

volatile int gyro2_x, gyro2_y, gyro2_z;
volatile int accl2_x, accl2_y, accl2_z;
volatile int magn2_x, magn2_y, magn2_z;

volatile int freeReadCycles = 0;
volatile int freeReadCyclesMax = 0;

volatile char currentFilename[] = "";


volatile int unitNumber;
volatile int boardVersion;

int stacksize = sizeof(_thread_state_t)+sizeof(int)*3 + sizeof(int)*100;

enum LogLevel {kAll, kFatal, kError, kWarn, kInfo, kDebug};
const char * LogLevelIdentifier[] = {"All:   ", "Fatal: ", "Error: ", "Warn:  ", "Info:  ", "Debug: "};


//TODO(SRLM): DisplayState is a poor choice of name... It should be something like "DeviceState"
enum DeviceState {kPowerUp, kUnknownError, kNoSD, kCharging, kDatalogging, kDone, kUnknown};
volatile DeviceState currentState = kUnknown;


Scheduler * displayDeviceState = NULL;

void DisplayDeviceStatus(DeviceState state){

	//Make sure that we're keeping track of the last time display has been updated.
	if(displayDeviceState == NULL){
		displayDeviceState = new Scheduler(1); //0.1 Hz
	}
	
	//Exit if the state is the same, and we've updated the display recently.
	if(currentState == state && !displayDeviceState->Run()){
		return;
	}
	
	currentState = state;	
	
	if(currentState == kUnknownError){
		//Alternate red and green triple
		elum->Pattern(Elum::kTriple);
	}else if(currentState == kNoSD){
		//Alternate red and green
		elum->Pattern(Elum::kSingle);
	}else if(currentState == kCharging){
		if(fuel_soc < 90){//pmic->GetCharge() == true){
			//Fade LED in and out
			elum->Fade(5);
		}else{
			//Done charging status
			elum->Fade(1000);
		}		
	}else if(currentState == kDatalogging){
		if(fuel_soc > 25){
			//Flash green LED
			elum->Flash(Elum::GREEN, 1000, fuel_soc * 10);
			
		}else{
			//Flash red LED
			//Plus one for the 0 case.
			elum->Flash(Elum::RED, 1000, (fuel_soc + 1) * 9);
		}
	}else if(currentState == kPowerUp){
		elum->On(Elum::GREEN);
	}else{
		//If nothing else, then turn off LED.
		elum->Off();
	}	
	
	
}

/**
Scans the SD card and searches for filenames of the type "red9log#.csv",
where # is any size number. Actually, it uses the FILENAME and FILEEXT const
section variables for the filename. It starts scanning at filenumber 0, and
continues until it doesn't find a file of that number on the SD card. It
then opens the file for writing, and then this function returns.

@param sd
@param identifier The unit number to store in the first part of the filename.

  */
void OpenFile(int identifier){
    char buffer[12];
//    char buff2[4];
    for(int i = 0; i < 1000; i++)
    {
        buffer[0] = 0;

//This version is B###F###.EXT
		strcat(buffer, "B");
		strcat(buffer, Numbers::Dec(identifier));
		strcat(buffer, "F");
		strcat(buffer, Numbers::Dec(i));
		strcat(buffer, ".RNB");
        int result = sd->Open(buffer, 'r');
        if(result == -1) break;
    }
    sd->Open(buffer, 'w');
    int i = 0;
    for(; buffer[i] != '\0'; i++){
    	currentFilename[i] = buffer[i];
    }
    currentFilename[i] = '\0';
}










/**
Six Byte binary version of @a PutIntoBuffer()
*/

void PutIntoBuffer(ConcurrentBuffer * buffer, char identifier, unsigned int cnt,
		unsigned int a, unsigned int b, unsigned int c){
	char data[13];
	data[0] = identifier;
	
//Little endian
	data[4] = (cnt & 0xFF000000) >> 24;
	data[3] = (cnt & 0xFF0000)   >> 16;
	data[2] = (cnt & 0xFF00)     >> 8;
	data[1] = (cnt & 0xFF)       >> 0;
	
// Two byte mode
	data[6] = (a & 0xFF00)     >> 8;
	data[5] = (a & 0xFF)       >> 0;
	
	data[8] = (b & 0xFF00)     >> 8;
	data[7] = (b & 0xFF)       >> 0;
	
	data[10]= (c & 0xFF00)     >> 8;
	data[ 9]= (c & 0xFF)       >> 0;

	while(buffer->Put(data, 11) == false){}			
}


/**
String version of @a PutIntoBuffer()

@param buffer     
@param identifier 
@param cnt        
@param string     
@param terminator All the characters in @a string are put into @a buffer until
                  the terminator character is found.

*/
void PutIntoBuffer(ConcurrentBuffer * buffer, char identifier, unsigned int cnt,
		const char * string, char terminator){
	char data[100];
	data[0] = identifier;
	
//Little endian
	data[4] = (cnt & 0xFF000000) >> 24;
	data[3] = (cnt & 0xFF0000)   >> 16;
	data[2] = (cnt & 0xFF00)     >> 8;
	data[1] = (cnt & 0xFF)       >> 0;
	
	int i;
	for(i = 0; string[i] != terminator; i++){
		data[5+i] = string[i];
	}
	data[5+i] = '\0';
	while(buffer->Put(data, 5+i+1) == false){}
	
}

void LogVElement(ConcurrentBuffer * buffer){
	char string [200];
	string[0] = '\0';
	
	strcat(string, __DATE__);
	strcat(string, " ");
	
	strcat(string, __TIME__);
	strcat(string, " ");
	
	strcat(string, Numbers::Dec(unitNumber));
	strcat(string, " ");
	
	strcat(string, Numbers::Hex(boardVersion, 8));
	strcat(string, " ");

	PutIntoBuffer(buffer, 'V', CNT, string, '\0');

}

void LogRElement(ConcurrentBuffer * buffer){
	char filename[13];
	for(int i = 0; currentFilename[i] != '\0'; i++){
		filename[i] = currentFilename[i];
	}
	PutIntoBuffer(buffer, 'R', CNT, filename, '\0');
}

void LogStatusElement(ConcurrentBuffer * buffer, const LogLevel level, const char * message){
	char completeMessage[70];
	completeMessage[0] = '\0';
	strcat(completeMessage, LogLevelIdentifier[level]);
	strcat(completeMessage, message);
	PutIntoBuffer(buffer, 'S', CNT, completeMessage, '\0');
}

void LogStatusElement(ConcurrentBuffer * buffer, const LogLevel level, const char * message, const int numberA ){
	char completeMessage[70];
	completeMessage[0] = '\0';
	strcat(completeMessage, LogLevelIdentifier[level]);
	strcat(completeMessage, message);
	strcat(completeMessage, Numbers::Dec(numberA));
	PutIntoBuffer(buffer, 'S', CNT, completeMessage, '\0');
}


void ReadDateTime(void){
	//Have to do these antics because year, month, etc. are volatile
	//and don't match the GetClock parameter list.
	int tyear, tmonth, tday, thour, tminute, tsecond;
	rtc->GetClock(tyear, tmonth, tday, thour, tminute, tsecond);
	year = tyear;
	month = tmonth;
	day = tday;
	hour = thour;
	minute = tminute;
	second = tsecond;
}

void ReadGyro(void){
	int x, y, z;
	l3g->ReadGyro(x, y, z);
	gyro_x = x;
	gyro_y = y;
	gyro_z = z;
}

void ReadAccl(void){
	int x,y,z;
	lsm->ReadAccl(x, y, z);
	accl_x = x;
	accl_y = y;
	accl_z = z;
}

void ReadMagn(void){
	int x, y, z;
	lsm->ReadMagn(x, y, z);
	magn_x = x;
	magn_y = y;
	magn_z = z;
}

void ReadFuel(){
	fuel_soc     = fuel->GetStateOfCharge();
	fuel_voltage = fuel->GetVoltage();
	fuel_rate    = fuel->GetChargeRate();
}

#ifdef EXTERNAL_IMU
void ReadGyro2(void){
	int x, y, z;
	l3g2->ReadGyro(x, y, z);
	gyro2_x = x;
	gyro2_y = y;
	gyro2_z = z;
}

void ReadAccl2(void){
	int x,y,z;
	lsm2->ReadAccl(x, y, z);
	accl2_x = x;
	accl2_y = y;
	accl2_z = z;
}

void ReadMagn2(void){
	int x, y, z;
	lsm2->ReadMagn(x, y, z);
	magn2_x = x;
	magn2_y = y;
	magn2_z = z;
}
#endif


	
void ReadSensors(){
	//Had to add volatile here: for some reason, the compiler was ignoring
	//gyroCNT and magnCNT, and logging 0 for them. Volatile seemed to fix it.
	volatile unsigned int acclCNT = 0;
	bool acclLog = false;
	volatile unsigned int gyroCNT = 0xDEADBEEF;
	bool gyroLog = false;
	volatile unsigned int magnCNT = 0xABCDEF01;
	bool magnLog = false;
	
	char * gpsString = NULL;
	//Flush GPS buffer.
	while( (gpsString = gps->Get()) != NULL) {/*Throw away stings*/}
	


	ConcurrentBuffer * buffer = new ConcurrentBuffer();
	
	LogRElement(buffer);
	LogVElement(buffer);

	Scheduler acclScheduler(150*10);
	Scheduler gyroScheduler(100*10);
	Scheduler magnScheduler(25*10);
	Scheduler fuelScheduler(1); //10 second cycle
	Scheduler timeScheduler(1); //10 second cycle

#ifdef EXTERNAL_IMU	
	Scheduler accl2Scheduler(150*10);
	Scheduler gyro2Scheduler(100*10);
	Scheduler magn2Scheduler(25*10);
#endif

	while(datalogging){
		
		//Note: each Put into the buffer must have a matching Get! Otherwise, on
		//occasion the get test will misinterpret a data byte as the start of a
		//packet.
		
		if(acclScheduler.Run()){
			freeReadCycles = 0;
			ReadAccl();
			acclCNT = CNT;
			acclLog = true;
		}
		
		if(gyroScheduler.Run()){
			freeReadCycles = 0;
			ReadGyro();
			gyroCNT = CNT;
			gyroLog = true;
		}

		if(magnScheduler.Run()){
			freeReadCycles = 0;
			ReadMagn();
			magnCNT = CNT;
			magnLog = true;
		}
		if(fuelScheduler.Run()){
			freeReadCycles = 0;
			ReadFuel();
			PutIntoBuffer(buffer, 'F', CNT, fuel_voltage, fuel_soc, fuel_rate);
		}
		if(timeScheduler.Run()){
			freeReadCycles = 0;
			ReadDateTime();
			PutIntoBuffer(buffer, 'T', CNT, hour, minute, second);	
			PutIntoBuffer(buffer, 'D', CNT, year, month, day);
		}
		
		if((gpsString = gps->Get()) != NULL){
			freeReadCycles = 0;
			PutIntoBuffer(buffer, 'P', CNT, gpsString, '\n');
		}


#ifdef EXTERNAL_IMU
		if(accl2Scheduler.Run()){
			freeReadCycles = 0;
			ReadAccl2();
			PutIntoBuffer(buffer, 'B', CNT, accl2_x, accl2_y, accl2_z);
		}
		
		if(gyro2Scheduler.Run()){
			freeReadCycles = 0;
			ReadGyro2();
			PutIntoBuffer(buffer, 'H', CNT, gyro2_x, gyro2_y, gyro2_z);
		}

		if(magn2Scheduler.Run()){
			freeReadCycles = 0;
			ReadMagn2();
			PutIntoBuffer(buffer, 'N', CNT, magn2_x, magn2_y, magn2_z);
		}
#endif


		//----------------------------------------------------------------------
		if(fuel_voltage < 3500){ //Dropout of 150mV@300mA, with some buffer
			LogStatusElement(buffer, kFatal, "Low Voltage");
			break;
		}
		
		if(SDBufferFree < buffer->GetkSize() / 2){
			led->low(); //On
			LogStatusElement(buffer, kInfo, "SDBuffer free less than 50%!");
		}else{
			led->high(); //Off
		}

		//----------------------------------------------------------------------
		if(acclLog){
			PutIntoBuffer(buffer, 'A', acclCNT, accl_x, accl_y, accl_z);
			acclLog = false;
		}
		
		if(gyroLog){
			PutIntoBuffer(buffer, 'G', gyroCNT, gyro_x, gyro_y, gyro_z);
			gyroLog = false;
		}
		
		if(magnLog){
			PutIntoBuffer(buffer, 'M', magnCNT, magn_x, magn_y, magn_z);
			magnLog = false;
		}

		
		//----------------------------------------------------------------------
		DisplayDeviceStatus(kDatalogging);

	}
	
	delete buffer;
}

void Datalog(void * parameter){
//WARNING: Must be called in it's own cog! (it has a cogstop at the end).

	ConcurrentBuffer *sdBuffer = new ConcurrentBuffer();

	const int buttonHz = 2*10;                    // 2*10 = 2Hz
	const int maxButtonCount = (buttonHz/10) * 3; // buttonHz * number of seconds pressed
	Scheduler buttonScheduler(buttonHz);
	int buttonCount = 0;
	
	
	
	SDBufferFree = sdBuffer->GetkSize();
	int i = 0xFFFFFF;
	while(true){
		volatile char * data;
		const int data_size = sdBuffer->Get(data);
	
		sd->Put((char *)data, data_size);
		//serial->Put(data);
		
		SDBufferFree = sdBuffer->GetFree();
		
		//Check if button has been pressed for some number of seconds.
		if(buttonScheduler.Run()){
			if(elum->GetButton()){
				buttonCount++;
				if(buttonCount >= maxButtonCount){
					break;
				}
			}else{
				buttonCount = 0;
			}
		}
	}
	
	//Todo(SRLM): Finish up remaining bytes in buffer here.
	
	datalogging = false;
	waitcnt(CLKFREQ/50 + CNT); //20 ms @80MHz
	
//	while((data_size = sdBuffer->Get(data)) != 0){
//	
//	}
	
	
	sd->Close();
	delete sdBuffer;
	waitcnt(CLKFREQ/10 + CNT); //Let everything settle down
	cogstop(cogid());
}



int main(void)
{


//Power
	pmic = new Max8819(kPIN_MAX8819_CEN, kPIN_MAX8819_CHG, kPIN_MAX8819_EN123, kPIN_MAX8819_DLIM1, kPIN_MAX8819_DLIM2);
	pmic->SetCharge(Max8819::HIGH); //TODO: There is some sort of bug where this *must* be in the code, otherwise it causes a reset.

//Buffer
	ConcurrentBuffer * buffer = new ConcurrentBuffer();
	
//Serial
	serial = new Serial;
//	serial->Start(kPIN_BLUETOOTH_RX, kPIN_BLUETOOTH_TX, kBLUETOOTH_BAUD);
	serial->Start(kPIN_USB_RX, kPIN_USB_TX, kUSB_BAUD);
	waitcnt(CLKFREQ/2 + CNT);

//DEBUG LED
	led = new Pin(kPIN_USER_4);
	led->high();


//EEPROM
	eeprom = new Eeprom;
	unitNumber = eeprom->Get(kEepromUnitAddress, 4);
	boardVersion = eeprom->Get(kEepromBoardAddress, 4);
	
//I2C
	bus = new i2c();
	//bus->Initialize(kPIN_EEPROM_SCL, kPIN_EEPROM_SDA); //For Beta Boards
	bus->Initialize(kPIN_I2C_SCL, kPIN_I2C_SDA);       //For Beta2 Boards
	
	fuel = new MAX17048(bus);
	if(fuel->GetStatus() == false){
		LogStatusElement(buffer, kError, "Failed to initialize the MAX17048");
	}else{
		ReadFuel();
	}
	
	lsm = new LSM303DLHC;
	if(!lsm->Init(bus)){
		LogStatusElement(buffer, kError, "Failed to initialize the LSM303DLHC.");
	}
	
	l3g = new L3GD20;
	if(!l3g->Init(bus)){
		LogStatusElement(buffer, kError, "Failed to initialize the L3GD20.");
	}
	
	rtc = new PCF8523(bus, kPIN_PCF8523_SQW);
	if(rtc->GetStatus() == false){
		LogStatusElement(buffer, kError, "Failed to initialize the PCF8523.");
	}
	
	
	
	
	
#ifdef EXTERNAL_IMU
//Second Bus for additional sensors.
	bus2 = new i2c();
	bus2->Initialize(kPIN_I2C_SCL_2, kPIN_I2C_SDA_2);       //For Beta2 Boards
	
	lsm2 = new LSM303DLHC;
	if(!lsm2->Init(bus2)){
		LogStatusElement(buffer, kError, "Failed to initialize the external LSM303DLHC.");
	}
	
	l3g2 = new L3GD20;
	if(!l3g2->Init(bus2)){
		LogStatusElement(buffer, kError, "Failed to initialize the external L3GD20.");
	}
#endif
	
	
	
	
	
//LEDs and Button
	elum = new Elum(kPIN_LEDR, kPIN_LEDG, kPIN_BUTTON);
	DisplayDeviceStatus(kPowerUp);
	if(elum->GetButton()){ //User has pressed the button, and powered the device
		 
	}else{                 //The device has been plugged in (and hence got it's power)
		LogStatusElement(buffer, kInfo, "Charging. Press button to begin datalogging.");
		pmic->Off(); //Turn off in case it's unpluged while charging
		while(!elum->GetButton()){
			ReadDateTime();
			ReadFuel();
			DisplayDeviceStatus(kCharging);
			
		} //Delay until/if they press the button.
		pmic->On(); //User wants to datalog, so we should keep the power on.
	}
		
	while(elum->GetButton()){} //Wait for the user to release the button
	waitcnt(CLKFREQ/10 + CNT);
	

//GPS
	gps = new MTK3339(kPIN_GPS_RX, kPIN_GPS_TX, kPIN_GPS_FIX);
	if(gps->GetStatus() == false){
		LogStatusElement(buffer, kError, "Failed to initialize the GPS.");
	}
	
	
//SD Card
	sd = new SecureDigitalCard;
	int mount = sd->Mount(kPIN_SD_DO, kPIN_SD_CLK, kPIN_SD_DI, kPIN_SD_CS);
	if(mount != 0)
	{
//		debug->Put("Failed to mount SD card: %i\n\r", mount);
		sdPresent = false;
		DisplayDeviceStatus(kNoSD);
	}else{
		sdPresent = true;
	}
	
	
	if(sdPresent){
		datalogging = true;
		//ReadConfiguration(sd);
		ReadDateTime();
		sd->SetDate(year+2000, month, day, hour, minute, second);
		OpenFile(unitNumber);


		//Log SD data in new cog
		int * datalogStack = (int *) malloc(stacksize);
		int datalogCog = cogstart(Datalog, NULL, datalogStack, stacksize);
		
		//Read Sensors (inc. I2C) in current cog
		ReadSensors();
		

//		//ReadI2C in new cog
//		int * i2cStack = (int*) malloc(stacksize);		
//		int i2cCog = cogstart(ReadI2C, NULL, i2cStack, stacksize);

//		//Datalog in current cog
//		SdDatalog(sd);
		
		//Cleanup
		waitcnt(CLKFREQ/10 + CNT); //Wait 100ms for everything to settle down.	
		free(datalogStack);

	}
	
	if(sdPresent){
		DisplayDeviceStatus(kDone);
	}else{
		waitcnt(CLKFREQ*5 + CNT); //Give the user time to see NO SD card status
	}
//	debug->Put("Done.\r\n");
	while(elum->GetButton()){}
//	debug->Put("Off.\r\n");
	waitcnt(CLKFREQ/2 + CNT);
	pmic->Off();
	
	
	//If we have reached this loop, then we are still plugged in...
	//TODO(SRLM): Intelligently be able to restart the program.
	for(;;);
	

}



























///**
//@returns the count to the character following a \n or a 0.

//In relation to SD card configuration

//*/
//int ParseKeyValue(const char * line, char * key, char * value){

//	int i = 0;
//	for(; line[i] != '='; i++){
//		key[i] = line[i];
//	}
//	key[i] = 0;
//	i++;

//	int valuei;
//	for(valuei = 0; line[i] != '\n' && line[i] != '\r' && line[i] != 0;i++){
//		value[valuei++] = line[i];
//	}
//	value[valuei] = 0;
//	
//	i++;
//	return i;
//	
//}

//bool ProcessKeyValue(const char * key, const char * value){

////	debug->Put("Key: '%s'\r\n", key);
////	debug->Put("Value: '%s'\r\n", value);

//	if(strcmp(key, "unit") == 0){
//		int unitNumber = Numbers::Dec(value);
//		eeprom->Put(kEepromUnitAddress, unitNumber, 4);
//		return true;
//	}
//	if(strcmp(key, "hardware version") == 0){
//		int boardVersion = -1;
//		if(     strcmp(value, "alpha") == 0){ boardVersion = kBoardAlpha;}
//		else if(strcmp(value, "beta")  == 0){ boardVersion = kBoardBeta;}
//		else if(strcmp(value, "beta2") == 0){ boardVersion = kBoardBeta2;}
//		else if(strcmp(value, "gamma") == 0){ boardVersion = kBoardGamma;}
//		else{ return false; }
//		eeprom->Put(kEepromBoardAddress, boardVersion, 4);
//		return true;
//	}
//	return false;

//}

//bool ReadConfiguration(SecureDigitalCard * sd){
//	int size = sd->Open("REDSCAD.CFG", 'r');
//	
//	if(size < 0){
//		return false;
//	}
//	char * buffer = new char[size];
//	if(buffer == NULL){
//		return false;
//	}
//	
//	sd->Get(buffer, size);
//	buffer[size] = 0; //null terminate

//	int i = 0;
//	bool success = true;
//	while(i < size){
//			
//		char key[20];
//		char value[20];
//			
//		i += ParseKeyValue(buffer + i, key, value);
//		if(ProcessKeyValue(key, value) == false){
//			success = false;
//		}
//		while( buffer[i] == '\r' || buffer[i] == '\n'){i++;}
//	}
//	
//	if(success){
////		debug->Put("Successfully read the configuration file.\r\n");
//		sd->Open("REDSCAD.OLD", 'w');
//		sd->Put(buffer, size);
//		sd->Close();
//		sd->Open("REDSCAD.CFG", 'd');
//	}else{
////		debug->Put("Failed to correctly read the configuration file.\r\n");
//	}

//	delete [] buffer;
//	return true;
//}




















