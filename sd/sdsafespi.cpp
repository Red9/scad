#include <propeller.h>
#include "sdsafespi.h"

#define YIELD() __asm__ volatile( "" ::: "memory" )
#define ERROR_CHECK {if(error != kNoError){return error;}} 

volatile unsigned char SdSafeSPI::dat[] = {
    0x2f, 0xf1, 0xbf, 0xa0, 0x31, 0xf3, 0xbf, 0xa0, 0x2b, 0xed, 0xbf, 0xa0, 0x01, 0x80, 0xfe, 0xa4,
    0xf0, 0x81, 0x3e, 0x08, 0xf1, 0x8b, 0xbe, 0xa0, 0xa4, 0x66, 0xfd, 0x5c, 0xf0, 0x81, 0xbe, 0x08,
    0x00, 0x80, 0x7e, 0xc3, 0x06, 0x00, 0x78, 0x5c, 0x72, 0x80, 0x7e, 0x86, 0x1d, 0x00, 0x68, 0x5c,
    0x77, 0x80, 0x7e, 0x86, 0x2d, 0x00, 0x68, 0x5c, 0x7a, 0x80, 0x7e, 0x86, 0x16, 0x00, 0x68, 0x5c,
    0x74, 0x80, 0x7e, 0x86, 0x2e, 0x6f, 0x2a, 0x08, 0x2d, 0x71, 0x2a, 0x08, 0x00, 0x80, 0xfe, 0xa0,
    0xf0, 0x81, 0x3e, 0x08, 0x06, 0x00, 0x7c, 0x5c, 0x7a, 0x82, 0xfe, 0xa0, 0x01, 0x76, 0xfe, 0xa4,
    0x01, 0x84, 0xfe, 0xa4, 0x35, 0x96, 0xfc, 0x5c, 0x41, 0x81, 0xbe, 0xa0, 0xf0, 0x81, 0x3e, 0x08,
    0x06, 0x00, 0x7c, 0x5c, 0x2e, 0x85, 0xbe, 0x08, 0x42, 0x7f, 0xbe, 0xa0, 0x01, 0x7e, 0xfe, 0x80,
    0x3b, 0x7f, 0x3e, 0x86, 0x72, 0x78, 0x6a, 0x86, 0x25, 0x00, 0x68, 0x5c, 0x72, 0x82, 0xfe, 0xa0,
    0x35, 0x96, 0xfc, 0x5c, 0x10, 0x78, 0xfd, 0x58, 0xb4, 0x8a, 0xfd, 0x5c, 0x41, 0x81, 0xbe, 0xa0,
    0xf0, 0x81, 0x3e, 0x08, 0x72, 0x82, 0xfe, 0xa0, 0x01, 0x84, 0xfe, 0x80, 0x35, 0x96, 0xfc, 0x5c,
    0x06, 0x00, 0x7c, 0x5c, 0x2e, 0x85, 0xbe, 0x08, 0x11, 0x78, 0xfd, 0x58, 0xb4, 0x8a, 0xfd, 0x5c,
    0x41, 0x81, 0xbe, 0xa0, 0xf0, 0x81, 0x3e, 0x08, 0x77, 0x82, 0xfe, 0xa0, 0x35, 0x96, 0xfc, 0x5c,
    0x06, 0x00, 0x7c, 0x5c, 0x42, 0x77, 0x3e, 0x86, 0x3c, 0x83, 0x2a, 0x86, 0x42, 0x00, 0x68, 0x5c,
    0x77, 0x78, 0x7e, 0x86, 0x5a, 0xc4, 0xe8, 0x5c, 0x72, 0x78, 0x7e, 0x86, 0x53, 0xac, 0xe8, 0x5c,
    0x77, 0x82, 0x7e, 0x86, 0x57, 0xb2, 0xe8, 0x5c, 0x72, 0x82, 0x7e, 0x86, 0x50, 0xa4, 0xe8, 0x5c,
    0x7a, 0x82, 0x7e, 0x86, 0x4c, 0x9e, 0xe8, 0x5c, 0x42, 0x77, 0xbe, 0xa0, 0x01, 0x76, 0xfe, 0x80,
    0x41, 0x79, 0xbe, 0xa0, 0x77, 0x82, 0x7e, 0x86, 0xf0, 0x48, 0xea, 0x5c, 0x72, 0x82, 0x7e, 0x86,
    0xc6, 0xde, 0xe9, 0x5c, 0x7a, 0x82, 0x7e, 0x86, 0x00, 0x82, 0xea, 0xa0, 0x00, 0x00, 0x7c, 0x5c,
    0x2a, 0xe9, 0xbf, 0x68, 0x8d, 0x46, 0xfd, 0x5c, 0x8d, 0x46, 0xfd, 0x5c, 0x00, 0x00, 0x7c, 0x5c,
    0xa4, 0x86, 0xfe, 0x58, 0x63, 0xf6, 0xfc, 0x5c, 0x00, 0x00, 0x7c, 0x5c, 0x98, 0x86, 0xfe, 0x58,
    0x63, 0xf6, 0xfc, 0x5c, 0x7c, 0x00, 0xfd, 0x5c, 0x00, 0x00, 0x7c, 0x5c, 0xb2, 0x86, 0xfe, 0x58,
    0x63, 0xf6, 0xfc, 0x5c, 0x00, 0x00, 0x7c, 0x5c, 0x7c, 0x00, 0xfd, 0x5c, 0x10, 0x7e, 0xfe, 0xa0,
    0x8d, 0x46, 0xfd, 0x5c, 0x5c, 0x7e, 0xfe, 0xe4, 0xfa, 0xf9, 0xff, 0x58, 0x81, 0x18, 0xfd, 0x5c,
    0x8d, 0x46, 0xfd, 0x5c, 0x7c, 0x00, 0xfd, 0x5c, 0x00, 0x00, 0x7c, 0x5c, 0x2b, 0xed, 0xbf, 0xa0,
    0x2a, 0xe9, 0xbf, 0x68, 0x2a, 0xe9, 0xbf, 0x64, 0x8d, 0x46, 0xfd, 0x5c, 0x43, 0xf9, 0xbf, 0xa0,
    0x81, 0x18, 0xfd, 0x5c, 0x42, 0xf9, 0xbf, 0xa0, 0x2c, 0xf9, 0xbf, 0x2c, 0x81, 0x18, 0xfd, 0x5c,
    0x01, 0xf8, 0xff, 0x24, 0x81, 0x18, 0xfd, 0x5c, 0x01, 0xf8, 0xff, 0x24, 0x81, 0x18, 0xfd, 0x5c,
    0x01, 0xf8, 0xff, 0x24, 0x81, 0x18, 0xfd, 0x5c, 0x8d, 0x46, 0xfd, 0x5c, 0x18, 0x86, 0xfe, 0x28,
    0x4c, 0x86, 0x7e, 0x86, 0x8d, 0x46, 0xe9, 0x5c, 0x09, 0x7e, 0xfe, 0xa0, 0x8d, 0x46, 0xfd, 0x5c,
    0x80, 0x7c, 0x7e, 0x63, 0x77, 0x7e, 0xf2, 0xe4, 0x3e, 0x83, 0x96, 0xa4, 0x00, 0x00, 0x7c, 0x5c,
    0x30, 0x7f, 0xbe, 0xa0, 0x8d, 0x46, 0xfd, 0x5c, 0xff, 0x7c, 0x7e, 0x86, 0x7d, 0x7e, 0xd6, 0xe4,
    0x00, 0x00, 0x7c, 0x5c, 0x29, 0xe9, 0xbf, 0x64, 0x00, 0xfa, 0xff, 0xa0, 0x80, 0xf6, 0xff, 0x58,
    0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x00, 0xf6, 0xff, 0xa0,
    0x00, 0x00, 0x7c, 0x5c, 0x01, 0xf8, 0xff, 0xa4, 0x00, 0x7c, 0xfe, 0xa0, 0xc0, 0xfa, 0xff, 0x58,
    0x40, 0xf6, 0xff, 0x58, 0xf2, 0x51, 0x3e, 0x61, 0x01, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61,
    0x01, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61, 0x01, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61,
    0x01, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61, 0x01, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61,
    0x01, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61, 0x01, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61,
    0x00, 0xf6, 0xff, 0xa0, 0x01, 0x7c, 0xfe, 0x34, 0x00, 0xf8, 0xff, 0xa0, 0x00, 0x00, 0x7c, 0x5c,
    0xf1, 0x7f, 0xbe, 0xa0, 0x3f, 0x73, 0xbe, 0x80, 0x45, 0x73, 0xbe, 0x84, 0x3f, 0x71, 0xbe, 0x80,
    0x45, 0x71, 0xbe, 0x84, 0x3f, 0x8b, 0xbe, 0xa0, 0x00, 0x7e, 0xfe, 0x08, 0x3f, 0x71, 0xbe, 0xe1,
    0x00, 0x6e, 0xfe, 0xc8, 0x3a, 0x73, 0x3e, 0x87, 0xb3, 0x00, 0x70, 0x5c, 0x7a, 0x82, 0xfe, 0xa0,
    0x01, 0x76, 0xfe, 0xa4, 0x01, 0x84, 0xfe, 0xa4, 0x35, 0x96, 0xfc, 0x5c, 0x00, 0x00, 0x7c, 0x5c,
    0x32, 0xf3, 0xbf, 0xa0, 0x01, 0xf6, 0xff, 0xa0, 0x2d, 0x89, 0xbe, 0x08, 0x04, 0x7a, 0xfe, 0xa0,
    0x46, 0x79, 0xfd, 0x54, 0x3f, 0x7f, 0xbe, 0x08, 0x20, 0x7e, 0xfe, 0xa0, 0x44, 0xfb, 0xbf, 0xa0,
    0xfd, 0x01, 0xbc, 0x08, 0x35, 0x79, 0xbd, 0x80, 0xbc, 0x7e, 0xfe, 0xe4, 0x36, 0x79, 0xbd, 0x84,
    0x04, 0x88, 0xfe, 0x80, 0xb9, 0x7a, 0xfe, 0xe4, 0x00, 0xf6, 0xff, 0xa0, 0x00, 0xfa, 0xff, 0xa0,
    0x31, 0xf3, 0xbf, 0xa0, 0x00, 0x00, 0x7c, 0x5c, 0x46, 0xcd, 0xfd, 0x54, 0x80, 0x7a, 0xfe, 0xa0,
    0x30, 0x7f, 0xbe, 0xa0, 0x8d, 0x46, 0xfd, 0x5c, 0xfe, 0x7c, 0x7e, 0x86, 0xc9, 0x7e, 0xd6, 0xe4,
    0x64, 0x82, 0xd6, 0xa4, 0xef, 0x00, 0x54, 0x5c, 0x01, 0xf8, 0xff, 0xa4, 0x80, 0x7a, 0xfe, 0xa0,
    0x04, 0x7e, 0xfe, 0xa0, 0xc0, 0xfa, 0xff, 0x58, 0x40, 0xf6, 0xff, 0x58, 0xf2, 0x51, 0x3e, 0x61,
    0x08, 0x7c, 0xfe, 0x34, 0xf2, 0x51, 0x3e, 0x61, 0x02, 0x7c, 0xfe, 0x70, 0xf2, 0x51, 0x3e, 0x61,
    0x04, 0x7c, 0xfe, 0x70, 0xf2, 0x51, 0x3e, 0x61, 0x08, 0x7c, 0xfe, 0x70, 0xf2, 0x51, 0x3e, 0x61,
    0x10, 0x7c, 0xfe, 0x70, 0xf2, 0x51, 0x3e, 0x61, 0x20, 0x7c, 0xfe, 0x70, 0xf2, 0x51, 0x3e, 0x61,
    0x40, 0x7c, 0xfe, 0x70, 0xf2, 0x51, 0x3e, 0x61, 0x00, 0xf6, 0xff, 0xa0, 0x80, 0x7c, 0xfe, 0x70,
    0xd2, 0x7e, 0xfe, 0xe4, 0x00, 0x7c, 0xfe, 0x3c, 0x3e, 0x01, 0xbc, 0xa0, 0x33, 0xcd, 0xbd, 0x80,
    0xd0, 0x7a, 0xfe, 0xe4, 0x00, 0xf8, 0xff, 0xa0, 0x8d, 0x46, 0xfd, 0x5c, 0x8d, 0x46, 0xfd, 0x5c,
    0x8d, 0x46, 0xfd, 0x5c, 0x00, 0x72, 0xfe, 0xa0, 0x00, 0x82, 0xfe, 0xa0, 0x00, 0x00, 0x7c, 0x5c,
    0x46, 0xed, 0xfd, 0x50, 0x80, 0x7a, 0xfe, 0xa0, 0x7c, 0x00, 0xfd, 0x5c, 0xf8, 0xf9, 0xff, 0x58,
    0x81, 0x18, 0xfd, 0x5c, 0x00, 0xfa, 0xff, 0xa0, 0x46, 0xf9, 0xbf, 0xa0, 0x01, 0xec, 0xfd, 0x80,
    0x18, 0xf8, 0xff, 0x24, 0x80, 0xf6, 0xff, 0x58, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x11, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x11, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x11, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24, 0x01, 0xf8, 0xff, 0x24,
    0x01, 0xf8, 0xff, 0x24, 0x00, 0xf6, 0xff, 0xa0, 0xf6, 0x7a, 0xfe, 0xe4, 0x8d, 0x46, 0xfd, 0x5c,
    0x8d, 0x46, 0xfd, 0x5c, 0x8d, 0x46, 0xfd, 0x5c, 0x1f, 0x7c, 0xfe, 0x60, 0x05, 0x7c, 0x7e, 0x86,
    0x00, 0x82, 0xea, 0xa0, 0x65, 0x82, 0xd6, 0xa4, 0x8d, 0x46, 0xfd, 0x5c, 0x00, 0x72, 0xfe, 0xa0,
    0x00, 0x00, 0x7c, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x42, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00,
};

