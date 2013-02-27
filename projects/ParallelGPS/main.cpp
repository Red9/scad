#include <stdint.h>
#include <propeller.h>
#include "FFDS1.h"
#include "fsrw.h"
#include "GPSParser.h"
#include "Elum.h"
#include "Max8819.h"


FFDS1 debug;
fsrwSpin sdfat;
int32_t logged_lines = 0;


static const uint8_t FILENAME [] = "pgps";
static const uint8_t FILEEXT  [] = ".txt";

void NMEA_debug(int32_t str_ptr, uint_fast8_t num)
{
    debug.Str("GPS ");
    debug.Tx(num);
    debug.Tx(':');
    debug.Str(str_ptr);
}

void LogGPS(GPSParser* gps, uint_fast8_t num)
{
    for(int32_t str_ptr = gps->GetStr(); str_ptr != -1; str_ptr = gps->GetStr())
    {
        logged_lines++;
        NMEA_debug(str_ptr, num);

        sdfat.Pputs("GPS ");
        sdfat.Pputc(num);
        sdfat.Pputc(':');
        sdfat.Pputs(str_ptr);
    }
}


/**
  * Converts a decimal number to it's string representation.
  *
  * @param value      The 32 bit 2's complement number to convert.
  * @param strpointer The address to store the string representation.
  * @returns          The starting address of the string pointer.
  */
int32_t Dec(int32_t Value, int32_t Strpointer)
{
    int32_t	I, Digits, Strpointer_start;
    int32_t result = 0;
    Strpointer_start = Strpointer;
    if (Value < 0)
    {
        Value = (-Value);
        ((uint8_t *)Strpointer)[0] = '-';
        (Strpointer++);
    }
    I = 1000000000;
    {
        int32_t _idx__0001;
        _idx__0001 = 10;
        do
        {
            if (Value >= I)
            {
                ((uint8_t *)Strpointer)[0] = ((Value / I) + '0');
                (Strpointer++);
                Value = (Value % I);
                result = -1;
            }
            else
            {
                if ((result) || (I == 1))
                {
                    ((uint8_t *)Strpointer)[0] = '0';
                    (Strpointer++);
                }
            }
            I = (I / 10);
            _idx__0001 = (_idx__0001 + -1);
        }
        while (_idx__0001 >= 1);
    }
    ((uint8_t *)Strpointer)[0] = 0;
    return Strpointer_start;
}


/**
  * Scans the SD card and searches for filenames of the type "red9log#.csv",
  * where # is any size number. Actually, it uses the FILENAME and FILEEXT const
  * section variables for the filename. It starts scanning at filenumber 0, and
  * continues until it doesn't find a file of that number on the SD card. It
  * then opens the file for writing, and then this function returns.
  */
void OpenFile()
{
    uint8_t buff1[12];
    uint8_t buff2[4];
    for(int i = 0; i < 1000; i++)
    {
        buff1[0] = 0;
        buff1[0] = 0;
        strcat(buff1, FILENAME);
        strcat(buff1, Dec(i, buff2));
        strcat(buff1, FILEEXT);
        int32_t result = sdfat.Popen(buff1, 'r');
        if(result == -1) break;
    }
    sdfat.Pclose();
    sdfat.Popen(buff1, 'w');

    debug.Str("Opened file: ");
    debug.Str(buff1);
    debug.Str("\r\n");
}


int main()
{
    const int32_t CEN_pin = 15;
    const int32_t CHG_pin = 14;
    const int32_t EN_pin  = 6;
    const int32_t DLIM1_pin = 16;
    const int32_t DLIM2_pin = 17;
    Max8819 pmic(CEN_pin, CHG_pin, EN_pin, DLIM1_pin, DLIM2_pin);
    pmic.SetCharge(Max8819::MEDIUM);

    const int32_t ledr = 5;
    const int32_t ledg = 7;
    const int32_t button = 8;
    Elum elum(ledr, ledg, button);
    elum.On(Elum::GREEN);

    const int32_t debug_rxpin = 31;
    const int32_t debug_txpin = 30;
    const int32_t debug_baud  = 230400;
    debug.Start(debug_rxpin, debug_txpin, debug_baud);

    debug.Str("Parallel GPS Tests!\r\n");

    debug.Str("Starting FSRW\r\n");
    const int32_t SD_DO  = 10;
    const int32_t SD_CLK = 11;
    const int32_t SD_DI  = 12;
    const int32_t SD_CS  = 13;


    int32_t success = sdfat.Mount_explicit(SD_DO, SD_CLK, SD_DI, SD_CS);
    if(success != 0)
    {
        elum.On(Elum::RED);
        debug.Str("Could not mount SD card. Error: ");
        debug.Dec(success);
        for(;;);
    }

    debug.Str("Mounted SD card.\r\n");
    OpenFile();

    waitcnt(CLKFREQ*2 + CNT);

    //Onboard GPS
    const int32_t gps0_rxpin = 25;
    const int32_t gps0_txpin = 24;
    const int32_t gps0_baud = 9600;
    GPSParser gps0(gps0_rxpin, gps0_txpin, gps0_baud);


    //External PA6H GPS
    const int32_t gps1_rxpin = 19;
    const int32_t gps1_txpin = 18;
    const int32_t gps1_baud = 9600;
    GPSParser gps1(gps1_rxpin, gps1_txpin, gps1_baud);


    //LS23060 GPS
    const int32_t gps2_rxpin = 21;
    const int32_t gps2_txpin = 20;
    const int32_t gps2_baud = 57600;
    GPSParser gps2(gps2_rxpin, gps2_txpin, gps2_baud);


    //Venus GPS
    const int32_t gps3_rxpin = 22;
    const int32_t gps3_txpin = 23;
    const int32_t gps3_baud = 9600;
    GPSParser gps3(gps3_rxpin, gps3_txpin, gps3_baud);

    elum.Flash(Elum::RED, 1000, 100);


    for(;;)
    {

        LogGPS(&gps0, '0');
        LogGPS(&gps1, '1');
        LogGPS(&gps2, '2');
        LogGPS(&gps3, '3');
        if(elum.getButton() == 1)
        {
            int32_t i = 0;
            sdfat.Pclose();
            debug.Str("\r\nDone Recording.");
            debug.Str("\r\nLogged lines: ");
            debug.Dec(logged_lines);
            elum.Flash(Elum::GREEN, 1000, 750);
            for(;;)
            {
                if(elum.getButton() == 1) i ++;
                else i = 0;

                if(i > 4000)
                {
                     pmic.Off();
                     elum.On(Elum::RED);
                     for(;;);
                }

                waitcnt(CLKFREQ/1000 + CNT);
            }
        }
    }

    return 0;
}
