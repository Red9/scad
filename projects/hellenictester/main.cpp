#include <stdio.h>



#include <string.h>
#include "libpropeller/i2c/i2c.h"
#include "libpropeller/sd/sd.h"
#include "libpropeller/pcf8523/pcf8523.h"
#include "libpropeller/eeprom/eeprom.h"
#include "libpropeller/numbers/numbers.h"

#include "libpropeller/pin/pin.h"

/* Pin definitions */
#include "scad.h"
#ifdef GAMMA
#include "scadgamma.h"
#elif BETA2
#include "scadbeta2.h"
#endif







const unsigned short kEepromUnitAddress = 0xFFFC;
const unsigned short kEepromBoardAddress = 0xFFF8;
const unsigned short kEepromCanonNumberAddress = 0xFFF4;

const int kBoardAlpha = 0x0000000A;
const int kBoardBeta = 0x0000000B;
const int kBoardBeta2 = 0x000000B2;
const int kBoardGamma = 0x00000004;

// ----------------------------------------------------------------------------
// Configuring
// ----------------------------------------------------------------------------

int GetBoardVersion(int oldVersion) {
    printf("Please select the board version: \n");
    printf(" * - 0x%08x (current version)\n", oldVersion);
    printf(" a - Alpha\n");
    printf(" b - Beta\n");
    printf(" c - Beta 2\n");
    printf(" d - Gamma\n");
    printf(" e - Other\n");
    printf(">>> ");
    char choice = getchar();
    getchar(); //Throw away enter
    int boardVersion;

    if (choice == 'a') {
        boardVersion = kBoardAlpha;
    } else if (choice == 'b') {
        boardVersion = kBoardBeta;
    } else if (choice == 'c') {
        boardVersion = kBoardBeta2;
    } else if (choice == 'd') {
        boardVersion = kBoardGamma;
    } else if (choice == '*') {
        boardVersion = oldVersion;
    } else {
        printf("Your choice '0x%08x' isn't supported right now. Keeping current version.\n", choice);
        boardVersion = oldVersion;
    }

    return boardVersion;
}

int GetUnitNumber(int oldNumber) {

    printf("Please enter the unit number or * to keep current number, then <enter>:\n>>> ");


    char buffer[13];
    for (int i = 0;; ++i) {
        buffer[i] = getchar();
        if (buffer[i] == '\n') {
            buffer[i] = 0;
            putchar('\n');
            break;
        }
    }
    int unitNumber;
    if (buffer[0] == '*') {
        unitNumber = oldNumber;
    } else {
        unitNumber = libpropeller::Numbers::Dec(buffer);
    }

    return unitNumber;


}

int GetCanonNumber(int oldNumber) {
    printf("Please enter the canon number, or * to keep current number, then <enter>:\n>>> ");

    char buffer[13];
    for (int i = 0;; ++i) {
        buffer[i] = getchar();
        if (buffer[i] == '\n') {
            buffer[i] = 0;
            putchar('\n');
            break;
        }
    }
    int canonNumber;
    if (buffer[0] == '*') {
        canonNumber = oldNumber;
    } else {
        canonNumber = libpropeller::Numbers::Dec(buffer);
    }

    return canonNumber;
}

int GetTwoDigits(const char * caption) {

    char num[3];
    printf(caption);
    num[0] = (char) getchar();
    num[1] = (char) getchar();
    num[2] = 0;
    getchar(); //Throw away enter
    return libpropeller::Numbers::Dec(num);
}

void SetTime(libpropeller::PCF8523 * rtc) {
    printf("Set time?\n(y)es\n*no\n>>> ");

    char choice = getchar();
    getchar(); //Throw away enter
    printf("%c\n", choice);

    if (choice != 'y') {
        return;
    }



    int year, month, day, hour, minute, second;

    printf("Type all numbers as 2 digits (pad with zeros if needed).\n");

    year = GetTwoDigits("Year: 20");
    month = GetTwoDigits("Month: ");
    day = GetTwoDigits("Day: ");
    hour = GetTwoDigits("Hour (24h format): ");
    minute = GetTwoDigits("Minute: ");
    second = GetTwoDigits("Second: ");

    printf("\nSelected 20%d-%d-%d at %d:%d:%d\n", year, month, day, hour, minute, second);

    rtc->SetClock(year, month, day, hour, minute, second);

}




// ----------------------------------------------------------------------------
// Testing
// ----------------------------------------------------------------------------

