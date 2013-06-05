/**

Based on Fast Full-Duplex Serial 1 (FFDS1) version 0.9 by Jonathan Dummer
(lonesock). C++ Port done by SRLM.

Serial provides a fast and stable serial interface using a single cog.

Max baudrate = clkfreq / (86 * 2)

    Clock  | MaxBaud | Standard
    -------+---------+---------
    96 MHz | 558_139 | 500_000    - 6MHz XTAL at 16x PLL
    80 MHz | 465_116 | 460_800    - 5MHz XTAL at 16x PLL (most common)
    12 MHz |  69_767 |  57_600    - approx RCFAST
    20 kHz |     116 | hah hah    - approx RCSLOW

Bit period is calculated to the nearest 2 clocks. So, the bit period should be
within 1 clock, or 12.5 ns at 80 MHz.

@author SRLM
@date   2013-06-04
@version 1.2

Version History
- 1.2
- 1.1 Added CTS support to suppress tx output from Propeller (2013-04-11)
- 1.0 Initial creation
 */


#ifndef SRLM_PROPGCC_SERIAL_H_
#define SRLM_PROPGCC_SERIAL_H_

#include <cstdarg>
#include <propeller.h>
#include "numbers.h"

class Serial {
public:

    /** Size of the RX buffer in bytes. No restrictions on size (well, Hub RAM 
     * limits [8^) Does not need to be a power of 2. There is no TX buffer, 
     * Serial sends directly from Hub RAM.
     * 
     * For some reason, 117 seems to be the minimum buffer size before the tests
     * start to lock up. Originally, it was 512. Seems safe to change, as long
     * as it's >= 117.
     */
    static const int kBufferLength = 512;

    ~Serial();

    /** Start Serial driver in a new cog.
     * 
     * Set any pin to -1 to disable it. No pin can match any other pin.
     * 
     * @param rxpin the pin [0..31] for incoming data.
     * @param txpin the pin [0..31] for outgoing data.
     * @param rate  the initial baud rate in bits per second.
     * @param ctspin  cts is an input for control of flow from the tx pin. If high, it disables transmissions.
     * @return  Returns true if the cog started OK, false otherwise
     */
    bool Start(const int rxpin, const int txpin, const int rate, const int ctspin = -1);

    /** Stops the Serial PASM engine, if it is running. Frees a cog.
     */
    void Stop(void);

    /** Does a live update of the baud rate in the Serial engine.
     * 
     * @param rate desired baud rate of the Serial engine.
     * @return     true if the baud rate was achieved
     */
    bool SetBaud(const int rate);

    /** Does a live update of the baud rate in the Serial engine.
     * 
     * Examples:
     *          got_desired_rate = SetBaudClock( 9600, CLKFREQ )
     *          got_desired_rate = SetBaudClock( 9600, actual_sys_clock_freq )
     * 
     * @param rate     desired baud rate of the Serial engine.
     * @param sysclock the system clock (use CLKFREQ), or provide your own
     *                  (useful when using RCFAST).
     * @return        true if the baud rate was achieved
     */
    bool SetBaudClock(const unsigned int rate, const unsigned int sysclock);


    /** Transmit a single character.
     * 
     * @param character the byte to send.
     */
    void Put(const char character);


    /** Transmit a set of bytes.
     * 
     * @param buffer_ptr The array of bytes to send.
     * @param count      The number of bytes to send.
     * @return           The number of bytes actually sent. An error has
     *                          occurred if this is less than strlen(buffer_ptr)
     */
    int Put(const char * buffer_ptr, const int count);

    /** Transmit a C string.
     * 
     * @param buffer_ptr The null terminated string to put. Note: does not 
     *                          transmit the trailing null.
     * @return The number of bytes actually sent. An error has occurred if this 
     *                          is less than strlen(buffer_ptr)
     */
    int Put(const char * buffer_ptr);

