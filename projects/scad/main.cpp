// -----------------------------------------------------------------------------
// Board configuration
// ------------------------------------------------------------------------------
//#define EXTERNAL_IMU

#define BLUETOOTH

//#define DEBUG_PORT


// ------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------
#include <string.h>
#include <propeller.h>

#include <stdlib.h>

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

#include "stopwatch.h"

#include "pin.h"

/* Pin definitions */
#include "scadbeta2.h"


// ------------------------------------------------------------------------------

//TODO(SRLM): Add tests to beginning to not log when battery voltage is too low.

//TODO(SRLM): Change ELUM to something more generic...

//TODO(SRLM): check pointers for null, and so on (be safe!).

/**

Cog Usage:
0: Main
1: GPS driver (ASM)
2: Sensors Controller cog
3: SD driver (ASM)
4: Serial driver (ASM)(USB/Bluetooth)
5: Datalog Controller cog
6: 
7:
 */


/*
Other constants
 */

const unsigned short kEepromUnitAddress = 0xFFFC;
const unsigned short kEepromBoardAddress = 0xFFF8;
const unsigned short kEepromCanonNumberAddress = 0xFFF4;

const int kBoardAlpha = 0x0000000A;
const int kBoardBeta = 0x0000000B;
const int kBoardBeta2 = 0x000000B2;
const int kBoardGamma = 0x00000004;

const int kBluetoothTimeout = 10000;
//TODO: Do any of these need to be volatile?

Max8819 pmic;
Elum elum;
Eeprom eeprom;
Sensors sensors;
DatalogController dc;

#ifdef DEBUG_PORT
Serial * debug = NULL;
#endif

#ifdef BLUETOOTH
Bluetooth * bluetooth = NULL;
const int kBLUETOOTH_BAUD = 460800;
#endif

bool pluggedIn = true; //Assume plugged in

/*
Status Variables
 */

int lastButtonPressDuration = 0;
int currentButtonPressDuration = 0;

//bool datalogging = false; //TODO(SRLM): do we need this?

volatile int unitNumber;
volatile int boardVersion;

int * datalogStack = NULL; //TODO(SRLM): can these be static?
int * sensorsStack = NULL;

const int stacksize = sizeof (_thread_state_t) + sizeof (int)*3 + sizeof (int)*100;

enum LogLevel {
    kAll, kFatal, kError, kWarn, kInfo, kDebug
};
const char * LogLevelIdentifier[] = {"All:   ", "Fatal: ", "Error: ", "Warn:  ", "Info:  ", "Debug: "};

enum DeviceState {
    kPowerOn, kUnknownError, kNoSD, kCharging, kDatalogging, kDone, kUnknown,
    kPowerOff, kWaiting
};

volatile DeviceState currentState = kUnknown;


Scheduler * displayDeviceState = NULL;

void DisplayDeviceStatus(DeviceState state) {

    // in a single place, Get fuel status
    const int kFuelSoc = sensors.fuel_soc;

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
        elum.Pattern(Elum::kTriple);
    } else if (currentState == kNoSD) {
        //Alternate red and green
        elum.Pattern(Elum::kSingle);
    } else if (currentState == kCharging) {
        if (kFuelSoc < 90) {//pmic->GetCharge() == true){
            //Fade LED in and out
            elum.Fade(5);
        } else {
            //Done charging status
            elum.Fade(1000);
        }
    } else if (currentState == kDatalogging) {
        if (kFuelSoc > 25) {
            //Flash green LED
            elum.Flash(Elum::GREEN, 1000, kFuelSoc * 10);

        } else {
            //Flash red LED
            //Plus one for the 0 case.
            elum.Flash(Elum::RED, 1000, (kFuelSoc + 1) * 9);
        }
    } else if (currentState == kPowerOn
            || currentState == kWaiting) {
        elum.On(Elum::GREEN);
    } else {
        //If nothing else, then turn off LED.
        elum.Off();
    }


}

void LogVElement() {
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

    PIB::_string('V', CNT, string, '\0');

}

void LogRElement(char * filename) {
    PIB::_string('R', CNT, filename, '\0');
}

void LogStatusElement(const LogLevel level, const char * message) {
    char completeMessage[70];
    completeMessage[0] = '\0';
    strcat(completeMessage, LogLevelIdentifier[level]);
    strcat(completeMessage, message);
    PIB::_string('S', CNT, completeMessage, '\0');


#ifdef DEBUG_PORT
    debug->Put("\r\nS: ");
    debug->Put(completeMessage);
#endif
}

