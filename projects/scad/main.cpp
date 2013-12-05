// -----------------------------------------------------------------------------
// Board configuration
// ------------------------------------------------------------------------------
//#define DEBUG_PORT


// ------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------
#include <string.h>
#include <ctype.h>
#include <propeller.h>
#include <stdlib.h>
#include <limits.h>

#include "libpropeller/serial/serial.h"
#include "libpropeller/max8819/max8819.h"
#include "libpropeller/eeprom/eeprom.h"

#include "rovingbluetooth.h"
#include "DatalogController.h"
#include "Sensors.h"
#include "UserInterface.h"

#include "scad.h"
/* Pin definitions */
#ifdef GAMMA
#include "scadgamma.h"
#elif BETA2
#include "scadbeta2.h"
#endif

//TODO(SRLM): Add tests to beginning to not log when battery voltage is too low.

//TODO(SRLM): Numbers::Dec is not thread safe... Check!

//TODO(SRLM): Add some sort of detection for removal of charging during the loop...

/**

Cog Usage:
0: Main
1: GPS driver (ASM)
2: Sensors Controller cog
3: SD driver (ASM)
4: Serial driver (ASM)(Bluetooth)
5: Datalog Controller cog (when logging)
6: Debug Serial
7:
 */

//TODO: Do any of these need to be volatile?

Max8819 pmic;
EEPROM eeprom;
DatalogController dc;
UserInterface ui;
Bluetooth bluetooth;

#ifdef DEBUG_PORT
Serial debug;
#endif

//TODO(SRLM): for some reason it suddenly can't find _thread_state_t!
//const int stacksize = sizeof (_thread_state_t) + sizeof (int)*3 + sizeof (int)*100;
const int stacksize = 80 + sizeof (int)*3 + sizeof (int)*141;

int datalogStack[stacksize];

void DatalogCogRunner(void * parameter) {
    DatalogController::Configuration * temp = (DatalogController::Configuration *) parameter;
    dc.RecordFile(*temp);
    waitcnt(CLKFREQ / 10 + CNT); //Let everything settle down
    cogstop(cogid());
}

void BeginRecording(void) {
#ifdef DEBUG_PORT
    debug.Put("\r\nTurnSDOn()");
#endif
    if (dc.DiskReady() == true && dc.IsRecording() == false) {
        pmic.On(); //Make sure we don't lose power while datalogging!

        int unitNumber = eeprom.GetNumber(board::kEepromUnitAddress, 4);
        int boardVersion = eeprom.GetNumber(board::kEepromBoardAddress, 4);

        char timeZoneSign = eeprom.GetNumber(board::kEepromTimeZoneSign, 4);
        int timeZoneHours = eeprom.GetNumber(board::kEepromTimeZoneHours, 4);
        int timeZoneMinutes = eeprom.GetNumber(board::kEepromTimeZoneMinutes, 4);

        int canonNumber = eeprom.GetNumber(board::kEepromCanonNumberAddress, 4);

        DatalogController::Configuration config;
        config.bitsPerTimestamp = 32;
        config.compileDate = __DATE__;
        config.compileTime = __TIME__;
        config.canonNumber = canonNumber;
        config.sensorJSON = Sensors::GetJSON();
        config.timezoneSign = timeZoneSign;
        config.timezoneHours = timeZoneHours;
        config.timezoneMinutes = timeZoneMinutes;
        config.unitNumber = unitNumber;
        config.boardVersion = boardVersion;

        config.timestamp = CNT;

        config.year = Sensors::year + 2000;
        config.month = Sensors::month;
        config.day = Sensors::day;
        config.hour = Sensors::hour;
        config.minute = Sensors::minute;
        config.second = Sensors::second;

        cogstart(DatalogCogRunner, &config, datalogStack, stacksize);
        waitcnt(8000 * 2 + CNT); // Wait for cog to start.

        if (dc.WaitUntilRecordingReady() == true) {
            eeprom.PutNumber(board::kEepromCanonNumberAddress, config.canonNumber, 4); //Store canonFilenumber for persistence

            Sensors::AddScales();
            Sensors::SetLogging(true);
            ui.SetState(UserInterface::kDatalogging);
        } else {
            ui.SetState(UserInterface::kNoSD);
#ifdef DEBUG_PORT
            debug.Put("Recording start failed");
#endif
        }

    } else {
        ui.SetState(UserInterface::kNoSD);
#ifdef DEBUG_PORT
        debug.Put("Could not turn SDOn()");
#endif
    }

}

