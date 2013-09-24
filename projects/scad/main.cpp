// -----------------------------------------------------------------------------
// Board configuration
// ------------------------------------------------------------------------------
#define DEBUG_PORT


// ------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------
#include <string.h>
#include <propeller.h>

#include <stdlib.h>

#include "libpropeller/c++allocate/c++allocate.h"
#include "libpropeller/pin/pin.h"

#include "libpropeller/serial/serial.h"
#include "libpropeller/i2c/i2c.h"
#include "libpropeller/max8819/max8819.h"
#include "libpropeller/numbers/numbers.h"
#include "libpropeller/scheduler/scheduler.h"
#include "librednine/concurrent_buffer/concurrent_buffer.h"
#include "librednine/concurrent_buffer/pib.h"
#include "libpropeller/eeprom/eeprom.h"
#include "libpropeller/stopwatch/stopwatch.h"

#include "rovingbluetooth.h"
#include "DatalogController.h"
#include "Sensors.h"
#include "UserInterface.h"





/* Pin definitions */
#ifdef GAMMA
#include "scadgamma.h"
#elif BETA2
#include "scadbeta2.h"
#endif


//TODO(SRLM): Make sure there isn't a problem with the delay between a command to DatalogController (such as start SD), and the time that getsdActive returns the new state.

//TODO(SRLM): Add tests to beginning to not log when battery voltage is too low.

//TODO(SRLM): check pointers for null, and so on (be safe!).

//TODO(SRLM): Numbers::Dec is not thread safe... Check!

//TODO(SRLM): Add some sort of detection for removal of charging during the loop...

/**

Cog Usage:
0: Main
1: GPS driver (ASM)
2: Sensors Controller cog
3: SD driver (ASM)
4: Serial driver (ASM)(Bluetooth)
5: Datalog Controller cog
6: Debug Serial
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

//TODO: Do any of these need to be volatile?

Max8819 pmic;
EEPROM eeprom;
Sensors sensors;
DatalogController dc;
UserInterface ui;

#ifdef DEBUG_PORT
Serial * debug = NULL;
#endif

Bluetooth * bluetooth = NULL;

/*
Status Variables
 */
volatile int unitNumber;
volatile int boardVersion;

int * datalogStack = NULL; //TODO(SRLM): can these be static?
int * sensorsStack = NULL;

//TODO(SRLM): for some reason it suddenly can't find _thread_state_t!
//const int stacksize = sizeof (_thread_state_t) + sizeof (int)*3 + sizeof (int)*100;
const int stacksize = 80 + sizeof (int)*3 + sizeof (int)*100;

enum LogLevel {
    kAll, kFatal, kError, kWarn, kInfo, kDebug
};
const char * LogLevelIdentifier[] = {"All:   ", "Fatal: ", "Error: ", "Warn:  ", "Info:  ", "Debug: "};

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

void DatalogCogRunner(void * parameter) {
    DatalogController * temp = (DatalogController *) parameter;
    temp->Server();
    waitcnt(CLKFREQ / 10 + CNT); //Let everything settle down
    cogstop(cogid());
}

void OutputGlobalLogHeaders(void) {
    LogVElement();
    sensors.AddScales();
}

void TurnSDOn(void) {
#ifdef DEBUG_PORT
    debug->Put("\r\nTurnSDOn()");
#endif
    if (dc.getsdMounted() == true) {
        if (dc.getsdActive() == false) {
            pmic.On(); //Make sure we don't lose power while datalogging!
            dc.SetClock(sensors.year + 2000, sensors.month, sensors.day,
                    sensors.hour, sensors.minute, sensors.second);
            dc.StartSD();
            dc.BlockUntilWaiting();
            eeprom.PutNumber(kEepromCanonNumberAddress, dc.GetLastFileNumber(), 4); //Store canonFilenumber for persistence
            OutputGlobalLogHeaders();
            Sensors::SetLogging(true);
            ui.SetState(UserInterface::kDatalogging);
        }
    } else {
        ui.SetState(UserInterface::kNoSD);
        LogStatusElement(kError, "SD Error!");
    }
}

void TurnSDOff(void) {
#ifdef DEBUG_PORT
    debug->Put("\r\nTurnSDOff()");
#endif    
    if (dc.getsdActive() == true) {
        Sensors::SetLogging(false);
        dc.StopSD();
    }
    
    ui.SetState(UserInterface::kWaiting);
}

