#include "serial.h"

extern char _load_start_serial_cog[];

Serial::~Serial() {
    Stop();
}

void Serial::SetDriverLong(const int index, const int value) {
    ((int *) &_load_start_serial_cog[index])[0] = value;
}

/* Warning: SetDriverLong(char **, int) must be declared second, otherwise it calls itself! */
void Serial::SetDriverLong(char ** index, const int value) {
    SetDriverLong((int) index, value);
}

bool Serial::Start(const int rx_pin, const int tx_pin, const int rate, const int ctspin) {
    // Prevent garbage collection
    volatile void * asm_driver_reference = NULL;
    __asm__ volatile ("mov %[asm_driver_reference], #Fds_entry \n\t"
            : [asm_driver_reference] "+r" (asm_driver_reference));

    Stop();

    extern char * Masktx asm("Masktx");
    extern char * Maskrx asm("Maskrx");
    extern char * Ctra_val asm("Ctra_val");
    extern char * Ctrb_val asm("Ctrb_val");
    extern char * Period_ptr asm("Period_ptr");
    extern char * Rx_head_ptr asm("Rx_head_ptr");
    extern char * Rx_end_ptr asm("Rx_end_ptr");
    extern char * Update_head_ptr asm("Update_head_ptr");
    extern char * Maskcts asm("Maskcts");


    SetDriverLong(&Masktx, 0);
    SetDriverLong(&Ctra_val, 0);
    if (tx_pin >= 0) {
        DIRA |= 1 << tx_pin;
        SetDriverLong(&Masktx, 1 << tx_pin);
        SetDriverLong(&Ctra_val, 0x10000000 | tx_pin);
    }
    SetDriverLong(&Maskrx, 0);
    SetDriverLong(&Ctrb_val, 0);
    if (rx_pin >= 0) {
        DIRA &= ~(1 << rx_pin);

        SetDriverLong(&Maskrx, 1 << rx_pin);
        SetDriverLong(&Ctrb_val, 0x54000000 | rx_pin);
    }


    SetDriverLong(&Maskcts, 0);
    if (ctspin >= 0) {
        //Set CTS pin to input:
        DIRA &= ~(1 << ctspin);

        SetDriverLong(&Maskcts, 1 << ctspin);
    }


    SetBaud(rate);

    SetDriverLong(&Period_ptr, (int) &half_bit_period_);
    memset((void *) &rx_buffer_, 0, 1 * (kBufferLength));

    SetDriverLong(&Rx_head_ptr, (int) &rx_buffer_);
    SetDriverLong(&Rx_end_ptr, (int) &rx_buffer_ + kBufferLength);

    rx_head_ = 0;
    rx_tail_ = 0;

    SetDriverLong(&Update_head_ptr, (int) &rx_head_);
    write_buf_ptr_ = 1;
    cog_ = 1 + cognew((int) (&(*(int *) &_load_start_serial_cog[0])), (int) (&write_buf_ptr_));
    if (cog_) {
        WaitForTransmissionCompletion();
        return true;
    }
    return false;
}

void Serial::Stop(void) {
    WaitForTransmissionCompletion();
    if (cog_) {
        cogstop(cog_ - 1);
        cog_ = 0;
    }
}

bool Serial::SetBaud(const int rate) {

    return SetBaudClock(rate, CLKFREQ);
}

bool Serial::SetBaudClock(const unsigned int rate, const unsigned int sysclock) {
    WaitForTransmissionCompletion();

    // how many clocks per 1/2 bit (pre-round to the nearest integer)
    int got_rate = ((sysclock >> 1) + (rate >> 1)) / rate;

    // clamp the period to the allowable range
    half_bit_period_ = got_rate > kMinimumHalfPeriod ? got_rate : kMinimumHalfPeriod;

    // return true if the requested period was >= the allowable limit
    return got_rate >= kMinimumHalfPeriod;
}

void Serial::WaitForTransmissionCompletion(void) {
    while (write_buf_ptr_) {
    };
}

void Serial::Put(const char output_char) {
    WaitForTransmissionCompletion();
    send_temp_ = output_char;
    write_buf_ptr_ = (int) (&send_temp_);

}

int Serial::Put(const char * buffer_ptr, const int count) {
    WaitForTransmissionCompletion();
    for (int i = 0; i < count; i++) {
        Put(buffer_ptr[i]);
    }
    return count;
}

int Serial::Put(const char * buffer_ptr) {
    return Put(buffer_ptr, strlen(buffer_ptr));
}

//
// The Put(buffer) function doesn't seem to work in CMM mode. In the tests, I 
// get a -1 for the matching Get(), instead of the character sent. The same code
// can pass in LMM mode.

// Update: now, it doesn't work at all.

//int Serial::PutBuffer(char * buffer_ptr, const bool wait, int buffer_bytes, const char terminator)
//{
//	volatile char * volatile temp_ptr = buffer_ptr;
//	if(buffer_bytes == -1){
//		if(terminator == '\0'){
//			buffer_bytes = strlen(buffer_ptr);
//			
//		}else{
//			for(buffer_bytes = 0; buffer_ptr[buffer_bytes] != terminator; buffer_bytes++){}
//		}
//			
//	}

//	buffer_bytes = 5;

//	if (buffer_bytes > 0 && buffer_ptr != NULL) {
//	
//		send_temp_ = (int)(buffer_ptr);
//    	write_buf_ptr_ = (send_temp_ | ((buffer_bytes - 1) << 16));
//  	}
//	
//	if(wait){
//		while (write_buf_ptr_){}
//	}
//	return buffer_bytes;
//}

