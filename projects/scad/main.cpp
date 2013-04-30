// -----------------------------------------------------------------------------
// Board configuration
// ------------------------------------------------------------------------------
//#define EXTERNAL_IMU

#define BLUETOOTH


// ------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------
#include <string.h>
#include <propeller.h>

#include "c++-alloc.h"

#include "serial.h"
#include "i2c.h"
#include "securedigitalcard.h"
#include "max8819.h"
#include "elum.h"
#include "numbers.h"
#include "scheduler.h"
#include "concurrentbuffer.h"
#include "pib.h"
#include "eeprom.h"

#include "rovingbluetooth.h"
#include "DatalogController.h"
#include "Sensors.h"

#include "pin.h"

/* Pin definitions */
#include "scadbeta2.h"


// ------------------------------------------------------------------------------

//TODO(SRLM): Add tests to beginning to not log when battery voltage is too low.

//TODO(SRLM): Change ELUM to something more generic...

//TODO(SRLM): check pointers for null, and so on (be safe!).

//TODO(SRLM): ConcurrentBuffer puts don't need an object, right? So, I should make each use the static methods (and not pass a class instance around...)

/**

Cog Usage:
0: Main (inc. datalog controller)
1: GPS
2: I2C Getter
3: SD driver
4: Serial (USB/Bluetooth)
5: Controller cog
6:
7:
 */


/*
Other constants
 */

const int kGPS_BAUD = 9600;

const unsigned short kEepromUnitAddress = 0xFFFC;
const unsigned short kEepromBoardAddress = 0xFFF8;
const unsigned short kEepromCanonNumberAddress = 0xFFF4;

const int kBoardAlpha = 0x0000000A;
const int kBoardBeta = 0x0000000B;
const int kBoardBeta2 = 0x000000B2;
const int kBoardGamma = 0x00000004;


//TODO: Do any of these need to be volatile?

//Serial     * serial = NULL;
Max8819 * pmic = NULL;
Elum * elum = NULL;
Eeprom * eeprom = NULL;
Sensors * sensors = NULL;
DatalogController * dc = NULL;

#ifdef BLUETOOTH
Bluetooth * bluetooth = NULL;

const int kBLUETOOTH_BAUD = 460800;

#endif

//Pin * led = NULL;


/*
Status Variables
 */
volatile bool datalogging = false;


volatile int unitNumber;
volatile int boardVersion;

int * datalogStack = NULL;
int * sensorsStack = NULL;

const int stacksize = sizeof (_thread_state_t) + sizeof (int)*3 + sizeof (int)*100;

enum LogLevel {
    kAll, kFatal, kError, kWarn, kInfo, kDebug
};
const char * LogLevelIdentifier[] = {"All:   ", "Fatal: ", "Error: ", "Warn:  ", "Info:  ", "Debug: "};

enum DeviceState {
    kPowerUp, kUnknownError, kNoSD, kCharging, kDatalogging, kDone, kUnknown
};

volatile DeviceState currentState = kUnknown;


Scheduler * displayDeviceState = NULL;

void DisplayDeviceStatus(DeviceState state) {

    //Get fuel status in a single place
    const int kFuelSoc = sensors->fuel_soc;

    //Make sure that we're keeping track of the last time display has been updated.
    if (displayDeviceState == NULL) {
        displayDeviceState = new Scheduler(1); //0.1 Hz
    }

    //Exit if the state is the same, and we've updated the display recently.
    if (currentState == state && !displayDeviceState->Run()) {
        return;
    }

    currentState = state;

    if (currentState == kUnknownError) {
        //Alternate red and green triple
        elum->Pattern(Elum::kTriple);
    } else if (currentState == kNoSD) {
        //Alternate red and green
        elum->Pattern(Elum::kSingle);
    } else if (currentState == kCharging) {
        if (kFuelSoc < 90) {//pmic->GetCharge() == true){
            //Fade LED in and out
            elum->Fade(5);
        } else {
            //Done charging status
            elum->Fade(1000);
        }
    } else if (currentState == kDatalogging) {
        if (kFuelSoc > 25) {
            //Flash green LED
            elum->Flash(Elum::GREEN, 1000, kFuelSoc * 10);

        } else {
            //Flash red LED
            //Plus one for the 0 case.
            elum->Flash(Elum::RED, 1000, (kFuelSoc + 1) * 9);
        }
    } else if (currentState == kPowerUp) {
        elum->On(Elum::GREEN);
    } else {
        //If nothing else, then turn off LED.
        elum->Off();
    }


}

