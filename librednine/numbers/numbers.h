/** Convert between numbers and the various string representations.
@author SRLM
@date   2013-01-21
 */

#ifndef SRLM_PROPGCC_NUMBERS_H_
#define SRLM_PROPGCC_NUMBERS_H_

#include <string.h>

class Numbers {
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

    static char * Dec(int n, char s[] = NULL) {
        if (s == NULL) {
            s = internalBuffer;
            //s = internalBuffer();
        }
        //Source: http://www.cplusplus.com/articles/D9j2Nwbp/
        int i, sign;

        if ((sign = n) < 0) /* record sign */
            n = -n; /* make n positive */
        i = 0;
        do { /* generate digits in reverse order */
            s[i++] = n % 10 + '0'; /* get next digit */
        } while ((n /= 10) > 0); /* delete it */
        if (sign < 0)
            s[i++] = '-';
        s[i] = '\0';
        Reverse(s);

        return s;
    }

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
    static int Dec(const char * number, char terminator = 0) {
        int result = 0;
        int index = 0;
        bool isNegative = false;
        if (number[index] == '-') {
            isNegative = true;
            index++;
        }
        for (; number[index] != terminator; index++) {
            //       Shift left      Add 1's unit
            result = (result * 10) + (number[index] - '0');
        }

        if (isNegative) {
            return -result;
        } else {
            return result;
        }
    }

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
    static char * Hex(int n, int digits, char s[] = NULL) {
        if (s == NULL) {
            s = internalBuffer;
            //s = internalBuffer();
        }
        const char characters[] = "0123456789ABCDEF";
        int i;
        for (i = 0; i < digits; i++) {
            s[i] = characters[n & 0xF];
            n >>= 4;
        }
        s[i] = 0;
        Reverse(s);
        return s;
    }

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
    static char * Bin(int n, int digits, char s[] = NULL) {
        if (s == NULL) {
            s = internalBuffer;
            //s = internalBuffer();
        }
        // Below is C++ version:
        const char characters[] = "01";
        int i;
        for (i = 0; i < digits; i++) {
            s[i] = characters[n & 0b1];
            n >>= 1;
        }
        s[i] = 0;
        Reverse(s);
        return s;

    }

    /**


    @returns the number of digits that a call to Dec(int) with @a n will produce.
     */
    static int DecDigits(int n) {
        int sign = 0;
        if (n < 0) {
            n = -n;
            sign = 1;
        }
        if (n < 100000) {
            // 5 or less
            if (n < 100) {
                // 1 or 2
                if (n < 10)
                    return 1 + sign;
                else
                    return 2 + sign;
            } else {
                // 3 or 4 or 5
                if (n < 1000)
                    return 3 + sign;
                else {
                    // 4 or 5
                    if (n < 10000)
                        return 4 + sign;
                    else
                        return 5 + sign;
                }
            }
        } else {
            // 6 or more
            if (n < 10000000) {
                // 6 or 7
                if (n < 1000000)
                    return 6 + sign;
                else
                    return 7 + sign;
            } else {
                // 8 to 10
                if (n < 100000000)
                    return 8 + sign;
                else {
                    // 9 or 10
                    if (n < 1000000000)
                        return 9 + sign;
                    else
                        return 10 + sign;
                }
            }
        }
    }

    /**
    @returns the number of digits that a call to Hex(int, ...) with @a n will
    produce if there are no leading 0s (ie, counts all the hex digits from left to
    right, starting with the first non-zero hex digit).
     */
    static int HexDigits(int n) {
        if ((n & 0xFFFF) == n) {
            //4 or less
            if ((n & 0xFF) == n) {
                //2 or less
                if ((n & 0xF) == n)
                    return 1;
                else
                    return 2;
            } else
                //4 or 3
                if ((n & 0xFFF) == n)
                return 3;
            else
                return 4;

        } else {
            //5 or more
            if ((n & 0xFFFFFF) == n) {
                //6 or 5
                if ((n & 0xFFFFF) == n)
                    return 5;
                else
                    return 6;
            } else {
                if ((n & 0xFFFFFFF) == n)
                    return 7;
                else
                    return 8;
            }
        }
    }

    /** Reverse the order of a string's characters, in place.
     * @warning This function modifies the given string.
     * 
     * @param s zero teminated string to reverse
     * @returns the reversed string starting address
     */
    static char * Reverse(char s[]) {
        //Source: http://www.cplusplus.com/articles/D9j2Nwbp/
        int i, j;
        char c;

        for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
            c = s[i];
            s[i] = s[j];
            s[j] = c;
        }
        return s;
    }

private:

    /** The internal string buffer. Note: holds at most one string, so multiple
     * sequential calls will overwrite the buffer's previous contents.
     */
    /*static char * internalBuffer() {
        static char buffer[33]; //33 for 32 binary digits and '\0'.
        return buffer;
    }*/
    
    static char internalBuffer[];


};



//char * Numbers::Dec(int input_num, char * string){
//	char * starting_string = string;
//	int count, divisor;

//	if(input_num == 0){
//		string[0] = '0';
//		string[1] = '\0';
//		return starting_string;
//	}
//	
//	if(input_num < 0){
//		input_num = -input_num;
//		string[0] = '-';
//		string++;
//	}
//               
//	__asm__ volatile(
//            "brw	#start_							\n\t"

//            "divisor_array: "
//            "long	1000000000						\n\t"
//			"long	100000000						\n\t"
//			"long	10000000						\n\t"
//			"long	1000000							\n\t"
//			"long	100000							\n\t"
//			"long	10000							\n\t"
//			"long	1000							\n\t"
//			"long	100								\n\t"
//			"long	10								\n\t"
//			"long	1								\n\t"

//			"start_: "
//			"mvi	r0, #divisor_array				\n\t"   
//			
//			"start_digit: "
//            "rdlong %[divisor], r0					\n\t" /* Figure out what the first divisor is. */
//            "cmp	%[input_num], %[divisor] wc		\n\t"
//	"if_c	 add	r0, #4							\n\t"
//	"if_c	 brw	#start_digit					\n\t"

//            "main_loop: "
//            "rdlong %[divisor], r0					\n\t" /* Read the current decimal divisor (10, 1000, 10000, etc.)*/
//        
//            "mov	%[count], #0					\n\t" /*Clear counter*/
//			
//			"count_digit: "
//			"cmp	%[input_num], %[divisor] wc		\n\t"
//	"if_nc   sub	%[input_num], %[divisor]		\n\t"
//	"if_nc   add	%[count], #1					\n\t"
//	"if_nc   brw    #count_digit					\n\t"
//	
//			"add	%[count], #48					\n\t" /*Convert to ascii representation*/
//			"wrbyte %[count], %[string]				\n\t" /*Store result*/
//	
//			"add	%[string], #1					\n\t" /*increment to next char address*/
//			
//			"cmp	%[divisor], #1 wz				\n\t" /*Are we at the last digit?*/
//	"if_nz	 add	r0, #4							\n\t"
//	"if_nz	 brw	#main_loop						\n\t"
//	
//			"mov	%[count], #0					\n\t" /*Null terminate */
//			"wrbyte %[count], %[string]				\n\t"
//         
//	: /*outputs (+inputs) */
//		[string] "+r" (string)
//		
//	: /*inputs */
//		[input_num] "r" (input_num),
//		[count] "r" (count),
//		[divisor] "r" (divisor)
//	
//	: /*clobber */
//		"r0"
//	);
//	
//	return starting_string;
//}

#endif // SRLM_PROPGCC_NUMBERS_H_