int Serial::PutFormatted(const char * format, ...) {
    if (format == NULL) {
        return 0;
    }

    int bytesWritten = 0;

    va_list list;
    va_start(list, format);


    for (int stringIndex = 0; format[stringIndex] != 0; stringIndex++) {

        if (format[stringIndex] == '%') {
            //Found formatter!
            stringIndex++;
            //Check for flags:
            bool padZero = false;
            int padAmount = 0;
            if (format[stringIndex] == '0') {
                padZero = true;
                stringIndex++;
            }
            if (format[stringIndex] >= '1' and format[stringIndex] <= '9') {
                char paddingBuffer[5];
                int paddingIndex = 0;


                //Non freezing version.				
                if (format[stringIndex] >= '0' and format[stringIndex] <= '9') {
                    paddingBuffer[paddingIndex++] = format[stringIndex++];
                    if (format[stringIndex] >= '0' and format[stringIndex] <= '9') {
                        paddingBuffer[paddingIndex++] = format[stringIndex++];
                        if (format[stringIndex] >= '0' and format[stringIndex] <= '9') {
                            paddingBuffer[paddingIndex++] = format[stringIndex++];
                            if (format[stringIndex] >= '0' and format[stringIndex] <= '9') {
                                paddingBuffer[paddingIndex++] = format[stringIndex++];
                            }
                        }
                    }
                }

                //TO DO(SRLM): figure out what is happening with the freezing version.
                //I think it freezes because of the CMM and fcache combination.
                //Freezing version:				
                //				while(format[stringIndex] >= '0' and format[stringIndex] <= '9'){
                //					paddingBuffer[paddingIndex++] = format[stringIndex];
                ////					printf("+%c+", format[stringIndex]);				
                //					stringIndex++;
                //				}
                paddingBuffer[paddingIndex] = 0;
                padAmount = Numbers::Dec(paddingBuffer);
                //				printf("paddingBuffer[0] = %c\r\n", paddingBuffer[0]);
                //				printf("paddingBuffer[1] = %c\r\n", paddingBuffer[1]);
                //				printf("paddingIndex = %i\r\n", paddingIndex);
                //				printf("padAmount: %i\r\n", padAmount);


            }



            if (format[stringIndex] == 0) {
                // Throw away the '%' character if it's at the end of the string.
                break;
            }
            if (format[stringIndex] == 'd' || format[stringIndex] == 'i') {
                int number = va_arg(list, int);
                int digits = Numbers::DecDigits(number);
                if (padAmount > 0) {
                    for (int i = padAmount - digits; i > 0; --i) {
                        Put(' ');
                    }
                }

                PutFormatted(Numbers::Dec(number));
                bytesWritten += digits;
            } else if (format[stringIndex] == 'x' || format[stringIndex] == 'X') {
                int number = va_arg(list, int);
                int digits = Numbers::HexDigits(number);
                if (padAmount > 0) {
                    for (int i = padAmount - digits; i > 0; --i) {
                        if (padZero) {
                            Put('0');
                        } else {
                            Put(' ');
                        }
                    }
                }

                PutFormatted(Numbers::Hex(number, digits));
                bytesWritten += digits;
            } else if (format[stringIndex] == 'c') {
                char character = (char) (va_arg(list, int));
                Put(character);
                bytesWritten++;
            } else if (format[stringIndex] == 's') {
                char * string = (char *) (va_arg(list, int));
                while (*string != 0) {
                    Put(*string++);
                    bytesWritten++;
                }
            } else if (format[stringIndex] == '%') {
                Put('%');
                bytesWritten++;
            }

        } else {
            Put(format[stringIndex]);
            bytesWritten++;
        }
    }

    va_end(list);
    return bytesWritten;
}

void Serial::GetFlush(void) {
    rx_tail_ = rx_head_;
}

int Serial::Get(const int timeout) {
    int rxbyte = 0;

    if (timeout <= -1) { //No timeout, wait forever

        while ((rxbyte = CheckBuffer()) < 0);
        return (char) rxbyte;
    } else {
        unsigned int total_cycles = (CLKFREQ / 1000) * timeout;
        unsigned int elapsed_cycles = 0;

        int previous_cnt = CNT;
        do {
            rxbyte = CheckBuffer();
            int current_cnt = CNT;
            elapsed_cycles += current_cnt - previous_cnt;
            previous_cnt = current_cnt;
        } while (rxbyte < 0 && elapsed_cycles < total_cycles);
        return rxbyte;
    }
}

int Serial::Get(char * const buffer, const int length, const int timeout) {
    int character_count;
    for (character_count = 0; character_count < length; ++character_count) {
        int character = Get(timeout);
        if (character <= -1) {
            return character_count;
        }
        buffer[character_count] = (char) character;
    }
    return character_count;
}

int Serial::Get(char * const buffer, const char terminator) {
    int character_count;
    char received_character = terminator + 1; // guarantee that they differ the first time.
    for (character_count = 0; received_character != terminator; ++character_count) {
        received_character = (char) Get();
        buffer[character_count] = received_character;
    }
    buffer[character_count] = '\0';
    return character_count;
}

int Serial::GetCount(void) {
    int tail = rx_tail_;
    int head = rx_head_;
    if (head >= tail) {
        return head - tail;
    } else {
        return kBufferLength - tail + head;
    }

}

int Serial::CheckBuffer(void) {
    int rxbyte = -1;
    if (rx_tail_ != rx_head_) {
        rxbyte = rx_buffer_[rx_tail_];
        rx_buffer_[rx_tail_] = 0;
        rx_tail_ = ((rx_tail_ + 1) % kBufferLength);
    }
    return rxbyte;
}