bool IsSpecialAddress(unsigned char address) {
    //	Ignore special addresses: http://www.i2c-bus.org/addressing/
    unsigned char special [] = {
        0b0000000, 0b0000001, 0b0000010, 0b0000011, //Assorted addresses
        0b0000100, 0b0000101, 0b0000110, 0b0000111, //High speed master code
        0b1111000, 0b1111001, 0b1111010, 0b1111011, //10-bit slave addressing
        0b1111100, 0b1111101, 0b1111110, 0b1111111
    }; //Reserved for future purposes

    int specialSize = 16;

    for (int i = 0; i < specialSize; ++i) {
        if (address == special[i]) {
            return true;
        }
    }
    return false;
}

bool IsKnownAddress(unsigned char address, char * buffer) {
    if (address == 0b0011001) {
        strcpy(buffer, "LSM303DLHC Acclerometer");
        return true;
    }
    if (address == 0b1101011) {
        strcpy(buffer, "L3GD20 Gyroscope");
        return true;
    }
    if (address == 0b0011110) {
        strcpy(buffer, "LSM303DLHC Magnetometer");
        return true;
    }
    if (address == 0b1110111) {
        strcpy(buffer, "MS5611 Barometer");
        return true;
    }
    if (address == 0b0110110) {
        strcpy(buffer, "MAX17048G Fuel Gauge");
        return true;
    }
    if (address == 0b1101000) {
        strcpy(buffer, "PCF8523 Real Time Clock");
        return true;
    }
    if (address == 0b1010000) {
        strcpy(buffer, "24LC512 EEPROM");
        return true;
    }


    return false;
}

/**
 * 
 * @param SCL
 * @param SDA
 * @return    The number of known devices found.
 */
int ScanBus(const int SCL, const int SDA) {
    libpropeller::I2C bus;
    bus.Init(SCL, SDA);

    int knownDevices = 0;

    char buffer[80];

    for (unsigned char address = 0; address < 128; address++) {
        if (IsSpecialAddress(address) == false) {
            //Ping bus
            if (bus.Ping(address << 1)) {
                if (IsKnownAddress(address, buffer)) {
                    printf("0x%X - Found ", address << 1);
                    printf(buffer);
                    printf("\n");
                    knownDevices += 1;
                } else {
                    printf("0x%X - Found [Unknown]\n", address << 1);
                }
            } else {
                //printf("0x%X - \n", address << 1);
            }
        } else {
            //printf("0x%X - ***\n", address << 1);
        }
    }
    return knownDevices;
}

bool ScanBusRunner(const int SCL, const int SDA, const int deviceCount, const char busName[]) {
    printf("Scanning i2c bus %s for %i devices\n", busName, deviceCount);
    int foundDevices = ScanBus(SCL, SDA);
    if (foundDevices != deviceCount) {
        printf("ERROR: Did not find all devices on bus %s\n", busName);
        return false;
    } else {
        printf("\n");
        return true;
    }

}

bool TestSD(const int Do, const int Clk, const int Di, const int Cs) {
    libpropeller::SD sd;
    sd.Mount(Do, Clk, Di, Cs);
    if (sd.HasError()) {
        printf("ERROR: SD could not mount. Error code %i\n", sd.GetError());
        return false;
    }


    const char tempSentence[] = "One word too many.";
    const char filename[] = "scadtest.txt";
    char buffer[strlen(tempSentence) + 1];

    sd.Open(filename, 'w');
    sd.Put(tempSentence);
    sd.Open(filename, 'r');
    sd.Get(buffer, strlen(tempSentence));
    buffer[strlen(tempSentence)] = '\0';
    sd.Open(filename, 'd');

    if (strcmp(tempSentence, buffer) != 0) {
        printf("ERROR: SD card write and read to file failed.\n");
        return false;
    }


    if (sd.HasError()) {
        printf("ERROR: SD could not write, read, or delete. Error code %i\n", sd.GetError());
        return false;
    }

    printf("SD card passed.\n");

    return true;
}

bool TestSQWDLIMcut() {
#ifdef GAMMA
    int mask = 1 << board::kPIN_MAX8819_DLIM2;
    DIRA &= !mask;

    for (int i = 0; i < 1000; i++) {
        if ((INA & mask) != 0) {
            printf("ERROR: SQW and DLIM not cut!\n");
            return false;
        }
        waitcnt(CLKFREQ / 1000 + CNT);
    }

    printf("SQW successfully cut.\n");
    return true;


#elif BETA2
    return true;
#endif
}

