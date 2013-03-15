/**A Mock Serial object to facilitate testing with unit tests.

Note: function documentation is located in the serial.h file.

@author SRLM
@date   2013-03-13
*/


#ifndef SRLM_PROPGCC_SERIAL_MOCK_H_
#define SRLM_PROPGCC_SERIAL_MOCK_H_

#include <stdint.h>
#include <cstdarg>
#include "numbers.h"

class Serial {
public:

static const int kBufferLength = 256;


bool Start(int rxpin, int txpin, int rate){
	if(rxpin < 0 || rxpin > 31){
		return false;
	}
	if(txpin < -1 || txpin > 31){
		return false;
	}
	
	rx_head_ = rx_tail_ = 0;
	tx_head_ = tx_tail_ = 0;
	
	return true;
}


void Stop(void){
	//Do nothing
	rx_head_ = rx_tail_ = 0;
	tx_head_ = tx_tail_ = 0;
}
  

bool SetBaud(int rate){
	return SetBaudClock(rate, CLKFREQ);
}

bool SetBaudClock(int rate, int sysclock){
	return true;
}

void Put(char character){
	tx_buffer[tx_head_] = character;
	tx_head_ = (tx_head_ + 1) % kBufferSize;
}

/** printf function-alike.
@warning The mock function doesn't do anything with the extra parameters.
*/
int Put(const char * format, ...){
	int i = 0;
	for(; format[i] != '\0'; ++i){
		Put(format[i]);
	}
	return i;
}

int PutRemove(void){
	if(tx_tail_ != tx_head_){
		char result = tx_buffer[tx_tail_];
		tx_tail_ = (tx_tail + 1) % kBufferSize;
		return (int) result;
	} else{
		return -1;
	}
}


/**
@return the number of bytes put into buffer.
*/
int PutRemove(char * buffer, const int count){
	int i = 0;
	for(; i < count; ++i){
		int temp = PutRemove();
		if(temp == -1){
			break;
		}
		buffer[i] = (char)temp;
	}
	return i;
}


/**
Waits for a byte to be received or a timeout to occur.
*/
int Get(int timeout = -1);

/** Get a buffer of characters
*/
int Get(char * buffer, int length, int timeout=-1);

/** Get a terminated string of characters
*/
int Get(char * buffer, char terminator='\n');

/** Flushes receive buffer.
*/
void GetFlush(void);

void GetAdd(char byte){
	rx_buffer_[rx_head_] = byte;
	rx_head_ = (rx_head + 1) % kBufferSize
}

void GetAdd(char * bytes, int count){
	for(int i = 0; i < count; ++i){
		GetAdd(bytes[i]);
	}
}


private:


  

/**
Half period must be at least this, otherwise the cog will
sleep for a whole counter cycle (2^32 / clkfreq seconds).
*/
static const int kMinimumHalfPeriod = 86;
  
/**not used directly, but handy to know*/
static const int kXon = 17;
static const int kXoff = 19;
  
/**
The binary version of the PASM driver. Must be binary so that we can do garbage
collection with the linker.
*/  
static uint8_t dat[];

/** Checks if byte is waiting in the buffer, but doesn't wait.

@return -1 if no byte received, 0x00..0xFF if byte
*/
  int	CheckBuffer(void);

  int32_t	volatile write_buf_ptr_;
  int32_t	volatile send_temp_;
  int32_t	volatile half_bit_period_;
  uint16_t	volatile rx_head_;
  uint16_t	volatile rx_tail_;
  uint8_t	volatile rx_buffer_[kBufferLength];
  
  //Mock use Only
  uint16_t	volatile tx_head_;
  uint16_t	volatile tx_tail_;
  uint8_t	volatile tx_buffer_[kBufferLength];
  
  
  uint8_t	cog_;
};

void Serial::GetFlush(void)
{
  rx_tail_ = rx_head_;
}

int Serial::Get(int timeout)
{
	if(timeout <= -1){ //No timeout, wait forever
		int rxbyte = 0;
		while ((rxbyte = CheckBuffer()) < 0);
		return (char)rxbyte;
	}else{
		int tout = (CLKFREQ/1000)*timeout;
		int rxbyte;
		int totaltime = 0;
		int previous_cnt = CNT;
		int current_cnt;
		do
		{
			rxbyte = CheckBuffer();
			current_cnt = CNT;
			totaltime += current_cnt-previous_cnt;
			previous_cnt = current_cnt;
		}while ( rxbyte < 0 && totaltime < tout);
		return rxbyte;
	}
}

int Serial::Get(char * buffer, int length, int timeout){
	int i;
	for(i = 0; i < length; ++i){
		int character = Get(timeout);
		if(character <= -1){
			return i;
		}
		buffer[i] = (char) character;
	}
	return i;
}

int Serial::Get(char * buffer, char terminator){
	int i;
	char character = terminator + 1; //guarentee that they differ the first time.
	for(i = 0; character != terminator; ++i){
		character = (char)Get();
		buffer[i] = character;
	}
	buffer[i] = '\0';
	return i;
}



int Serial::CheckBuffer(void)
{
  int rxbyte = -1;
  if (rx_tail_ != rx_head_) {
    rxbyte = rx_buffer_[rx_tail_];
    rx_buffer_[rx_tail_] = 0;
    rx_tail_ = ((rx_tail_ + 1) % kBufferLength);
  }
  return rxbyte;
}


#endif //SRLM_PROPGCC_SERIAL_MOCK_H_