void TransferFile(const char * filename) {
#ifdef DEBUG_PORT
    debug->Put("\r\nTransferFile()");
    debug->Put(" Filename: '");
    debug->Put(filename);
    debug->Put("'");
#endif

    
    Sensors::SetLogging(false);
    if (dc.getsdActive() == false) {
        //TODO output file Header here
        dc.InjectFile(filename);
        //TODO output file Closer here
    }
    Sensors::SetLogging(true);
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

    ui.SetState(UserInterface::kPowerOff);
    ui.DisplayState(sensors.fuel_soc);

    waitcnt(CLKFREQ + CNT);
    pmic.Off(); //TODO(SRLM): Make this set a global variable instead of hacking it...

    while (true) {
    }

    //TODO(SRLM): Add check here: if still on, we're plugged in.
}

void ParseBluetoothCommands(void) {
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


    ParseBluetoothCommands();

    if (ui.CheckButton() == false
            && ui.GetButtonPressDuration() > 100
            && ui.GetButtonPressDuration() < 1000) {
        ui.ClearButtonPressDuration();
        TurnSDOn();
    }
    
    if (ui.CheckButton() == true
            && ui.GetButtonPressDuration() > 3000) {
        KillSelf();
    }
    
    ui.DisplayState(sensors.fuel_soc);

}

void Init(void) {
    //Power
    pmic.Start(board::kPIN_MAX8819_CEN, board::kPIN_MAX8819_CHG,
            board::kPIN_MAX8819_EN123, board::kPIN_MAX8819_DLIM1,
            board::kPIN_MAX8819_DLIM2);
    //pmic.SetCharge(Max8819::HIGH); //TODO: There is some sort of bug where this *must* be in the code, otherwise it causes a reset.
    pmic.On();

    //LEDs and Button
#ifdef GAMMA
    ui.Init(board::kPIN_LEDW, board::kPIN_LEDR, board::kPIN_BUTTON);
#elif BETA2
    ui.Init(board::kPIN_LEDG, board::kPIN_LEDR, board::kPIN_BUTTON);
#endif
    
    ui.SetState(UserInterface::kWaiting);
    


    //EEPROM
    eeprom.Init();
    unitNumber = eeprom.GetNumber(kEepromUnitAddress, 4);
    boardVersion = eeprom.GetNumber(kEepromBoardAddress, 4);
#ifdef DEBUG_PORT
    debug->Put("\r\nEEPROM initialized.");
    debug->PutFormatted("\r\nUnit number: %i", unitNumber);
    debug->PutFormatted("\r\nBoard Version: %i", boardVersion);
#endif

    bluetooth = new Bluetooth(board::kPIN_BLUETOOTH_RX, board::kPIN_BLUETOOTH_TX,
            board::kPIN_BLUETOOTH_CTS, board::kPIN_BLUETOOTH_CONNECT);

#ifdef DEBUG_PORT
    debug->Put("\r\nBluetooth initialized.");
#endif    




    const int canonNumber = eeprom.GetNumber(kEepromCanonNumberAddress, 4);
    //Datalog Controller
    bool mounted = dc.Init(canonNumber, unitNumber, board::kPIN_SD_DO, board::kPIN_SD_CLK,
            board::kPIN_SD_DI, board::kPIN_SD_CS);
    if (mounted == false) {
        ui.SetState(UserInterface::kNoSD);
        LogStatusElement(kError, "SD Error at Init!");
    } else {
        datalogStack = (int *) malloc(stacksize);
        cogstart(DatalogCogRunner, &dc, datalogStack, stacksize);
    }

#ifdef DEBUG_PORT
    debug->Put("\r\nDatalog Cog initialized.");
#endif
    sensors.Start();
    sensors.SetLogging(false);

#ifdef DEBUG_PORT
    debug->Put("\r\nSensor Cog initialized.");
    debug->PutFormatted("\r\n\tFuel SOC: %i", sensors.fuel_soc);

#endif
}

int main(void) {

#ifdef DEBUG_PORT    
    debug = new Serial();
    debug->Start(31, 30, 460800);
    debug->Put("\r\nSCAD Debug Port: ");
#ifdef GAMMA
    debug->Put(" Gamma!");
#elif BETA2
    debug->Put(" Beta2!");
#endif
#endif


    Init();
    
    
    
    
    while (ui.CheckButton()) {
        waitcnt(CLKFREQ / 10 + CNT);
    } //Wait for the user to release the button, if turned on that way.
    waitcnt(CLKFREQ / 10 + CNT);

    if (pmic.GetPluggedIn() == true) {
        ui.SetState(UserInterface::kCharging);
        pmic.Off(); //Turn off in case it's unplugged while charging
    }
    
    
    while (true) {
        InnerLoop();
    }
}