void EndRecording(void) {
    if (dc.IsRecording() == true) {
#ifdef DEBUG_PORT
        debug.Put("\r\nEndRecording()");
#endif    
        Sensors::SetLogging(false);
        waitcnt(CLKFREQ / 10 + CNT); // Give everything some time to settle
        dc.StopRecording();

        ui.SetState(UserInterface::kWaiting);
    }
}

void KillSelf(bool doNotReboot = true) {
#ifdef DEBUG_PORT
    debug.Put("\r\nKillSelf()");
#endif    
    EndRecording();

    ui.SetState(UserInterface::kPowerOff);
    ui.UpdateDisplayState(Sensors::fuel_soc);

    waitcnt(CLKFREQ / 4 + CNT);

    if (doNotReboot == true) {
        pmic.Off();
    } else {

    }

    //TODO(SRLM): Add check here: if still on, we're plugged in.
    while (true) {
    }
}


const int kBUFFER_NO_VALUE = -1;

char command[board::kMAX_SIZE_COMMAND + 1]; // +1 for /0
int commandIndex = 0;
bool commandComplete = false;
char parameter[board::kMAX_SIZE_PARAMETER + 1];
int parameterIndex = 0;

void BluetoothError(const char * const parameter) {
#ifdef DEBUG_PORT
    debug.PutFormatted("Error parsing bluetooth: '%s'\r\n", parameter);
#endif
}

/**
 * 
 * @param buffer
 * @param index
 * @param kMAX_SIZE
 * @return true if word is complete, false otherwise.
 */
bool ReadWordIntoBuffer(char * const buffer, int & index, const int kMAX_SIZE) {
    bool result = false;

    int c = bluetooth.Get(0);
    if (c != -1) {
        if (isspace(c) != 0) {
            c = '\0';
            result = true;
        }
        buffer[index] = (char) c;
        index++;

        // Are we at the end of our buffer? If we are, then we need to return true.
        if (index == kMAX_SIZE) {
            result = true;
        }
    }

    if (result == true) { // Reset for next time.
        index = 0;
    }
    return result;
}

void ListFiles(const char * const parameter) {
    if (dc.ListFiles() == false) {
        //BluetoothError("SD not available.");
    }
}

bool CopyFile(const char * const filename) {
    bool successFlag = false;
    if (dc.TransferFile(filename) != true) {
        //BluetoothError("CopyFile failed");
        successFlag = true;
    }
    return successFlag;
}

void DeleteFile(const char * const filename) {
    dc.DeleteFile(filename);
}

void MoveFile(const char * const filename) {
    if (CopyFile(filename) == true) {
        // Only delete if we've successfully copied the file.
        DeleteFile(filename);
    }
}

/** Breaks a sting into two parts. Replaces the delimiter with \0
 * 
 * @param parameter A string of format outnehotnu=otuhaentsha (string + '=' + string)
 * @return the second string if found, NULL if not found.
 */
char * BreakOnDelimiter(char * parameter) {
    char * value = strchr(parameter, '=');
    if (value == NULL) {
        return NULL;
    } else {
        value[0] = '\0';
        return value + 1;
    }
}

void ParseStoredVariable(const bool echo, const short kEepromAddress, const char * userSpecifiedValue) {
#ifdef DEBUG_PORT
    debug.Put("\r\nStored variable...");
#endif
    if (echo == true) {
        int value = eeprom.GetNumber(kEepromAddress, 4);
        bluetooth.Put(Numbers::Dec(value));
    } else {
        int value = Numbers::Dec(userSpecifiedValue);
        if (value != INT_MIN) {
            eeprom.PutNumber(kEepromAddress, value, 4);
        }
    }
}

void ParseStoredTimezone(const bool echo, const char * userSpecifiedValue) {
    if (echo == true) {
        char timeZoneSign = eeprom.GetNumber(board::kEepromTimeZoneSign, 4);
        int timeZoneHours = eeprom.GetNumber(board::kEepromTimeZoneHours, 4);
        int timeZoneMinutes = eeprom.GetNumber(board::kEepromTimeZoneMinutes, 4);
        bluetooth.PutFormatted("%c%02i%02i", timeZoneSign, timeZoneHours, timeZoneMinutes);
    } else {
        if (strlen(userSpecifiedValue) != 5
                || (userSpecifiedValue[0] != '+' && userSpecifiedValue[0] != '-')) {
#ifdef DEBUG_PORT
            debug.Put("Timezone preconditions not met.");
#endif
            return;
        }

        char sign = userSpecifiedValue[0];
        int minutes = Numbers::Dec(userSpecifiedValue + 3);

        const char hourBuffer[3] = {userSpecifiedValue[1], userSpecifiedValue[2], '\0'};
        int hours = Numbers::Dec(hourBuffer);



        if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
#ifdef DEBUG_PORT
            debug.PutFormatted("Got timezone: '%c%02i%02i'", sign, hours, minutes);
#endif
            eeprom.PutNumber(board::kEepromTimeZoneSign, sign, 4);
            eeprom.PutNumber(board::kEepromTimeZoneHours, hours, 4);
            eeprom.PutNumber(board::kEepromTimeZoneMinutes, minutes, 4);

        } else {
#ifdef DEBUG_PORT
            debug.Put("Timezone hours or minutes out of range.");
#endif
        }
    }
}

