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

unsigned char i2cBase::ReadByte(const bool acknowledge){	
    int result;
	int datamask, nextCNT, temp;

	__asm__ (
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
		[datamask] "=&r" (datamask),
    	[result]   "=&r" (result),
    	[temp]     "=&r" (temp),
    	[nextCNT]  "=&r" (nextCNT)

	: /* inputs */
		[SDAMask]     "r" (SDAMask),
		[SCLMask]     "r" (SCLMask),
		[acknowledge] "r" (acknowledge),
		[clockDelay]  "r" (clockDelay)
	);

	return result;
}

bool i2cBase::SendByte(const unsigned char byte){
	static_assert((int)true == 1, "Boolean true must be 1 for inline assembly to work.");
    
    int result;

	int datamask, nextCNT, temp;

	__asm__ (
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
	"         muxz    %[result],  #1            \n\t" // Set result to equal to Z flag (aka, 1 if ack'd)
   	"         or      dira,       %[SCLMask]    \n\t" // Set scl low
   	"         or      dira,       %[SDAMask]    \n\t" // Set sda low 
	"         jmp     __LMM_RET                 \n\t"
	"PutByteEnd: "
	"         .compress default                 \n\t"
	: /* outputs */
		[datamask] "=&r" (datamask),
    	[result]   "=&r" (result),
    	[nextCNT]  "=&r" (nextCNT),
    	[temp]     "=&r" (temp)
	: /* inputs */
		[SDAMask] "r" (SDAMask),
		[SCLMask] "r" (SCLMask),	
		[databyte]   "r" (byte),	
		[clockDelay] "r" (clockDelay)
	);

	return result;
}

void i2cBase::Initialize(const int SCLPin, const int SDAPin)
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

}
