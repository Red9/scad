/** Convert between numbers and the various string representations.
@author SRLM
@date   2013-01-21
*/

#ifndef SRLM_PROPGCC_NUMBERS_H_
#define SRLM_PROPGCC_NUMBERS_H_

#include <string.h>


class Numbers{
public:
/** Converts a decimal number to it's string representation.
  * 
  * @warning This function modifies the given string.
  * @warning Converted integers must be in the range of –2,147,483,648 to
  *          2,147,483,647 (32 bit signed integer). This range is not checked.
  *
  * Note: Effectively the same as the C atoi.
  *
  * 
  *
  * @param n  The 32 bit 2's complement number to convert.
  * @param s  The address to store the string representation.
  * @returns  The starting address of the null terminated string.
  */

static char * Dec(int n, char s[] = buffer);

/** Converts the string representation of a decimal number to it's value.
  * 
  * 
  * @param number     the string representation of a  base 10 number.
  *                   Valid characters are '-' (optional) followed by '0'
  *                   through '9'.
  * @param terminator optionally specify a termination character to end the
  *                   string.
  * @returns          the 32 bit value of the representation.
  */
static int Dec(const char * number, char terminator = 0);

/** Converts a hex number to it's string representation.
  *
  * @warning This function modifies the given string.
  * @warning Converted integers must be 32 bit integers. This range is not
  *          checked. 
  *
  * @param n  The 32 bit number to convert.
  * @param digits The number of Hex digits to print.
  * @param s  The address to store the string representation. Defaults to
  *           the internal buffer.
  * @returns  The starting address of the string pointer.
  */
static char * Hex(int n, int digits, char s[] = buffer);


/** Converts a binary number to it's string representation.
  *
  * @warning This function modifies the given string.
  * @warning Converted integers must be 32 bit integers. This range is not
  *          checked. 
  *
  * @param n  The 32 bit number to convert.
  * @param digits The number of binary digits to print.
  * @param s  The address to store the string representation. Defaults to
  *           the internal buffer.
  * @returns  The starting address of the string pointer.
  */
static char * Bin(int n, int digits, char s[] = buffer);


/**


@returns the number of digits that a call to Dec(int) with @a n will produce.
*/
static int DecDigits(int n);

/**
@returns the number of digits that a call to Hex(int, ...) with @a n will
produce if there are no leading 0s (ie, counts all the hex digits from left to
right, starting with the first non-zero hex digit).
*/
static int HexDigits(int n);



/** Reverse the order of a string's characters, in place.
  * @warning This function modifies the given string.
  * 
  * @param s zero teminated string to reverse
  * @returns the reversed string starting address
  */
static char * Reverse(char s[]);

private:



/** The internal string buffer. Note: holds at most one string, so multiple
  * sequential calls will overwrite the buffer's previous contents.
  */
static char buffer[]; 


};




#endif // SRLM_PROPGCC_NUMBERS_H_
