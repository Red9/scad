//TODO(SRLM): Warning: the Read_slow function has a Crash call, but nowhere else
//is the Read_slow function, when called, enclosed by a test to check for the
//negative return value. Every time that it is called needs something like:
// int errno = Read_slow(...)
// if(errno < 0)
// 	return errno;

#include <propeller.h>
#include "sdsafespi.h"

#include <stdio.h>

#define INLINE__ static inline
#define Yield__() __asm__ volatile( "" ::: "memory" )
#define PostEffect__(X, Y) __extension__({ int32_t tmp__ = (X); (X) = (Y); tmp__; })


//TODO(SRLM): Convert the SD SPI driver to GAS instead of binary.

INLINE__ int32_t Shr__(uint32_t a, uint32_t b) {
    return (a >> b);
}
volatile uint8_t SdSafeSPI::dat[] = {
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

int32_t SdSafeSPI::Start(int32_t Basepin) {
    return Start_explicit(Basepin, (Basepin + 1), (Basepin + 2), (Basepin + 3));
}

/**
 * 
 * @return 
 * kErrorSpiEngineNotRunning
 * kErrorBlockNotLongAligned
 * 
 */
int32_t SdSafeSPI::Readblock(int32_t Block_index, char * Buffer_address) {
    if (Spi_engine_cog == 0) {
        return kErrorSpiEngineNotRunning;
    }
    if ((int) Buffer_address & 0x3) {
        return kErrorBlockNotLongAligned;
    }
    Spi_block_index = Block_index;
    Spi_buffer_address = Buffer_address;
    Spi_command = 'r';
    while (Spi_command == 'r') {
        Yield__();
    }
    if (Spi_command < 0) {
        return Spi_command;
    }
    return 0;
}

/**
 * 
 * @return 
 *  kErrorSpiEnigneNotRunning
 *  kErrorBlockNotLongAligned
 * 0
 */
int32_t SdSafeSPI::Writeblock(int32_t Block_index, char * Buffer_address) {
    if (Spi_engine_cog == 0) {
        return kErrorSpiEngineNotRunning;
    }
    if ((int) Buffer_address & 0x3) {
        return kErrorBlockNotLongAligned;
    }
    Spi_block_index = Block_index;
    Spi_buffer_address = Buffer_address;
    Spi_command = 'w';
    while (Spi_command == 'w') {
        Yield__();
    }
    if (Spi_command < 0) {
        return Spi_command;
    }
    return 0;
}

/**
 * 
 * @return
 * kErrorSpiEngineNotRunning
 * Seconds
 */
int32_t SdSafeSPI::Get_seconds(void) {
    if (Spi_engine_cog == 0) {
        return kErrorSpiEngineNotRunning;
    }
    Spi_command = 't';
    //  seconds are in SPI_block_index, remainder is in SPI_buffer_address
    while (Spi_command == 't') {
        Yield__();
    }
    return Spi_block_index;
}

/**
 * 
 * @return 
 * kErrorSpiEngineNotRunning
 * milliseconds
 */
int32_t SdSafeSPI::Get_milliseconds(void) {
    int32_t Ms = 0;
    if (Spi_engine_cog == 0) {
        return kErrorSpiEngineNotRunning;
    }
    Spi_command = 't';
    //seconds are in SPI_block_index, remainder is in SPI_buffer_address
    while (Spi_command == 't') {
        Yield__();
    }
    Ms = (Spi_block_index * 1000);
    Ms = (Ms + (((int) Spi_buffer_address * 1000) / CLKFREQ));
    return Ms;
}

/**
 * 
 * @return 
 */
int32_t SdSafeSPI::Start_explicit(int32_t Do, int32_t Clk, int32_t Di, int32_t Cs) {

    // Do all of the card initialization in C++, then hand off the pin
    // information to the assembly cog for hot SPI block R/W action!

    Stop();
    waitcnt(500 + CLKFREQ * 4 / 1000 + CNT);

    maskDO = 1 << Do;
    maskDI = 1 << Di;
    maskCS = 1 << Cs;
    maskAll = maskCS | (1 << Clk) | maskDI;

    ((int32_t *) & dat[1172])[0] = Do; // pinDo
    ((int32_t *) & dat[1184])[0] = maskDO; // maskDo
    ((int32_t *) & dat[1176])[0] = Clk; //pinClk
    ((int32_t *) & dat[1180])[0] = Di; //pinDI
    ((int32_t *) & dat[1188])[0] = maskDI; //maskDI
    ((int32_t *) & dat[1192])[0] = maskCS; //maskCS
    ((int32_t *) & dat[1200])[0] = 9; //adrShift //block = 512 * index, and 512 = 1 << 9
    
    ((int32_t *) & dat[1196])[0] = maskAll; // pass the output pin mask via the command register
    DIRA |= maskAll;

    // get the card in a ready state: set DI and CS high, send => 74 clocks
    OUTA |= maskAll;
    for (int i = 0; i < 4096; i++) {
        OUTA |= 1 << Clk;
        OUTA &= ~(1 << Clk);
    }

    Spi_block_index = CNT; //Time hack
    
    int TmpA = 0;
    for (int i = 0; i < 10; i++) {
        if (TmpA != 1) {
            TmpA = Send_cmd_slow(Cmd0, 0, 0x95);
            if (TmpA & 4) {
                // the card said CMD0 ("go idle") was invalid, so we're possibly stuck in read or write mode
                if (i & 1) {
                    // exit multiblock read mode

                    for (int jk = 0; jk < 4; jk++) {
                        Read_32_slow(); // these extra clocks are required for some MMC cards
                    }
                    Send_slow(0xFD, 8); // stop token
                    Read_32_slow();
                    while (Read_slow() != 0xFF) {
                        Yield__();
                    }
                } else {
                    //  exit multiblock read mode
                    Send_cmd_slow(Cmd12, 0, 0x61);
                }
            }
        }

    }
    if (TmpA != 1) {
        //  the reset command failed!
        return Crash(kErrorCardNotReset);
    }

    int Card_type = 0;
    if (Send_cmd_slow(Cmd8, 426, 135) == 1) { // Is this a SD type 2 card?
        //  Type2 SD, check to see if it's a SDHC card

        if ((Read_32_slow() & 0x1ff) != 0x1AA) { //check the supported voltage
            return Crash(kError3v3NotSupported);
        }

        //  try to initialize the type 2 card with the High Capacity bit
        while (Send_cmd_slow(Acmd41, ((1 << 30)), 0x77)) {
            Yield__();
        }

        // the card is initialized, let's read back the High Capacity bit
        if (Send_cmd_slow(Cmd58, 0, 0xFD) != 0) {
            return Crash(kErrorOcrFailed);
        }


        //  get back the data
        if (Read_32_slow() & (1 << 30)) { //Check the bit
            Card_type = Type_sdhc;
            ((int32_t *) & dat[1200])[0] = 0; // adrShift
        } else {
            Card_type = Type_sd;
        }
    } else {
        //  Either a type 1 SD card, or it's MMC, try SD 1st
        if (Send_cmd_slow(Acmd41, 0, 0xE5) < 2) {
            //  this is a type 1 SD card (1 means busy, 0 means done initializing)
            Card_type = Type_sd;
            while (Send_cmd_slow(Acmd41, 0, 0xE5)) {
                Yield__();
            }
        } else {
            // mark that it's MMC, and try to initialize
            Card_type = Type_mmc;
            while (Send_cmd_slow(Cmd1, 0, 0xF9)) {
                Yield__();
            }
        }

        // some SD or MMC cards may have the wrong block size, set it here
        Send_cmd_slow(Cmd16, 512, 0x15);
    }

    // card is mounted, make sure the CRC is turned off
    Send_cmd_slow(Cmd59, 0, 0x91);

    // done with the SPI bus for now
    OUTA |= maskCS;

    // set my counter modes for super fast SPI operation

    // writing: NCO single-ended mode, output on DI
    ((int32_t *) & dat[1212])[0] = (0b00100 << 26) | (Di << 0);


    ((int32_t *) & dat[1220])[0] = (0b00100 << 26) | (Clk << 0); //  NCO, 50% duty cycle


    ((int32_t *) & dat[1216])[0] = CLKFREQ >> (1 + 2 + 3); //  how many bytes (8 clocks, >>3) fit into 1/2 of a second (>>1), 4 clocks per instruction (>>2)?

    //  how long should we wait before auto-exiting any multiblock mode?
    const int idle_limit = 125; // ms, NEVER make this > 1000
    ((int32_t *) & dat[1256])[0] = CLKFREQ / (1000 / idle_limit); //  convert to counts

    // Hand off control to the assembly engine's cog 
    ((int32_t *) & dat[1204])[0] = (int32_t) (&Spi_buffer_address);
    ((int32_t *) & dat[1208])[0] = (int32_t) (&Spi_block_index);
    Spi_command = 0;

    Spi_engine_cog = (cognew((int32_t) (&(*(int32_t *) & dat[0])), (int32_t) (&Spi_command)) + 1);

    if (Spi_engine_cog == 0) {
        return Crash(kErrorSpiEngineNotRunning);
    }

    while (Spi_command != (-1)) {
        Yield__();
    }

    DIRA &= ~maskAll;
    return Card_type;
}

void SdSafeSPI::Release(void) {
    // I do not want to abort if the cog is not
    // running, as this is called from stop, which
    // is called from start/ [8^)  
    
    if (Spi_engine_cog) {
        Spi_command = 'z';
        while (Spi_command == 'z') {
            Yield__();
        }
    }
    
}

void SdSafeSPI::Stop(void) {
    Release();
    if (Spi_engine_cog) {
        cogstop(Spi_engine_cog - 1);
        Spi_engine_cog = 0;
    }
}

int SdSafeSPI::Crash(int Abort_code) {
    // and we no longer need to control any pins from here
    DIRA &= ~maskAll;
    return Abort_code;
}

int32_t SdSafeSPI::Send_cmd_slow(int32_t Cmd, int32_t Val, int32_t Crc) {
    // if this is an application specific command, handle it
    if (Cmd & 0x80) {
        // ACMD<n> is the command sequence of CMD55-CMD<n>
        Cmd &= 0x7f;
        int ReplyA = Send_cmd_slow(Cmd55, 0, 0x65);
        if (ReplyA > 1) {
            return ReplyA;
        }
    }
    
    //  the CS line needs to go low during this operation
    OUTA |= maskCS;
    OUTA &= ~maskCS;
    
    //  give the card a few cocks to finish whatever it was doing
    Read_32_slow();
    Send_slow(Cmd, 8);
    Send_slow(Val, 32);
    Send_slow(Crc, 8);
    if (Cmd == Cmd12) { // if so, stuff byte
        Read_slow();
    }
    
    //  read back the response (spec declares 1-8 reads max for SD, MMC is 0-8)
    int ReplyB;
    int Time_stamp = 9;
    do {
        ReplyB = Read_slow();
    } while ((ReplyB & 0x80) && (Time_stamp--));
    return ReplyB;
}

int32_t SdSafeSPI::Send_slow(int32_t Value, int32_t Bits_to_send) {
    int32_t result = 0;
    Value = (__builtin_propeller_rev(Value, 32 - Bits_to_send));
    {
        int32_t _idx__0022;
        _idx__0022 = Bits_to_send;
        do {
            OUTA &= ~(1 << (*(int32_t *) & dat[1176]));
            OUTA = ((OUTA & (~(1 << (*(int32_t *) & dat[1180])))) | ((Value & 0x1) << (*(int32_t *) & dat[1180])));
            Value = (Shr__(Value, 1));
            OUTA = ((OUTA & (~(1 << (*(int32_t *) & dat[1176])))) | (1 << (*(int32_t *) & dat[1176])));
            _idx__0022 = (_idx__0022 + -1);
        } while (_idx__0022 >= 1);
    }
    return result;
}

int32_t SdSafeSPI::Read_32_slow(void) {
    int32_t R = 0;
    {
        int32_t _idx__0023;
        _idx__0023 = 4;
        do {
            R = (R << 8);
            R = (R | Read_slow());
            _idx__0023 = (_idx__0023 + -1);
        } while (_idx__0023 >= 1);
    }
    return R;
}

int32_t SdSafeSPI::Read_slow(void) {
    int32_t R = 0;
    OUTA = ((OUTA & (~(1 << (*(int32_t *) & dat[1180])))) | (1 << (*(int32_t *) & dat[1180])));
    {
        int32_t _idx__0024;
        _idx__0024 = 8;
        do {
            OUTA &= ~(1 << (*(int32_t *) & dat[1176]));
            OUTA = ((OUTA & (~(1 << (*(int32_t *) & dat[1176])))) | (1 << (*(int32_t *) & dat[1176])));
            R = (R + (R + ((INA >> (*(int32_t *) & dat[1172])) & 0x1)));
            _idx__0024 = (_idx__0024 + -1);
        } while (_idx__0024 >= 1);
    }
    if ((CNT - Spi_block_index) > (CLKFREQ << 2)) {
        return Crash(kErrorCardBusyTimeout);
    }
    return R;
}

