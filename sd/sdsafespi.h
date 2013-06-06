/**
 *
 *   SPI interface routines for SD & SDHC & MMC cards
 * 
 * C++ conversion by SRLM, based on sdsafespi.spin version 0.3.0 by Jonathan
 * "lonesock" Dummer.
 * 
 * Using multiblock SPI mode exclusively.
 * 
 * This is the "SAFE" version...uses
 * - 1 instruction per bit writes
 * - 2 instructions per bit reads
 * 
 * Notes:
 * - It appears that negative Spi_command is never used in the assembly code.
 * This looks like an old error possibility that is no longer used. Can we take
 * it out?
 * 
 * @author SRLM (srlm@srlmproductions.com)     
 */

#ifndef SRLM_PROPGCC_SDSAFESPI_H__
#define SRLM_PROPGCC_SDSAFESPI_H__

class SdSafeSPI {
public:
    static const int kCardTypeMMC = 1;
    static const int kCardTypeSD = 2;
    static const int kCardTypeSDHC = 3;

    static const int kNoError = 0;

    static const int kErrorCardNotReset = -1;
    static const int kError3v3NotSupported = -2;
    static const int kErrorOcrFailed = -3;
    static const int kErrorBlockNotLongAligned = -4;
    // These errors are for the assembly engine...they are negated inside, and need to be <= 511
    static const int kErrorAsmNoReadToken = 100;
    static const int kErrorAsmBlockNotWritten = 101;
    // NOTE: errors -128 to -255 are reserved for reporting R1 response errors
    static const int kErrorSpiEngineNotRunning = -999;
    static const int kErrorCardBusyTimeout = -1000;







    /**requires that the pins be
  both consecutive, and in the order DO CLK DI CS.
     * 
     * @param Do
     * @param Clk
     * @param Di
     * @param Cs
     * @return card type
     */
    int Start(int Basepin);
    int Start(int Do, int Clk, int Di, int Cs);


    /**
     * Error codes 
     *
     */
    void ReadBlock(int Block_index, char * Buffer_address);
    void WriteBlock(int Block_index, char * Buffer_address);


    /** Release the SPI bus and allow other devices to use it. The SPI bus is
     * reacquired the next time a block is transfered. 
     * 
     * @warning If you release the lines, make sure that the other devices are
     * not using the bus the next time you do an SD operation.
     * 
     */
    void ReleaseCard(void);

    /** kill the assembly driver cog.
     */
    void Stop(void);

    int CheckError(void);

private:
    volatile static unsigned char dat[];


    int maskAll;
    int maskDO;
    int maskDI;
    int maskCS;
    int maskCLK;

    int error;
    int Spi_engine_cog;

    // these are used for interfacing with the assembly engine
    volatile int Spi_command;
    volatile int Spi_block_index; // which 512-byte block to read/write
    volatile char * Spi_buffer_address; // Accessed by the GAS cog, and points to a data buffer.

    /** In case of Bad Things(TM) happening,
  exit as gracefully as possible.
     * 
     * @param Abort_code passed through to return.
     * @return 
     */
    void Crash(int Abort_code);

    /**
     *  Send down a command and return the reply.
  Note: slow is an understatement!
  Note: this uses the assembly DAT variables for pin IDs,
  which means that if you run this multiple times (say for
  multiple SD cards), these values will change for each one.
  But this is OK as all of these functions will be called
  during the initialization only, before the PASM engine is
  running.
     * @param Cmd
     * @param Val
     * @param Crc
     * @return positive number > 1 OR byte
     * Passes Error code:
     *  - kErrorCardBusyTimeout
     */
    int SendCommandSlow(int Cmd, int Val, int Crc);
    void SendSlow(int Value, int Bits_to_send);


    /**
     * Passes Error Code:
     *  - kErrorCardBusyTimeout
     * @return 
     */
    int ReadSlow32(void);

    /** Read back 8 bits from the card
     *  
     * @return a byte or kErrorCardBusyTimeout
     * Sets Error Code:
     *  - kErrorCardBusyTimeout
     */
    int ReadSlow(void);


    int GetSeconds(void);
    int GetMilliseconds(void);

    //  SDHC/SD/MMC command set for SPI
    static const int Cmd0 = 0x40 + 0;
    static const int Cmd1 = 0x40 + 1;
    static const int Acmd41 = 0xC0 + 41;
    static const int Cmd8 = 0x40 + 8;
    static const int Cmd9 = 0x40 + 9;
    static const int Cmd10 = 0x40 + 10;
    static const int Cmd12 = 0x40 + 12;
    static const int Cmd13 = 0x40 + 13;
    static const int Acmd13 = 0xC0 + 13;
    static const int Cmd16 = 0x40 + 16;
    static const int Cmd17 = 0x40 + 17;
    static const int Cmd18 = 0x40 + 18;
    static const int Cmd23 = 0x40 + 23;
    static const int Acmd23 = 0xC0 + 23;
    static const int Cmd24 = 0x40 + 24;
    static const int Cmd25 = 0x40 + 25;
    static const int Cmd55 = 0x40 + 55;
    static const int Cmd58 = 0x40 + 58;
    static const int Cmd59 = 0x40 + 59;

};

#endif // SRLM_PROPGCC_SDSAFESPI_H__
