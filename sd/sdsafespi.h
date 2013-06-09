/**
 *
 *   SPI interface routines for SD & SDHC & MMC cards
 * 
 * C++ conversion by SRLM, based on sdsafespi.spin version 0.3.0 by Jonathan
 * "lonesock" Dummer.
 * 
 * This uses multiblock SPI mode exclusively.
 * 
 * This is the "SAFE" version...uses
 * - 1 instruction per bit writes
 * - 2 instructions per bit reads
 * 
 * @warning You should check the error code after each function call. Something
 * may have gone wrong, and this is the only way to know! If there is an error,
 * the safest thing to do would be to destroy the object instance, fix the
 * error, and try again. For some errors, it may be ok to simply clear the error
 * and try again.
 * 
 * Notes:
 * - It appears that negative Spi_command is never used in the assembly code.
 * This looks like an old error possibility that is no longer used. Can we take
 * it out?
 * 
 * @todo (SRLM): Convert the SD SPI driver to GAS instead of binary.
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

    /** Start a new cog with the SPI driver.
     * 
     * @warning requires that the pins be both consecutive, and in the order 
     * -# DO
     * -# CLK 
     * -# DI
     * -# CS
     * 
     * @return The card type constant.
     */
    int Start(int basepin);

    /**  Start a new cog with the SPI driver.
     * 
     * @param Do
     * @param Clk
     * @param Di
     * @param Cs
     * @return The card type constant.
     */
    int Start(int Do, int Clk, int Di, int Cs);

    void ReadBlock(int block_index, char * buffer_address);
    void WriteBlock(int block_index, char * buffer_address);


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

    /** If there was an error in the SD routines then this function will return
     * an error code.
     * 
     * @return The error code.
     */
    bool HasError(void);

    /** Resets the error flag to kNoError.
     */
    void ClearError(void);
    
    /**
     */
    int GetError(void);

private:
    volatile static unsigned char dat[];

    int mask_all;
    int mask_do;
    int mask_di;
    int mask_cs;
    int mask_clk;

    int error;
    int spi_engine_cog;

    // these are used for interfacing with the assembly engine
    volatile int spi_command;
    volatile int spi_block_index; // which 512-byte block to read/write
    volatile char * spi_buffer_address; // Accessed by the GAS cog, and points to a data buffer.

    /** In case of Bad Things(TM) happening, exit as gracefully as possible.
     * 
     * @param Abort_code passed through to return.
     */
    void SetErrorCode(int abort_code);

    /** Send down a command and return the reply.
     *  
     * Note: slow is an understatement!
     * 
     * Note(SRLM): The note below may not be relevant in the C++ version.
     * Note: this uses the assembly DAT variables for pin IDs, which means that 
     * if you run this multiple times (say for multiple SD cards), these values 
     * will change for each one. But this is OK as all of these functions will 
     * be called during the initialization only, before the PASM engine is running.
     * @param command
     * @param value
     * @param crc
     * @return ?
     */
    int SendCommandSlow(int command, int value, int crc);
    
    void SendSlow(int value, int bits_to_send);


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


    // SRLM: I don't know what these functions do...
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