int SdSafeSPI::Start(int basepin) {
    return Start(basepin, (basepin + 1), (basepin + 2), (basepin + 3));
}

void SdSafeSPI::ReadBlock(int block_index, char * buffer_address) {
    if (spi_engine_cog == 0) {
        Crash(kErrorSpiEngineNotRunning);
        return;
    }
    if ((int) buffer_address & 0x3) {
        Crash(kErrorBlockNotLongAligned);
        return;
    }
    spi_block_index = block_index;
    spi_buffer_address = buffer_address;
    spi_command = 'r';
    while (spi_command == 'r') {
        YIELD();
    }
    if (spi_command < 0) {
        Crash(spi_command);
    }
}

void SdSafeSPI::WriteBlock(int block_index, char * buffer_address) {
    if (spi_engine_cog == 0) {
        Crash(kErrorSpiEngineNotRunning);
        return;
    }
    if ((int) buffer_address & 0x3) {
        Crash(kErrorBlockNotLongAligned);
        return;
    }
    spi_block_index = block_index;
    spi_buffer_address = buffer_address;
    spi_command = 'w';
    while (spi_command == 'w') {
        YIELD();
    }
    
    if (spi_command < 0) {
        Crash(spi_command);
    }
}



int SdSafeSPI::Start(int pin_do, int pin_clk, int pin_di, int pin_cs) {

    // Do all of the card initialization in C++, then hand off the pin
    // information to the assembly cog for hot SPI block R/W action!
    error = kNoError;

    Stop();
    waitcnt(500 + CLKFREQ * 4 / 1000 + CNT);

    mask_do = 1 << pin_do;
    mask_di = 1 << pin_di;
    mask_cs = 1 << pin_cs;
    mask_clk = 1 << pin_clk;
    mask_all = mask_cs | (1 << pin_clk) | mask_di;

    ((int *) & dat[1172])[0] = pin_do; // pinDo
    ((int *) & dat[1184])[0] = mask_do; // maskDo
    ((int *) & dat[1176])[0] = pin_clk; //pinClk
    ((int *) & dat[1180])[0] = pin_di; //pinDI
    ((int *) & dat[1188])[0] = mask_di; //maskDI
    ((int *) & dat[1192])[0] = mask_cs; //maskCS
    ((int *) & dat[1200])[0] = 9; //adrShift //block = 512 * index, and 512 = 1 << 9

    ((int *) & dat[1196])[0] = mask_all; // pass the output pin mask via the command register
    DIRA |= mask_all;

    // get the card in a ready state: set DI and CS high, send => 74 clocks
    OUTA |= mask_all;
    for (int i = 0; i < 4096; i++) {
        OUTA |= 1 << pin_clk;
        OUTA &= ~(1 << pin_clk);
    }

    spi_block_index = CNT; //Time hack

    int TmpA = 0;
    for (int i = 0; i < 10; i++) {
        if (TmpA != 1) {
            TmpA = SendCommandSlow(Cmd0, 0, 0x95);
            if (TmpA & 4) {
                // the card said CMD0 ("go idle") was invalid, so we're possibly stuck in read or write mode
                if (i & 1) {
                    // exit multiblock read mode

                    for (int jk = 0; jk < 4; jk++) {
                        ReadSlow32(); // these extra clocks are required for some MMC cards
                        ERROR_CHECK;
                    }
                    SendSlow(0xFD, 8); // stop token
                    ReadSlow32();
                    ERROR_CHECK;
                    
                    while (ReadSlow() != 0xFF) {
                        ERROR_CHECK;
                    }
                    ERROR_CHECK;
                } else {
                    //  exit multiblock read mode
                    SendCommandSlow(Cmd12, 0, 0x61);
                }
            }
        }

    }
    if (TmpA != 1) {
        //  the reset command failed!
        Crash(kErrorCardNotReset);
        return 0;
    }

    int card_type = 0;
    if (SendCommandSlow(Cmd8, 426, 135) == 1) { // Is this a SD type 2 card?
        //  Type2 SD, check to see if it's a SDHC card

        if ((ReadSlow32() & 0x1ff) != 0x1AA) { //check the supported voltage
            ERROR_CHECK;
            Crash(kError3v3NotSupported);
            return 0;
        }
        ERROR_CHECK; // For the previous ReadSlow32

        //  try to initialize the type 2 card with the High Capacity bit
        while (SendCommandSlow(Acmd41, ((1 << 30)), 0x77)) {
            YIELD();
        }

        // the card is initialized, let's read back the High Capacity bit
        if (SendCommandSlow(Cmd58, 0, 0xFD) != 0) {
            Crash(kErrorOcrFailed);
            return 0;
        }


        //  get back the data
        if (ReadSlow32() & (1 << 30)) { //Check the bit
            card_type = kCardTypeSDHC;
            ((int *) & dat[1200])[0] = 0; // adrShift
        } else {
            card_type = kCardTypeSD;
        }
        ERROR_CHECK; // For the previous ReadSlow32
    } else {
        //  Either a type 1 SD card, or it's MMC, try SD 1st
        if (SendCommandSlow(Acmd41, 0, 0xE5) < 2) {
            //  this is a type 1 SD card (1 means busy, 0 means done initializing)
            card_type = kCardTypeSD;
            while (SendCommandSlow(Acmd41, 0, 0xE5)) {
                YIELD();
            }
        } else {
            // mark that it's MMC, and try to initialize
            card_type = kCardTypeMMC;
            while (SendCommandSlow(Cmd1, 0, 0xF9)) {
                YIELD();
            }
        }

        // some SD or MMC cards may have the wrong block size, set it here
        SendCommandSlow(Cmd16, 512, 0x15);
    }

    // card is mounted, make sure the CRC is turned off
    SendCommandSlow(Cmd59, 0, 0x91);

    // done with the SPI bus for now
    OUTA |= mask_cs;

    // set my counter modes for super fast SPI operation

    // writing: NCO single-ended mode, output on DI
    ((int *) & dat[1212])[0] = (0b00100 << 26) | (pin_di << 0);

    ((int *) & dat[1220])[0] = (0b00100 << 26) | (pin_clk << 0); //  NCO, 50% duty cycle

    ((int *) & dat[1216])[0] = CLKFREQ >> (1 + 2 + 3); //  how many bytes (8 clocks, >>3) fit into 1/2 of a second (>>1), 4 clocks per instruction (>>2)?

    //  how long should we wait before auto-exiting any multiblock mode?
    const int idle_limit = 125; // ms, NEVER make this > 1000
    ((int *) & dat[1256])[0] = CLKFREQ / (1000 / idle_limit); //  convert to counts

    // Hand off control to the assembly engine's cog 
    ((int *) & dat[1204])[0] = (int) (&spi_buffer_address);
    ((int *) & dat[1208])[0] = (int) (&spi_block_index);
    spi_command = 0;

    spi_engine_cog = (cognew((int) (&(*(int *) & dat[0])), (int) (&spi_command)) + 1);

    if (spi_engine_cog == 0) {
        Crash(kErrorSpiEngineNotRunning);
        return 0;
    }

    while (spi_command != (-1)) {
        YIELD();
    }

    DIRA &= ~mask_all;
    return card_type;
}