bool TestButton(int pin){
    libpropeller::Pin button = libpropeller::Pin(pin);
    
    if(button.input() == 1){
        return false;
    }
    printf("Press button now");
    
    int counter = 0;
    while(button.input() == 0 && counter < 1000){
        waitcnt(CLKFREQ/100 + CNT);
        counter++;
    }
    
    if(button.input() == 1){
        return true;
    }else{
        printf("ERROR: Button not pressed.");
        return false;
    }
}

void configureBoard() {
    printf("Let's configure!\n");

    libpropeller::EEPROM eeprom;
    eeprom.Init();

    int boardVersion = 0;
    int unitNumber = 0;
    int canonNumber = 0;
    
    int timeZoneSign = 0;
    int timeZoneHours = 0;
    int timeZoneMinutes = 0;

    libpropeller::I2C rtcBus;

#ifdef GAMMA
    rtcBus.Init(board::kPIN_I2C_SCL_2, board::kPIN_I2C_SDA_2);
#elif BETA2
    rtcBus.Init(board::kPIN_I2C_SCL, board::kPIN_I2C_SDA);
#endif


    libpropeller::PCF8523 rtc;
    rtc.Init(&rtcBus);

    for (;;) {


        unitNumber = eeprom.GetNumber(kEepromUnitAddress, 4);
        boardVersion = eeprom.GetNumber(kEepromBoardAddress, 4);
        canonNumber = eeprom.GetNumber(kEepromCanonNumberAddress, 4);
        
        timeZoneSign = eeprom.GetNumber(board::kEepromTimeZoneSign, 4);
        timeZoneHours = eeprom.GetNumber(board::kEepromTimeZoneHours, 4);
        timeZoneMinutes = eeprom.GetNumber(board::kEepromTimeZoneMinutes, 4);

        int year, month, day, hour, minute, second;
        year = month = day = hour = minute = second = 0;
        rtc.GetClock(year, month, day, hour, minute, second);


        printf("\n--------------------------\n\n");
        printf("Current unit number:   %d\n", unitNumber);
        printf("Current board version: 0x%x\n", boardVersion);
        printf("Current Canon file:    %d\n", canonNumber);
        printf("Current RTC clock is: 20%d-%d-%d at %d:%d:%d\n", year, month, day, hour, minute, second);
        printf("Current timezone is: %c%d%d", timeZoneSign, timeZoneHours, timeZoneMinutes);
        printf("\n--------------------------\n\n");


        boardVersion = GetBoardVersion(boardVersion);
        eeprom.PutNumber(kEepromBoardAddress, boardVersion, 4);

        unitNumber = GetUnitNumber(unitNumber);
        eeprom.PutNumber(kEepromUnitAddress, unitNumber, 4);

        canonNumber = GetCanonNumber(canonNumber);
        eeprom.PutNumber(kEepromCanonNumberAddress, canonNumber, 4);
        
        SetTime(&rtc);
    }

}



int main(void) {


    printf("\nSCAD Tester.\n");

    bool pass = true;

#ifdef GAMMA
    pass = pass && ScanBusRunner(board::kPIN_EEPROM_SCL, board::kPIN_EEPROM_SDA, 1, "EEPROM");
    pass = pass && ScanBusRunner(board::kPIN_I2C_SCL_1, board::kPIN_I2C_SDA_1, 3, "1");
    pass = pass && ScanBusRunner(board::kPIN_I2C_SCL_2, board::kPIN_I2C_SDA_2, 3, "2");


#elif BETA2
    pass = pass && ScanBusRunner(board::kPIN_EEPROM_SCL, board::kPIN_EEPROM_SDA, 1, "EEPROM");
    pass = pass && ScanBusRunner(board::kPIN_I2C_SCL, board::kPIN_I2C_SDA, 6, "main");
#endif
    pass = pass && TestSQWDLIMcut();

    pass = pass && TestSD(board::kPIN_SD_DO, board::kPIN_SD_CLK, board::kPIN_SD_DI, board::kPIN_SD_CS);

    pass = pass && TestButton(board::kPIN_BUTTON);
    
    if (pass) {
#ifdef GAMMA
        libpropeller::Pin led = libpropeller::Pin(board::kPIN_LEDW);
        led.high();
#endif

        printf("\nOK\n");

        printf("\nConfigure (y/n)?");
        int choice = getchar();
        getchar(); //throw away enter
        if (choice == 'y') {
            configureBoard();
        }


    } else {
#ifdef GAMMA
        libpropeller::Pin led = libpropeller::Pin(board::kPIN_LEDR);
        led.high();
#endif
        printf("\nFAIL FAIL FAIL\n");
    }

    putchar(0xff);
    putchar(0x00);
    putchar(0);

}