void LogVElement(ConcurrentBuffer * buffer) {
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

    PIB::_string(buffer, 'V', CNT, string, '\0');

}

void LogRElement(ConcurrentBuffer * buffer) {
    PIB::_string(buffer, 'R', CNT, (char *) dc->GetCurrentFilename(), '\0');
}

void LogStatusElement(ConcurrentBuffer * buffer, const LogLevel level, const char * message) {
    char completeMessage[70];
    completeMessage[0] = '\0';
    strcat(completeMessage, LogLevelIdentifier[level]);
    strcat(completeMessage, message);
    PIB::_string(buffer, 'S', CNT, completeMessage, '\0');
}

void LogStatusElement(ConcurrentBuffer * buffer, const LogLevel level, const char * message, const int numberA) {
    char completeMessage[70];
    completeMessage[0] = '\0';
    strcat(completeMessage, LogLevelIdentifier[level]);
    strcat(completeMessage, message);
    strcat(completeMessage, Numbers::Dec(numberA));
    PIB::_string(buffer, 'S', CNT, completeMessage, '\0');
}

void SensorsServerRunner(void * parameter) {
    Sensors * temp = (Sensors *) parameter;
    temp->Server();
    waitcnt(CLKFREQ / 10 + CNT); //Let everything settle down
    cogstop(cogid());
}

void DatalogCogRunner(void * parameter) {
    DatalogController * temp = (DatalogController *) parameter;
    temp->Server();
    waitcnt(CLKFREQ / 10 + CNT); //Let everything settle down
    cogstop(cogid());
}

/**
 * 
 * @return True if datalogging successfully occurred, false otherwise.
 */
bool DatalogControllerLoop(void) {
    //Number of the last written file
    int canonNumber = eeprom->Get(kEepromCanonNumberAddress, 4);
    sensors->Update(Sensors::kTime);
    dc->SetClock(sensors->year + 2000, sensors->month, sensors->day,
            sensors->hour, sensors->minute, sensors->second);

    int newCanonNumber = dc->InitSD(board::kPIN_SD_DO, board::kPIN_SD_CLK,
            board::kPIN_SD_DI, board::kPIN_SD_CS,
            canonNumber, unitNumber);


    if (newCanonNumber >= 0) { //Success!!!
        eeprom->Put(kEepromCanonNumberAddress, newCanonNumber, 4); //Store canonFilenumber for persistance
        datalogging = true;
        dc->SetLogSD(true); //Start recording to SD card.

        //Buffer
        ConcurrentBuffer * buffer = new ConcurrentBuffer();

        LogRElement(buffer);
        LogVElement(buffer);

        const int buttonHz = 2 * 10; // 2*10 = 2Hz
        const int maxButtonCount = (buttonHz / 10) * 3; // buttonHz * number of seconds pressed
        Scheduler buttonScheduler(buttonHz);
        int buttonCount = 0;

        while (datalogging) {
            //From sensors
            //----------------------------------------------------------------------
            if (sensors->fuel_voltage < 3500
                    and sensors->fuel_voltage != sensors->kDefaultFuelVoltage) { //Dropout of 150mV@300mA, with some buffer
                LogStatusElement(buffer, kFatal, "Low Voltage");

                datalogging = false;
            }

            if (dc->GetBufferFree() < ConcurrentBuffer::GetkSize() / 2) {
                //led->low(); //On
                LogStatusElement(buffer, kInfo, "SDBuffer free less than 50%!");
            }
            //----------------------------------------------------------------------
            DisplayDeviceStatus(kDatalogging);

            //Check if button has been pressed for some number of seconds.
            if (buttonScheduler.Run()) {
                if (elum->GetButton()) {
                    buttonCount++;
                    if (buttonCount >= maxButtonCount) {
                        datalogging = false;
                    }
                } else {
                    buttonCount = 0;
                }
            }

        }

        datalogging = false;

        //Cleanup
        waitcnt(CLKFREQ / 5 + CNT); //Wait 200ms for everything to settle down.	

        dc->SetLogSD(false);
        free(datalogStack);
        free(sensorsStack);

        DisplayDeviceStatus(kDone);
        
        delete buffer;
        return true;
    } else { //Could not open file!
        //TODO(SRLM): put up log message for no SD here.
        DisplayDeviceStatus(kNoSD);
        waitcnt(CLKFREQ * 5 + CNT); //Give the user time to see NO SD card status

        return false;
    }

}