void SdSafeSPI::ReleaseCard(void) {
    // I do not want to abort if the cog is not
    // running, as this is called from stop, which
    // is called from start/ [8^)  

    if (spi_engine_cog) {
        spi_command = 'z';
        while (spi_command == 'z') {
            YIELD();
        }
    }

}

void SdSafeSPI::Stop(void) {
    ReleaseCard();
    if (spi_engine_cog) {
        cogstop(spi_engine_cog - 1);
        spi_engine_cog = 0;
    }
}

void SdSafeSPI::Crash(int abort_code) {
    // and we no longer need to control any pins from here
    DIRA &= ~mask_all;
    error = abort_code;
    //return Abort_code;
}

int SdSafeSPI::SendCommandSlow(int command, int value, int crc) {
    // if this is an application specific command, handle it
    if (command & 0x80) {
        // ACMD<n> is the command sequence of CMD55-CMD<n>
        command &= 0x7f;
        int ReplyA = SendCommandSlow(Cmd55, 0, 0x65);
        if (ReplyA > 1) {
            return ReplyA;
        }
    }

    //  the CS line needs to go low during this operation
    OUTA |= mask_cs;
    OUTA &= ~mask_cs;

    //  give the card a few cocks to finish whatever it was doing
    ReadSlow32();
    ERROR_CHECK;
    SendSlow(command, 8);
    SendSlow(value, 32);
    SendSlow(crc, 8);
    if (command == Cmd12) { // if so, stuff byte
        ReadSlow();
        ERROR_CHECK;
    }

    //  read back the response (spec declares 1-8 reads max for SD, MMC is 0-8)
    int ReplyB;
    int Time_stamp = 9;
    do {
        ReplyB = ReadSlow();
        ERROR_CHECK;
    } while ((ReplyB & 0x80) && (Time_stamp--));
    return ReplyB;
}