void ParseStoredTime(const bool echo, char * userSpecifiedValue) {
    // Value is specified as in the following example:
    // 0123456789012345678
    // 2013-12-03T19:14:17

    if (echo == true) {
        bluetooth.PutFormatted("%04i-%02i-%02iT%02i:%02i:%02i",
                Sensors::year + 2000, Sensors::day, Sensors::hour, Sensors::minute, Sensors::second);
    } else {
        if (strlen(userSpecifiedValue) != 19
                || userSpecifiedValue[4] != '-'
                || userSpecifiedValue[7] != '-'
                || userSpecifiedValue[10] != 'T'
                || userSpecifiedValue[13] != ':'
                || userSpecifiedValue[16] != ':') {
#ifdef DEBUG_PORT            
            debug.PutFormatted("\r\nTime preconditions not met: '%s'", userSpecifiedValue);
#endif
            return;
        }

        userSpecifiedValue[4] = '\0';
        userSpecifiedValue[7] = '\0';
        userSpecifiedValue[10] = '\0';
        userSpecifiedValue[13] = '\0';
        userSpecifiedValue[16] = '\0';

        int year = Numbers::Dec(userSpecifiedValue);
        int month = Numbers::Dec(userSpecifiedValue + 5);
        int day = Numbers::Dec(userSpecifiedValue + 8);
        int hour = Numbers::Dec(userSpecifiedValue + 11);
        int minute = Numbers::Dec(userSpecifiedValue + 14);
        int second = Numbers::Dec(userSpecifiedValue + 17);

        if (year < 0 || year > 9999
                || month < 0 || month > 12
                || day < 0 || day > 31
                || hour < 0 || hour > 23
                || minute < 0 || minute > 59
                || second < 0 || second > 59) {
#ifdef DEBUG_PORT
            debug.Put("\r\nCould not parse given time (out of range");
#endif

        } else {
            Sensors::SetClock(year, month, day, hour, minute, second);
        }
    }
}

void ParseVariable(char * key) {
#ifdef DEBUG_PORT
    debug.Put("\r\nParseVariable()");
#endif

    char * value = BreakOnDelimiter(key);
    const bool echo = value == NULL;
#ifdef DEBUG_PORT
    debug.Put("Finding key...");
#endif
    if (strcasecmp(key, "$recording") == 0) {
        if (echo == true) {
            if (dc.IsRecording() == true) {
                bluetooth.Put("true");
            } else {
                bluetooth.Put("false");
            }
        } else {
            if (strcasecmp(value, "true") == 0) {
                BeginRecording();
            } else {
                EndRecording();
            }
        }

    } else if (strcasecmp(key, "$recording_destination") == 0) {
        if (echo == true) {
            bluetooth.Put("sd");
        } else {

        }
    } else if (strcasecmp(key, "$timezone") == 0) {
        ParseStoredTimezone(echo, value);
    } else if (strcasecmp(key, "$hardware_version") == 0) {
        ParseStoredVariable(echo, board::kEepromBoardAddress, value);
    } else if (strcasecmp(key, "$unit_number") == 0) {
        ParseStoredVariable(echo, board::kEepromUnitAddress, value);
    } else if (strcasecmp(key, "$canon_number") == 0) {
        ParseStoredVariable(echo, board::kEepromCanonNumberAddress, value);
    } else if (strcasecmp(key, "$date_time") == 0) {
        ParseStoredTime(echo, value);
    } else {
        //BluetoothError("Echo error: can't find key.");
    }
}

void ParseCommand(char * command, char * parameter) {
#ifdef DEBUG_PORT
    debug.Put("\r\nParseCommand: ");
    debug.Put(command);
    debug.Put(", parameter: ");
    debug.Put(parameter);
#endif
    if (strcasecmp(command, "ls") == 0) {
        ListFiles(parameter);
    } else if (strcasecmp(command, "cp") == 0) {
        CopyFile(parameter);
    } else if (strcasecmp(command, "rm") == 0) {
        DeleteFile(parameter);
    } else if (strcasecmp(command, "mv") == 0) {
        MoveFile(parameter);
    } else if (strcasecmp(command, "export") == 0) {
        ParseVariable(parameter);
    } else if (strcasecmp(command, "echo") == 0) {
        ParseVariable(parameter);
    } else if (strcasecmp(command, "shutdown") == 0) {
        KillSelf();
    } else if (strcasecmp(command, "reboot") == 0) {

    } else {
        //BluetoothError(command);
    }
}