void init(void) {

    //Power
    pmic = new Max8819(board::kPIN_MAX8819_CEN, board::kPIN_MAX8819_CHG,
            board::kPIN_MAX8819_EN123, board::kPIN_MAX8819_DLIM1,
            board::kPIN_MAX8819_DLIM2);
    pmic->SetCharge(Max8819::HIGH); //TODO: There is some sort of bug where this *must* be in the code, otherwise it causes a reset.



    //EEPROM
    eeprom = new Eeprom;
    unitNumber = eeprom->Get(kEepromUnitAddress, 4);
    boardVersion = eeprom->Get(kEepromBoardAddress, 4);

#ifdef BLUETOOTH
    bluetooth = new Bluetooth(board::kPIN_BLUETOOTH_RX, board::kPIN_BLUETOOTH_TX,
            board::kPIN_BLUETOOTH_CTS, board::kPIN_BLUETOOTH_CONNECT);

#endif


    //Datalog Controller
    dc = new DatalogController();
    dc->SetLogSerial(true);
    dc->SetLogSD(false);
    datalogStack = (int *) malloc(stacksize);
    //int datalogCog = 
    cogstart(DatalogCogRunner, dc, datalogStack, stacksize);

    //Sensors
    sensors = new Sensors();
    sensors->init();
    //Read Sensors (inc. I2C) in new cog
    sensorsStack = (int *) malloc(stacksize);
    cogstart(SensorsServerRunner, sensors, sensorsStack, stacksize);


    //LEDs and Button
    elum = new Elum(board::kPIN_LEDR, board::kPIN_LEDG, board::kPIN_BUTTON);
    DisplayDeviceStatus(kPowerUp);

}

int main(void) {

    init();

    //System State
    datalogging = false;

    //Buffer
    ConcurrentBuffer * buffer = new ConcurrentBuffer();

    bool pluggedIn = true; //Assume plugged in
    while (elum->GetButton()) { //Check the plugged in assumption
        pluggedIn = false; //If it's not plugged in, then the button will be pressed.
        pmic->On(); //It's not plugged in, so we should keep the power on...
    } //Wait for the user to release the button, if turned on that way.
    waitcnt(CLKFREQ / 10 + CNT);

    if (pluggedIn) {
        pmic->Off(); //Turn off in case it's unpluged while charging
        DisplayDeviceStatus(kCharging);
    }

    int ButtonOnTime = 0;

    while (true) {


        if (elum->GetButton() == true) {
            ButtonOnTime++;
        } else {
            //Has user "briefly" pressed the power button? That indicates to datalog.
            if (ButtonOnTime > 50 && ButtonOnTime < 1000) {
                //"Standard" Datalog here:
                //Source = Sensors
                dc->SetLogSD(true);
                dc->SetLogSerial(false);
                pmic->On(); //Make sure we don't lose power while datalogging!
                bool datalogResult = DatalogControllerLoop();
                break;
            }
            ButtonOnTime = 0;
        }

        if (ButtonOnTime > 3000) { //Power off press
            break;
        }

        //Process Serial Commands here

        waitcnt(CLKFREQ / 1000 + CNT);



    }








    //Spin while the button is pressed.
    while (elum->GetButton()) {
    }
    waitcnt(CLKFREQ / 2 + CNT);
    pmic->Off();


    //If we have reached this loop, then we are still plugged in...
    //TODO(SRLM): Intelligently be able to restart the program.
    for (;;);


}
























//ReadConfiguration(sd);


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




