    // SRLM: Put(buffer) has a bug in it's implementation. I don't know what it is.
    //int PutBuffer(char * buffer_ptr, const bool wait = false, int buffer_bytes = -1, const char terminator = '\0');

    /** Transmit a string (printf function-alike).
     *
     * This function is based on the requirements found on this page:
     * http://www.cplusplus.com/reference/cstdio/printf/
     * 
     * @warning This function does not implement the full printf functionality.
     * 
     * Formatters must follow the following prototype:
     * %[flags][width]specifier
     * The following flags are supported
     * - 0 (only with x,X,b,B). Sets padding to the 0 character instead of space.
     * This function supports the following specifiers:

    
     * - d or i : signed decimal integer. The width specifier will pad with 
     *          spaces if necessary.
     * - x or X : hexadecimal integer. All caps (with either). The width
     *          specifier will pad with spaces (or 0s if the 0 flag is set) if
     *          necessary
     * - c      : output a single character.
     * - s      : output a string of characters, 0 terminated.
     * - %      : output a % symbol.
     * 
     * Each specifier must have a matching typed optional argument.
     * 
     * Behavior is undefined if % is used without a specifier.
     * 
     * @param format the string to send, optionally with specifiers.
     * @param ...    additional arguments. Depending on the format string, the
     *                  function may expect a sequence of additional arguments.
     * @returns      on success, the total number of characters written. On
     *                  error, a negative number is returned.
     */
    int PutFormatted(const char * format, ...);


    /** Receive a byte (wait) or timeout.
     * 
 
    
     * @warning This function may block indefinitely if timeout is set to a
     *                  negative value, and no data is received.
     * 
     * The timeout duration is a minimum, not a precise number. Experimental 
     * results indicate that a timeout of 10ms results in a total function time 
     * of 10.047ms.
     * 
     * @param   timeout the number of milliseconds to wait. Defaults to forever 
     *                          wait (negative timeout). Maximum value is 53000 
     *                          milliseconds (no bounds checking).
     * @return          -1 if no byte received, 0x00..0xFF if byte
     */
    int Get(const int timeout = -1);

    /** Get a buffer of characters
     * 
     * @param buffer  The buffer to store the received characters in.
     * @param length  The maximum number of characters to read (may be less 
     *                  than @a length if @a timeout is enabled.
     * @param timeout The maximum amount of time to wait for _each_ character, 
     *                  _not_ the total time of the function.
     * @returns the number of characters read. If less than @a length, then a
     *                  timeout occurred.
     */
    int Get(char * const buffer, const int length, const int timeout = -1);

    /** Get a terminated string of characters
     * 
     * @param buffer     The buffer to store the received characters in. Note:
     *                          an extra '\0' is always added to the end (null 
     *                          terminate).
     * @param terminator The last character to put into the buffer. On the 
     *                          first instance of this character, the function 
     *                          returns.
     * @returns          The number of characters read, including terminator. 
     *                          Does not include null terminator.
     */
    int Get(char * const buffer, const char terminator = '\n');

    /** Flushes receive buffer.
     */
    void GetFlush(void);


    /** Get the number of bytes in the receive buffer.
     */
    int GetCount(void);


private:
    /**
    Half period must be at least this, otherwise the cog will
    sleep for a whole counter cycle (2^32 / clkfreq seconds).
     */
    static const int kMinimumHalfPeriod = 86;
    
    volatile int write_buf_ptr_;
    volatile int send_temp_;
    volatile int half_bit_period_;
    volatile short rx_head_;
    volatile short rx_tail_;
    volatile char rx_buffer_[kBufferLength];
    int cog_;

    /** Checks if byte is waiting in the buffer, but doesn't wait.
     * @return -1 if no byte received, 0x00..0xFF if byte
     */
    int CheckBuffer(void);

    void WaitForTransmissionCompletion(void);

    void SetDriverLong(const int index, const int value);
    void SetDriverLong(char ** index, const int value);
};

#endif //SRLM_PROPGCC_SERIAL_H_