void LogStatusElement(const LogLevel level, const char * message, const int numberA) {
    char completeMessage[70];
    completeMessage[0] = '\0';
    strcat(completeMessage, LogLevelIdentifier[level]);
    strcat(completeMessage, message);
    strcat(completeMessage, Numbers::Dec(numberA));
    PIB::_string('S', CNT, completeMessage, '\0');
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
Scans the SD card and searches for filenames of the type "red9log#.csv",
where # is any size number. Actually, it uses the FILENAME and FILEEXT const
section variables for the filename. It starts scanning at filenumber 0, and
continues until it doesn't find a file of that number on the SD card. 

@param sd
@param identifier The unit number to store in the first part of the filename.
@returns the current file number if successful, or a negative number if
 * unsuccessful. -1 is returned if all filenames are taken.
 */
int GetSDCannonNumber(SecureDigitalCard * sd, int lastFileNumber, int identifier) {
    char buffer[12];
    //    char buff2[4];
    int currentNumber = (lastFileNumber + 1) % 1000;
    //canonNumber refers to the last created file, so we need
    // to move to the next free one. This line is necessary
    // in case the file using the current canonNumber has been
    // deleted: we don't want to still create a file with
    // that number.


    while (currentNumber != lastFileNumber) { //Loop until we have looped around.

        buffer[0] = 0;

        //This version is B###F###.EXT
        strcat(buffer, "B");
        strcat(buffer, Numbers::Dec(identifier));
        strcat(buffer, "F");
        strcat(buffer, Numbers::Dec(currentNumber));
        strcat(buffer, ".RNB");
        sd->Open(buffer, 'r');
        if (sd->HasError()) {
            sd->ClearError();
            break;
        }

        currentNumber = (currentNumber + 1) % 1000;
    }

    sd->Close();
    if (currentNumber == lastFileNumber) {
        return -1;
    }

    return currentNumber;
}

/** Figures out the best Canon number.
 * 
 * @return the best guess canon number.
 */
int GetCanonNumber(void) {
    int canonNumber = eeprom.Get(kEepromCanonNumberAddress, 4);

    SecureDigitalCard sd = SecureDigitalCard();
    sd.Mount(board::kPIN_SD_DO, board::kPIN_SD_CLK,
            board::kPIN_SD_DI, board::kPIN_SD_CS);
    if (sd.HasError()) {
        //error!
    } else {
        int newCanonNumber = GetSDCannonNumber(&sd, canonNumber, unitNumber);
        sd.Unmount();
        if (newCanonNumber >= 0) {
            return newCanonNumber;
        }
    }

    //Return a default canonNumber...
    return (canonNumber + 1) % 1000;

}

/** Opens a new file on the SD card.
 * 
 * @return false on error, true otherwise
 */
bool SetupLogSD(int newCanonNumber) {
    sensors.Update(Sensors::kTime, false);
    dc.SetClock(sensors.year + 2000, sensors.month, sensors.day,
            sensors.hour, sensors.minute, sensors.second);
    if (dc.InitSD(newCanonNumber, unitNumber)) {
        dc.SetLogSD(true); //Start recording to SD card.
        return true;
    } else {
        return false;
    }
}

bool SetupLogSerial(int newCanonNumber) {
    dc.SetLogSerial(true);
    return true;
}

void GlobalLogData(void) {
    LogVElement();
    if (strlen((char *) dc.GetCurrentFilename()) != 0) {
        LogRElement((char *) dc.GetCurrentFilename()); //TODO(SRLM): make sure that the filename is correctly stored
    }

}

void SetupLog(bool newLogSD, bool newLogSerial) {
    int canonNumber = GetCanonNumber();
    if (newLogSD == true) {
        if (SetupLogSD(canonNumber) == false) {
            DisplayDeviceStatus(kNoSD);
            waitcnt(CLKFREQ * 5 + CNT);
            //return -1;
            return;
        } else {
            eeprom.Put(kEepromCanonNumberAddress, canonNumber, 4); //Store canonFilenumber for persistence
        }
    } else {
        dc.SetLogSD(false);
    }

    if (newLogSerial == true) {
        SetupLogSerial(canonNumber);
        eeprom.Put(kEepromCanonNumberAddress, canonNumber, 4); //Store canonFilenumber for persistence
    } else {
        dc.SetLogSerial(false);
    }

    GlobalLogData();

    //TODO(SRLM): Add in something here to log the scale factors as well.

    sensors.SetAutomaticRead(true);

    //datalogging = true;

    pmic.On(); //Make sure we don't lose power while datalogging!

}

void CloseLog() {
    sensors.SetAutomaticRead(false);

    //Cleanup
    waitcnt(CLKFREQ / 5 + CNT); //Wait 200ms for everything to settle down.	

    dc.SetLogSD(false);
    dc.SetLogSerial(false);

}

Scheduler * bufferNoticeScheduler = NULL; //TODO(SRLM): make static...

void DataloggingInnerLoop(void) {
    if (bufferNoticeScheduler == NULL) {
        bufferNoticeScheduler = new Scheduler(500);
    }
    Pin led(22);
    if (dc.GetBufferFree() < ConcurrentBuffer::GetkSize() / 4) {
        led.high();
        LogStatusElement(kInfo, "SDBuffer free less than 25%!");
    } else {
        //led.low();
    }

    if (bufferNoticeScheduler->Run()) {
        char buffer[50] = "SDBuffer Free: ";

        LogStatusElement(kInfo, strcat(buffer, Numbers::Dec(dc.GetBufferFree())));
    }

    //----------------------------------------------------------------------


}

class StateMachine {
    typedef void (StateMachine::*stateFunctionPointer)(void);
    stateFunctionPointer nextState;
public:

    StateMachine() {
        nextState = &StateMachine::Wait;
    }

    void Run(void) {
        (this->*nextState)();
    }

    void Wait(void);
    void ReadPacketHeader(void);
    void ProcessElementPacket(void);
    void ProcessListFilesPacket(void);
    void ProcessOutputDeviceSettingsPacket(void);
    void ProcessPowerPacket(void);
    void PowerOff(void);
    void ProcessDataloggingPacket(void);
    void ButtonDatalogStart(void);
    void DataloggingWait(void);
    void DataloggingReadPacketHeader(void);
    void DataloggingProcessPowerPacket(void);
    void DataloggingSoftOff(void);
    void DataloggingProcessDataloggingPacket(void);
};

void StateMachine::Wait(void) {
#ifdef DEBUG_PORT
    //debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    sensors.Update(Sensors::kFuel, false);
    if (pluggedIn == true) {
        DisplayDeviceStatus(kCharging);
    } else {
        DisplayDeviceStatus(kWaiting);
    }

    // Test: Battery is too low?
    if (sensors.fuel_voltage < 3500
            and sensors.fuel_voltage != sensors.kDefaultFuelVoltage) { //Dropout of 150mV@300mA, with some buffer
#ifdef DEBUG_PORT
        debug->Put("\r\nWarning: Low fuel!!! Turning off.");
#endif
        LogStatusElement(kFatal, "Low Voltage");
        nextState = &StateMachine::PowerOff;
    } else if (lastButtonPressDuration > 50 && lastButtonPressDuration < 1000) {
        nextState = &StateMachine::ButtonDatalogStart;
    } else if (currentButtonPressDuration > 3000) {
        nextState = &StateMachine::PowerOff;
    } else if (bluetooth->Get(0) == 'C') {
        nextState = &StateMachine::ReadPacketHeader;
    } else {
        nextState = &StateMachine::Wait;
    }
}

void StateMachine::ReadPacketHeader(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    int commandCharacter;
    for (int i = 0; i < 5; i++) { // 4 time bytes + 1 command byte
        commandCharacter = bluetooth->Get(kBluetoothTimeout);
        if (commandCharacter == -1) {
            nextState = &StateMachine::Wait;
        }
    }

    if (commandCharacter == 'E') {
        nextState = &StateMachine::ProcessElementPacket;
    } else if (commandCharacter == 'F') {
        nextState = &StateMachine::ProcessListFilesPacket;
    } else if (commandCharacter == 'M') {
        nextState = &StateMachine::ProcessOutputDeviceSettingsPacket;
    } else if (commandCharacter == 'P') {
        nextState = &StateMachine::ProcessPowerPacket;
    } else if (commandCharacter == 'D') {
        nextState = &StateMachine::ProcessDataloggingPacket;
    } else {
        nextState = &StateMachine::Wait;
    }
}

void StateMachine::ProcessElementPacket(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    Sensors::SensorType type;
    switch (bluetooth->Get(kBluetoothTimeout)) {
        case 'A':
            type = Sensors::kAccl;
            break;
        case 'B':
            type = Sensors::kAccl2;
            break;
        case 'D':
            type = Sensors::kTime;
            break;
        case 'E':
            type = Sensors::kBaro;
            break;
        case 'F':
            type = Sensors::kFuel;
            break;
        case 'G':
            type = Sensors::kGyro;
            break;
        case 'H':
            type = Sensors::kGyro2;
            break;
        case 'M':
            type = Sensors::kMagn;
            break;
        case 'N':
            type = Sensors::kMagn2;
            break;
        case 'P':
            type = Sensors::kGPS;
            break;
        default:
            type = Sensors::kNone;
            break;
    }


#ifdef DEBUG_PORT
    //debug->Put("\r\nUpdating sensor.");
    //debug->Put("\r\nDatalogController Buffer free: ");
    //debug->Put(Numbers::Dec(dc->GetBufferFree()));
#endif
    sensors.Update(type, true);

    nextState = &StateMachine::Wait;
}

void StateMachine::ProcessListFilesPacket(void) {
    //TODO(SRLM): Do something here...
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif

    char filenameBuffer[15];

    while (dc.GetNextFilenameOnDisk(filenameBuffer) == true) {
#ifdef DEBUG_PORT
        debug->PutFormatted("\r\n   Found filename: %s", filenameBuffer);
#endif
        LogRElement(filenameBuffer);
    }

    nextState = &StateMachine::Wait;
}

void StateMachine::ProcessOutputDeviceSettingsPacket(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    GlobalLogData();

    nextState = &StateMachine::Wait;
}

void StateMachine::ProcessPowerPacket(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    int input = bluetooth->Get(kBluetoothTimeout);

    if (input == 'T') {
        //Do something to renew the power...
        dc.SetLogSerial(true); //We're turned on, so "respond" to serial with this.
        pmic.On();
        nextState = &StateMachine::Wait;
    } else if (input == 'F') {
        nextState = &StateMachine::PowerOff;
    } else {
        nextState = &StateMachine::Wait;
    }


}

void StateMachine::PowerOff(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    CloseLog();
    DisplayDeviceStatus(kPowerOff);

    //Spin while the button is pressed.
    while (elum.GetButton()) {
    }
    pmic.Off(); //TODO(SRLM): Make this set a global variable instead of hacking it...
    nextState = &StateMachine::PowerOff; //Shouldn't matter, but just in case...
}

void StateMachine::ProcessDataloggingPacket(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    bool logSD = bluetooth->Get(kBluetoothTimeout) == 'T';
    bool logSerial = bluetooth->Get(kBluetoothTimeout) == 'T';

    if (logSD == false && logSerial == false) {
        nextState = &StateMachine::DataloggingSoftOff;
    } else {
        SetupLog(logSD, logSerial);
        nextState = &StateMachine::DataloggingWait;
    }
}

void StateMachine::ButtonDatalogStart(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    SetupLog(true, false);
    nextState = &StateMachine::DataloggingWait;
}

void StateMachine::DataloggingSoftOff(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    CloseLog();
    nextState = &StateMachine::Wait;
}

void StateMachine::DataloggingWait(void) {
#ifdef DEBUG_PORT
    //debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    DisplayDeviceStatus(kDatalogging);
    DataloggingInnerLoop();

    //sensors.Update(Sensors::kFuel, false);

    // Test: Battery is too low?
    if (sensors.fuel_voltage < 3500
            and sensors.fuel_voltage != sensors.kDefaultFuelVoltage) { //Dropout of 150mV@300mA, with some buffer
#ifdef DEBUG_PORT
        debug->Put("\r\nWarning: Low fuel!!! Turning off.");
#endif
        LogStatusElement(kFatal, "Low Voltage");
        nextState = &StateMachine::PowerOff;
    } else if (currentButtonPressDuration > 3000) {
        nextState = &StateMachine::PowerOff;
    } else if (bluetooth->Get(0) == 'C') {
        nextState = &StateMachine::DataloggingReadPacketHeader;
    } else {
        nextState = &StateMachine::DataloggingWait;
    }
}

void StateMachine::DataloggingReadPacketHeader(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    int commandCharacter;
    for (int i = 0; i < 5; i++) { // 4 time bytes + 1 command byte
        commandCharacter = bluetooth->Get(kBluetoothTimeout);
        if (commandCharacter == -1) {
            nextState = &StateMachine::Wait;
        }
    }

    if (commandCharacter == 'P') {
        nextState = &StateMachine::DataloggingProcessPowerPacket;
    } else if (commandCharacter == 'D') {
        nextState = &StateMachine::DataloggingProcessDataloggingPacket;
    } else {
        nextState = &StateMachine::DataloggingWait;
    }
}

void StateMachine::DataloggingProcessDataloggingPacket(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    bool logSD = bluetooth->Get(kBluetoothTimeout) == 'T';
    bool logSerial = bluetooth->Get(kBluetoothTimeout) == 'T';
    if (logSD == false && logSerial == false) {
        nextState = &StateMachine::DataloggingSoftOff;
    } else {
        nextState = &StateMachine::DataloggingWait;
    }
}

void StateMachine::DataloggingProcessPowerPacket(void) {
#ifdef DEBUG_PORT
    debug->PutFormatted("\r\nCurrent State Function: %s", __func__);
#endif
    bool powerOn = bluetooth->Get(kBluetoothTimeout) == 'T';
    if (powerOn == true) {
        nextState = &StateMachine::DataloggingWait;
    } else {
        nextState = &StateMachine::PowerOff;
    }
}

void MainLoop(void) {


    Stopwatch buttonTimer;
    StateMachine machine;

    while (true) {


        //Time the button:
        if (elum.GetButton() == true) {
            if (buttonTimer.GetStarted() == false) {
                buttonTimer.Start();
            }
            currentButtonPressDuration = buttonTimer.GetElapsed();
        } else {
            lastButtonPressDuration = currentButtonPressDuration;
            currentButtonPressDuration = 0;
            buttonTimer.Reset();
        }

        machine.Run();

    }
}

void init(void) {

    //Power
    pmic.Start(board::kPIN_MAX8819_CEN, board::kPIN_MAX8819_CHG,
            board::kPIN_MAX8819_EN123, board::kPIN_MAX8819_DLIM1,
            board::kPIN_MAX8819_DLIM2);
    pmic.SetCharge(Max8819::HIGH); //TODO: There is some sort of bug where this *must* be in the code, otherwise it causes a reset.

    //EEPROM
    eeprom.Start();
    unitNumber = eeprom.Get(kEepromUnitAddress, 4);
    boardVersion = eeprom.Get(kEepromBoardAddress, 4);
#ifdef DEBUG_PORT
    debug->Put("\r\nEEPROM initialized.");
#endif
#ifdef BLUETOOTH
    bluetooth = new Bluetooth(board::kPIN_BLUETOOTH_RX, board::kPIN_BLUETOOTH_TX,
            board::kPIN_BLUETOOTH_CTS, board::kPIN_BLUETOOTH_CONNECT);

#ifdef DEBUG_PORT
    debug->Put("\r\nBluetooth initialized.");
#endif    
#endif


    //Datalog Controller
    dc.Start(board::kPIN_SD_DO, board::kPIN_SD_CLK,
            board::kPIN_SD_DI, board::kPIN_SD_CS);
    dc.SetLogSerial(false);
    dc.SetLogSD(false);
    datalogStack = (int *) malloc(stacksize);
    //int datalogCog = 
    cogstart(DatalogCogRunner, &dc, datalogStack, stacksize);

#ifdef DEBUG_PORT
    debug->Put("\r\nDatalog Cog initialized.");
#endif
    //Sensors
    sensors.init();
    //Read Sensors (inc. I2C) in new cog
    sensorsStack = (int *) malloc(stacksize);
    cogstart(SensorsServerRunner, &sensors, sensorsStack, stacksize);

#ifdef DEBUG_PORT
    debug->Put("\r\nSensor Cog initialized.");
#endif

    //LEDs and Button
    elum.Start(board::kPIN_LEDR, board::kPIN_LEDG, board::kPIN_BUTTON);




}

int main(void) {

#ifdef DEBUG_PORT    
    debug = new Serial();
    debug->Start(31, 30, 460800);
    debug->Put("\r\nSCAD Debug Port: ");
#endif

    init();
    DisplayDeviceStatus(kPowerOn);



    while (elum.GetButton()) { //Check the plugged in assumption
        pluggedIn = false; //If it's not plugged in, then the button will be pressed.
        pmic.On(); //It's not plugged in, so we should keep the power on...
    } //Wait for the user to release the button, if turned on that way.
    waitcnt(CLKFREQ / 10 + CNT);

    if (pluggedIn) {
        pmic.Off(); //Turn off in case it's unplugged while charging
    }

#ifdef DEBUG_PORT
    debug->Put("\r\nStarting main loop...");
#endif


    MainLoop();
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




















