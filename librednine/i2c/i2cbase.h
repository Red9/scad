#ifndef SRLM_PROPGCC_I2CBASE_H__
#define SRLM_PROPGCC_I2CBASE_H__

#include <propeller.h>

#define i2c_float_scl_high (DIRA &= ~SCLMask)
#define i2c_set_scl_low    (DIRA |= SCLMask)
#define i2c_float_sda_high (DIRA &= ~SDAMask)
#define i2c_set_sda_low    (DIRA |= SDAMask)

//Note: refactoring this into it's own class took all of 8 bytes extra :^)

/** Low level I2C driver. Only does the most basic functions that all I2C
devices implement.

Requires that the SDA and SCL pins have sufficient pullups. These should be
selected based on the capacitance of the devices on the I2C bus, and the
expected clock speed (400kHz currently).


@author SRLM
@date   2013-01-21
@version 1.1

Version History
+ 1.1 Updated the routines to use FCACHE'd inline assembly. It now runs faster
      and more precisely.
+ 1.0 Initial release.
 */
class i2cBase {
public:

    /** Set the IO Pins to float high. Does not require a cog.

    Sets the bus speed to 444kHz.

    @param scl The I2C SCL pin. Defaults to the Propeller default SCL pin.
    @param sda The I2C SDA pin. Defaults to the Propeller default SDA pin.
    @returns Nothing, right now...
     */
    void Initialize(const int scl = 28, const int sda = 29) {
        SCLMask = 1 << scl;
        SDAMask = 1 << sda;

        //Set pins to input
        i2c_float_scl_high;
        i2c_float_sda_high;


        //Set outputs low
        OUTA &= ~SCLMask;
        OUTA &= ~SDAMask;

        clockDelay = 90;
    }

    /** Output a start condition on the I2C bus.
     */
    void Start() {
        i2c_float_sda_high;
        i2c_float_scl_high;
        i2c_set_sda_low;
        i2c_set_scl_low;
    }

    /** Output a stop condition on the I2C bus.
     */
    void Stop() {
        i2c_float_scl_high;
        i2c_float_sda_high;
    }

    /** Ouput a byte on the I2C bus.
    @param   byte the 8 bits to send on the bus.
    @returns      true if the device acknowledges, false otherwise.
     */
    bool SendByte(const unsigned char byte) {
        int result;

        int datamask, nextCNT, temp;

        __asm__ volatile(
                "         fcache #(PutByteEnd - PutByteStart)\n\t"
                "         .compress off                  \n\t"
                /* Setup for transmit loop */
                "PutByteStart: "
                "         mov %[datamask], #256          \n\t" /* 0x100 */
                "         mov %[result],   #0            \n\t"
                "         mov %[nextCNT],  cnt           \n\t"
                "         add %[nextCNT],  %[clockDelay] \n\t"

                /* Transmit Loop (8x) */
                //Output bit of byte
                "PutByteLoop: "
                "         shr  %[datamask], #1                \n\t" // Set up mask
                "         and  %[datamask], %[databyte] wz,nr \n\t" // Move the bit into Z flag
                "         muxz dira,        %[SDAMask]        \n\t"

                //Pulse clock
                "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                "         andn    dira,       %[SCLMask]    \n\t" // Set SCL high
                "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                "         or      dira,       %[SCLMask]    \n\t" // Set SCL low

                //Return for more bits
                "         djnz %[datamask], #__LMM_FCACHE_START+(PutByteLoop-PutByteStart) nr \n\t"

                // Get ACK
                "         andn    dira,       %[SDAMask]    \n\t" // Float SDA high (release SDA)
                "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                "         andn    dira,       %[SCLMask]    \n\t" // SCL high (by float)
                "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                "         mov     %[temp],    ina           \n\t" //Sample input
                "         and     %[SDAMask], %[temp] wz,nr \n\t" // If != 0, ack'd, else nack
                "         muxz    %[result],  #1            \n\t" // Set result to equal to Z flag (aka, 1 if ack'd)
                "         or      dira,       %[SCLMask]    \n\t" // Set scl low
                "         or      dira,       %[SDAMask]    \n\t" // Set sda low 
                "         jmp     __LMM_RET                 \n\t"
                "PutByteEnd: "
                "         .compress default                 \n\t"
                : // Outputs
                [datamask] "=&r" (datamask),
                [result] "=&r" (result),
                [nextCNT] "=&r" (nextCNT),
                [temp] "=&r" (temp)
                : // Inputs
                [SDAMask] "r" (SDAMask),
                [SCLMask] "r" (SCLMask),
                [databyte] "r" (byte),
                [clockDelay] "r" (clockDelay)
                );

        return result;
    }

    /** Get a byte from the I2C bus.

    @param acknowledge true to acknowledge the byte received, false otherwise.
    @returns the byte clocked in off the bus.
     */
    unsigned char ReadByte(const bool acknowledge) {

        int result = 0;
        int datamask, nextCNT, temp;

        __asm__ volatile(
                "         fcache #(GetByteEnd - GetByteStart)\n\t"
                "         .compress off                   \n\t"
                // Setup for receive loop
                "GetByteStart: "
                "         andn dira,        %[SDAMask]    \n\t"
                "         mov  %[datamask], #256          \n\t" /* 0x100 */
                "         mov  %[result],   #0            \n\t"
                "         mov  %[nextCNT],  cnt           \n\t"
                "         add  %[nextCNT],  %[clockDelay] \n\t"

                // Recieve Loop (8x)
                //Get bit of byte
                "GetByteLoop: "

                "         waitcnt %[nextCNT],  %[clockDelay] \n\t"
                "         shr     %[datamask], #1            \n\t" // Set up mask

                //Pulse clock
                "         andn    dira,       %[SCLMask]       \n\t" // Set SCL high
                "         waitcnt %[nextCNT], %[clockDelay]    \n\t"
                "         mov     %[temp],    ina              \n\t" //Sample the input
                "         and     %[temp],    %[SDAMask] nr,wz \n\t"
                "         muxnz   %[result],  %[datamask]      \n\t"
                "         or      dira,       %[SCLMask]       \n\t" // Set SCL low

                //Return for more bits
                "         djnz %[datamask], #__LMM_FCACHE_START+(GetByteLoop-GetByteStart) nr \n\t"

                // Put ACK

                "         and     %[acknowledge], #1 nr,wz  \n\t" //Output ACK

                "         muxnz   dira,       %[SDAMask]    \n\t"
                "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                "         andn    dira,       %[SCLMask]    \n\t" // SCL high (by float)
                "         waitcnt %[nextCNT], %[clockDelay] \n\t"

                "         or   dira, %[SCLMask]       \n\t" // Set scl low
                "         or   dira, %[SDAMask]       \n\t" // Set sda low 
                "         jmp  __LMM_RET              \n\t"
                "GetByteEnd: "
                "         .compress default           \n\t"

                : // Outputs
                [datamask] "=&r" (datamask),
                [result] "=&r" (result),
                [temp] "=&r" (temp),
                [nextCNT] "=&r" (nextCNT)

                : // Inputs
                [SDAMask] "r" (SDAMask),
                [SCLMask] "r" (SCLMask),
                [acknowledge] "r" (acknowledge),
                [clockDelay] "r" (clockDelay)
                );

        return result;

    }

private:
    unsigned int SCLMask;
    unsigned int SDAMask;


    //Clock delay values (@80MHz system clock):
    // 1600 == 25kHz
    //  400 == 100kHz
    //  100 == 400kHz
    //   90 == 444kHz
    //   32 == 1.25MHz
    int clockDelay;

};



#endif // SRLM_PROPGCC_I2CBASE_H__