void SdSafeSPI::SendSlow(int value, int bits_to_send) {
    value = (__builtin_propeller_rev(value, 32 - bits_to_send));
    
    for(int i = 0; i < bits_to_send; i++){
        OUTA &= ~mask_clk;
        if(value & 1){
            OUTA |= mask_di;
        }else{
            OUTA &= ~mask_di;
        }
        value = ((unsigned)value) >>1;
        OUTA |= mask_clk;
    }
}

int SdSafeSPI::ReadSlow32(void) {
    int R = 0;
    for (int i = 0; i < 4; i++) {
        R <<= 8;
        R |= ReadSlow();
        ERROR_CHECK;
    }
    return R;

}

int SdSafeSPI::ReadSlow(void) {
    OUTA |= mask_di; // we need the DI line high so a read can occur

    int R = 0;
    for (int i = 0; i < 8; i++) { // Get 8 bits
        OUTA &= ~mask_clk;
        OUTA |= mask_clk;
        R += R + ((INA & mask_do) ? 1 : 0);
    }
    if ((CNT - spi_block_index) > (CLKFREQ << 2)) {
        Crash(kErrorCardBusyTimeout);
        return 0;
    }
    return R;
}

int SdSafeSPI::CheckError(void){
    return error;
}

void SdSafeSPI::ClearError(void){
    error = kNoError;
}

int SdSafeSPI::GetSeconds(void) {
    if (spi_engine_cog == 0) {
        Crash(kErrorSpiEngineNotRunning);
        return 0;
    }
    spi_command = 't';
    //  seconds are in SPI_block_index, remainder is in SPI_buffer_address
    while (spi_command == 't') {
        YIELD();
    }
    return spi_block_index;
}

int SdSafeSPI::GetMilliseconds(void) {
    int Ms = 0;
    if (spi_engine_cog == 0) {
        Crash(kErrorSpiEngineNotRunning);
        return 0;
    }
    spi_command = 't';
    //seconds are in SPI_block_index, remainder is in SPI_buffer_address
    while (spi_command == 't') {
        YIELD();
    }
    Ms = (spi_block_index * 1000);
    Ms = (Ms + (((int) spi_buffer_address * 1000) / CLKFREQ));
    return Ms;
}