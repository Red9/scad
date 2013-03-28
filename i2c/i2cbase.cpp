//TODO: Add assert that bool is 4 bytes (or puttable in register?). (Static_assertion)
//TODO(SRLM): Do the ASM sections need to be volatile?

#include "i2cbase.h"

#define i2c_float_scl_high (DIRA &= ~SCLMask)
#define i2c_set_scl_low    (DIRA |= SCLMask)
#define i2c_float_sda_high (DIRA &= ~SDAMask)
#define i2c_set_sda_low    (DIRA |= SDAMask)

void i2cBase::Start()
{
	i2c_float_sda_high;
	i2c_float_scl_high;
	i2c_set_sda_low;
	i2c_set_scl_low;
}

void i2cBase::Stop()
{
	i2c_float_scl_high;
	i2c_float_sda_high;
}

unsigned char i2cBase::ReadByte(bool acknowledge)
{
	
    int result;
	int datamask, nextCNT, temp;
//	datamask = nextCNT = temp = 0; //Results in gibberish if uncommented?

	__asm__ volatile(
	"         fcache #(GetByteEnd - GetByteStart)\n\t"
	"         .compress off                   \n\t"
	/* Setup for receive loop */
	"GetByteStart: "
	"         andn dira,        %[SDAMask]    \n\t"
	"         mov  %[datamask], #256          \n\t" /* 0x100 */
	"         mov  %[result],   #0            \n\t"
	"         mov  %[nextCNT],  cnt           \n\t"
	"         add  %[nextCNT],  %[clockDelay] \n\t"
	
	/* Recieve Loop (8x) */
	//Get bit of byte
    "GetByteLoop: "

	"         waitcnt %[nextCNT],  %[clockDelay] \n\t"  
	"         shr     %[datamask], #1            \n\t"      // Set up mask
	
	    //Pulse clock
	"         andn    dira,       %[SCLMask]       \n\t"    // Set SCL high
	"         waitcnt %[nextCNT], %[clockDelay]    \n\t" 
	"         mov     %[temp],    ina              \n\t"  //Sample the input
	"         and     %[temp],    %[SDAMask] nr,wz \n\t"
	"         muxnz   %[result],  %[datamask]      \n\t"
	"         or      dira,       %[SCLMask]       \n\t"        // Set SCL low
	
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
	
	: /* outputs */
		[datamask] "+&r" (datamask),
    	[result]   "=&r" (result)

	: /* inputs */
		[SDAMask] "r" (SDAMask),
		[SCLMask] "r" (SCLMask),
		
		[temp]        "r" (temp),
		[acknowledge] "r" (acknowledge),
		
		
		[nextCNT]    "r" (nextCNT),
		[clockDelay] "r" (clockDelay)
	
	);

	return result;

//    uint8_t byte = 0;
//    int count;

//    i2c_float_sda_high;

//    for (count = 8; --count >= 0; ) {
//        byte <<= 1;
//        i2c_float_scl_high;
//        waitcnt(CNT + halfCycle);
//        byte |= (INA & SDAMask) ? 1 : 0;
//        i2c_set_scl_low;
//        waitcnt(CNT + halfCycle);
//    }

//    // acknowledge
//    if (acknowledge)
//        i2c_set_sda_low;
//    else
//        i2c_float_sda_high;
//    //Send SCL 
//    //Clock out the ack bit
//    i2c_float_scl_high;
//    i2c_set_scl_low;
//    
//    i2c_set_sda_low;

//    return byte;
}

int i2cBase::SendByte(unsigned char byte)
{
    int result;

	int datamask, nextCNT, temp;
	datamask = nextCNT = temp = 0;

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
	"         shr  %[datamask], #1                \n\t"      // Set up mask
	"         and  %[datamask], %[databyte] wz,nr \n\t" // Move the bit into Z flag
    "         muxz dira,        %[SDAMask]        \n\t"
    
    //Pulse clock
	"         waitcnt %[nextCNT], %[clockDelay] \n\t"  
	"         andn    dira,       %[SCLMask]    \n\t"    // Set SCL high
	"         waitcnt %[nextCNT], %[clockDelay] \n\t" 
	"         or      dira,       %[SCLMask]    \n\t"        // Set SCL low
	
	//Return for more bits
	"         djnz %[datamask], #__LMM_FCACHE_START+(PutByteLoop-PutByteStart) nr \n\t"
	
	// Get ACK
	"         andn    dira,       %[SDAMask]    \n\t" // Float SDA high (release SDA)
	"         waitcnt %[nextCNT], %[clockDelay] \n\t"
   	"         andn    dira,       %[SCLMask]    \n\t" // SCL high (by float)
   	"         waitcnt %[nextCNT], %[clockDelay] \n\t"
   	"         mov     %[temp],    ina           \n\t" //Sample input
   	"         and     %[SDAMask], %[temp] wz,nr \n\t" // If != 0, ack'd, else nack
	"         muxz    %[result],  #1            \n\t" // Set result to equal to Z flag
   	"         or      dira,       %[SCLMask]    \n\t" // Set scl low
   	"         or      dira,       %[SDAMask]    \n\t" // Set sda low 
	"         jmp     __LMM_RET                 \n\t"
	"PutByteEnd: "
	"         .compress default                 \n\t"
	
	: /* outputs */
		[datamask] "+&r" (datamask),
    	[result]   "=&r" (result)
	: /* inputs */
		[SDAMask] "r" (SDAMask),
		[SCLMask] "r" (SCLMask),
		
		[temp]    "r" (temp),
		
		[databyte]   "r" (byte),
		[nextCNT]    "r" (nextCNT),
		[clockDelay] "r" (clockDelay)
	);

	return result;

//C++ version:
//    int count;
//    /* send the byte, high bit first */
//    for (count = 8; --count >= 0; ) {
//    	//Change byte while clock is low
//        if (byte & 0x80)
//            i2c_float_sda_high;
//        else
//            i2c_set_sda_low;
//            
//        //Send clock high pulse
//        i2c_float_scl_high;
//        waitcnt(CNT + halfCycle);
//		//Prepare byte for next time around
//        byte <<= 1;

//        i2c_set_scl_low;
//        
//     } 
//    
//    /* receive the acknowledgement from the slave */

//    i2c_float_sda_high; //DIRA &= ~SDAMask
//    i2c_float_scl_high;
//    
//    result = !((INA & SDAMask) != 0);
//    i2c_set_scl_low;  // DIRA |= SCLMask
//    
//    //Pull SDA low
//    i2c_set_sda_low;

//    return result; //Ack or NAK

}

int i2cBase::Initialize(int SCLPin, int SDAPin)
{
	SCLMask = 1 << SCLPin;
	SDAMask = 1 << SDAPin;
	
	
	//Set pins to input
	i2c_float_scl_high;
	i2c_float_sda_high;
	
	
	//Set outputs low
	OUTA &= ~SCLMask;
	OUTA &= ~SDAMask;
	
	clockDelay = 90;
	
	return 0;

}
