// -----------------------------------------------------------------------------
// Board configuration
// ------------------------------------------------------------------------------
//#define EXTERNAL_IMU

#define BLUETOOTH

#define DEBUG_PORT


// ------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------
#include <string.h>
#include <propeller.h>

#include <stdlib.h>

#include "c++-alloc.h"

#include "serial.h"
#include "i2c.h"
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

//TODO(SRLM): Make sure there isn't a problem with the delay between a command to DatalogController (such as start SD), and the time that getsdActive returns the new state.

// ------------------------------------------------------------------------------

//TODO(SRLM): Add tests to beginning to not log when battery voltage is too low.

//TODO(SRLM): Change ELUM to something more generic...

//TODO(SRLM): check pointers for null, and so on (be safe!).

//TODO(SRLM): Numbers::Dec is not thread safe... Check!

//TODO(SRLM): Somewhere I have two calls to dc.AddScales();

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

void LogRElement(const char * filename) {
    char buffer[15] = "T";
    strcat(buffer, filename);
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

void OutputGlobalLogHeaders(void) {
    LogVElement();
    sensors.AddScales();
    //TODO(SRLM): Add in something here to log the scale factors as well.
}

void TurnSDOn(void) {
#ifdef DEBUG_PORT
    debug->Put("\r\nTurnSDOn()");
#endif
    if (dc.getsdMounted() == true) {
        if (dc.getsdActive() == false) {
            pmic.On(); //Make sure we don't lose power while datalogging!
            sensors.SetAutomaticRead(false);
            dc.SetClock(sensors.year + 2000, sensors.month, sensors.day,
            sensors.hour, sensors.minute, sensors.second);
            dc.StartSD();
            dc.BlockUntilWaiting();
            eeprom.Put(kEepromCanonNumberAddress, dc.GetLastFileNumber(), 4); //Store canonFilenumber for persistence
            OutputGlobalLogHeaders();
            sensors.SetAutomaticRead(true);
            DisplayDeviceStatus(kDatalogging);
        }
    } else {
        debug->Put("\r\nSD Error!");
        DisplayDeviceStatus(kNoSD);
        debug->Put("\r\n before");
        LogStatusElement(kError, "SD Error!");
        debug->Put("\r\n after...");
    }
}

void TurnSDOff(void) {
#ifdef DEBUG_PORT
    debug->Put("\r\nTurnSDOff()");
#endif    
    if (dc.getsdActive() == true) {
        dc.StopSD();
    }
    
    DisplayDeviceStatus(kWaiting);
}

void TransferFile(const char * filename) {
#ifdef DEBUG_PORT
    debug->Put("\r\nTransferFile()");
    debug->Put(" Filename: '");
    debug->Put(filename);
    debug->Put("'");
#endif
    sensors.SetAutomaticRead(false);
    if (dc.getsdActive() == false) {
        //TODO output file Header here
        dc.InjectFile(filename);
        //TODO output file Closer here
    }
    sensors.SetAutomaticRead(true);
    //TODO(SRLM): Add error output if SD is open (and can't list files)
}

void ListFile(void) {
#ifdef DEBUG_PORT
    debug->Put("\r\nListFile()");
#endif    
    if (dc.getsdActive() == false) {
        dc.ListFilenamesOnDisk();
    }
    //TODO(SRLM): Add error output if SD is open (and can't list files)
}

void ReadStringFromBluetooth(char * filename, const int length) {
    int i;
    for (i = 0; i < length; i++) {
        filename[i] = bluetooth->Get();
        if (filename[i] == '\0') {
            break;
        }
    }

    //Make sure we don't overflow past the end of the filename.
    if (filename[i] != '\0') {
        filename[i] = '\0';
    }
}

void ReviveSelf(void) {
#ifdef DEBUG_PORT
    debug->Put("\r\nReviveSelf()");
#endif    
    pmic.On();
    //Do something where it changes the LED to waiting (if charging)
}

void KillSelf(void) {
#ifdef DEBUG_PORT
    debug->Put("\r\nKillSelf()");
#endif    
    TurnSDOff();
    DisplayDeviceStatus(kPowerOff);

    pmic.Off(); //TODO(SRLM): Make this set a global variable instead of hacking it...

    while (true) {
    }

    //TODO(SRLM): Add check here: if still on, we're plugged in.
}

void InnerLoop(void) {





    // Test: Battery is too low?
    if (sensors.fuel_voltage < 3500
            and sensors.fuel_voltage != sensors.kDefaultFuelVoltage) { //Dropout of 150mV@300mA, with some buffer
#ifdef DEBUG_PORT
        debug->Put("\r\nWarning: Low fuel!!! Turning off.");
#endif
        LogStatusElement(kFatal, "Low Voltage");
        KillSelf();
    }


    if (bluetooth->Get(0) == 'C') {
        for (int i = 0; i < 4; i++) {
            bluetooth->Get(); //Throw away CNT
            //TODO(SRLM): add test to make sure that these are all 0's.
        }

        int command = bluetooth->Get();
        if (command == 'D') {
            int dataloggingStateChar = bluetooth->Get();
            if (dataloggingStateChar == 'T') {
                TurnSDOn();
            } else if (dataloggingStateChar == 'F') {
                TurnSDOff();
            }
        } else if (command == 'P') {
            int temp = bluetooth->Get();
            if (temp == 'T') {
                ReviveSelf();
            } else if (temp == 'F') {
                KillSelf();
            }

        } else if (command == 'M') {
            OutputGlobalLogHeaders();
        } else if (command == 'T') {
            //TODO(SRLM) read file from bluetooth.
            char filename[13];
            ReadStringFromBluetooth(filename, 13);
            TransferFile(filename);
        } else if (command == 'L') {
            ListFile();
        }

    }

    if (lastButtonPressDuration > 50 && lastButtonPressDuration < 1000) {
        TurnSDOn();
    }
    if (currentButtonPressDuration > 3000) {
        KillSelf();
    }



}

void MainLoop(void) {
    Stopwatch buttonTimer;

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

        InnerLoop();
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


    int canonNumber = eeprom.Get(kEepromCanonNumberAddress, 4);
    //Datalog Controller
    dc.Init(canonNumber, unitNumber, board::kPIN_SD_DO, board::kPIN_SD_CLK,
            board::kPIN_SD_DI, board::kPIN_SD_CS);
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


    sensors.SetAutomaticRead(true);

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
        DisplayDeviceStatus(kWaiting);
    } //Wait for the user to release the button, if turned on that way.
    waitcnt(CLKFREQ / 10 + CNT);

    if (pluggedIn == true) {
        DisplayDeviceStatus(kCharging);
        pmic.Off(); //Turn off in case it's unplugged while charging
    }

#ifdef DEBUG_PORT
    debug->Put("\r\nStarting main loop...");
#endif


    MainLoop();
}