void ParseBluetoothCommands(void) {

    if (commandComplete == false) {
        commandComplete = ReadWordIntoBuffer(command, commandIndex, board::kMAX_SIZE_COMMAND);
    } else {
        if (ReadWordIntoBuffer(parameter, parameterIndex, board::kMAX_SIZE_PARAMETER) == true) {
            bluetooth.Put("<");
            ParseCommand(command, parameter);
            commandComplete = false;
            bluetooth.Put(">\r\n");
        }
    }
}

bool MountSD() {
    //Datalog Controller
    bool mounted = dc.Init(board::kPIN_SD_DO, board::kPIN_SD_CLK,
            board::kPIN_SD_DI, board::kPIN_SD_CS);
    if (mounted == false) {
        ui.SetState(UserInterface::kNoSD);
    } else {
#ifdef DEBUG_PORT
        debug.Put("\r\nDatalog Cog initialized.");
#endif
    }
    return mounted;
}

void InnerLoop(void) {

    // Test: Battery is too low?
    if (Sensors::fuel_voltage < 3500
            and Sensors::fuel_voltage != Sensors::kDefaultFuelVoltage) { //Dropout of 150mV@300mA, with some buffer
#ifdef DEBUG_PORT
        debug.Put("\r\nWarning: Low fuel!!! Turning off.");
#endif
        KillSelf();
    }


    ParseBluetoothCommands();

    if (ui.CheckButton() == false
            && ui.GetButtonPressDuration() > 150
            && ui.GetButtonPressDuration() < 1000) {
        ui.ClearButtonPressDuration();
        if (MountSD() == true) {
            BeginRecording();
        }
    }

    if (ui.CheckButton() == true
            && ui.GetButtonPressDuration() > 3000) {
        KillSelf();
    }

    ui.UpdateDisplayState(Sensors::fuel_soc);
}

void Init(void) {
    //Power
    pmic.Start(board::kPIN_MAX8819_CEN, board::kPIN_MAX8819_CHG,
            board::kPIN_MAX8819_EN123, board::kPIN_MAX8819_DLIM1,
            board::kPIN_MAX8819_DLIM2);
    pmic.On(); // We'll turn it off later if we need to.

    //LEDs and Button
#ifdef GAMMA
    ui.Init(board::kPIN_LEDW, board::kPIN_LEDR, board::kPIN_BUTTON);
#elif BETA2
    ui.Init(board::kPIN_LEDG, board::kPIN_LEDR, board::kPIN_BUTTON);
#endif

    ui.SetState(UserInterface::kWaiting);

    //EEPROM
    eeprom.Init();

#ifdef DEBUG_PORT
    int unitNumber = eeprom.GetNumber(board::kEepromUnitAddress, 4);
    int boardVersion = eeprom.GetNumber(board::kEepromBoardAddress, 4);

    debug.Put("\r\nEEPROM initialized.");
    debug.PutFormatted("\r\nUnit number: %i", unitNumber);
    debug.PutFormatted("\r\nBoard Version: %i", boardVersion);
#endif

    bluetooth.Start(board::kPIN_BLUETOOTH_RX, board::kPIN_BLUETOOTH_TX,
            board::kPIN_BLUETOOTH_CTS, board::kPIN_BLUETOOTH_CONNECT);

#ifdef DEBUG_PORT
    debug.Put("\r\nBluetooth initialized.");
#endif    

    if (Sensors::Start() == false) {
        ui.SetState(UserInterface::kUnknownError);
#ifdef DEBUG_PORT
        debug.Put("\r\nSensor Cog FAILED TO INIT");
#endif
    } else {
        Sensors::SetLogging(false);
#ifdef DEBUG_PORT
        debug.Put("\r\nSensor Cog initialized.");
#endif       
    }

#ifdef DEBUG_PORT
    debug.PutFormatted("\r\n\tFuel SOC: %i", Sensors::fuel_soc);

#endif
}

int main(void) {

#ifdef DEBUG_PORT    
    debug.Start(31, 30, 460800);
    debug.Put("\r\nSCAD Debug Port: ");
#ifdef GAMMA
    debug.Put(" Gamma!");
#elif BETA2
    debug.Put(" Beta2!");
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
#ifdef DEBUG_PORT
        debug.Put("\r\nCharging.");
#endif
    }


    while (true) {
        InnerLoop();
    }
}






