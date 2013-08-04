#ifndef SRLM_PROPGCC_PIB_H__
#define SRLM_PROPGCC_PIB_H__

#include "concurrentbuffer.h"

/** Put elements into the buffer.

This class provides a convenient method to put elements into the buffer. It is
based on the Red9 SCAD element format, which is:

[Identifier] [CNT] [Data...]

Identifier is 1 byte, CNT is 4 bytes, and Data is element specific. For this
class, data size is indicated by the function names, which are of the format:

[count of numbers] x [number size in bytes]

All numbers are stored in Little Endian format.

@author SRLM (srlm@srlmproductions.com)
@date   2013-03-28
@version 1.0

Version History
+ 1.0 Initial creation (pulled from SCAD code).

 */

class PIB {
public:

    /** Put 3 numbers 2 bytes wide into the buffer.

    @param buffer     The buffer object to add the data to.
    @param identifier A single byte identifier for the element
    @param cnt        The element timestamp.
    @param a          Number 0, will be masked to 16 bits.
    @param b          Number 1, will be masked to 16 bits.
    @param c          Number 2, will be masked to 16 bits.
     */
    static void _3x2(char identifier, unsigned int cnt,
            unsigned int a, unsigned int b, unsigned int c) {
        const int data_size = 11;
        char data[data_size];
        data[0] = identifier;

        //Little endian
        data[4] = (cnt & 0xFF000000) >> 24;
        data[3] = (cnt & 0xFF0000) >> 16;
        data[2] = (cnt & 0xFF00) >> 8;
        data[1] = (cnt & 0xFF) >> 0;

        // Two byte mode
        data[6] = (a & 0xFF00) >> 8;
        data[5] = (a & 0xFF) >> 0;

        data[8] = (b & 0xFF00) >> 8;
        data[7] = (b & 0xFF) >> 0;

        data[10] = (c & 0xFF00) >> 8;
        data[ 9] = (c & 0xFF) >> 0;

        while (ConcurrentBuffer::Put(data, data_size) == false) {
        }
    }

    /** Put 3 numbers 4 bytes wide into the buffer.

    @param buffer     The buffer object to add the data to.
    @param identifier A single byte identifier for the element
    @param cnt        The element timestamp.
    @param a          Number 0
    @param b          Number 1
    @param c          Number 2
     */
    static void _3x4(const char identifier,
            const unsigned int cnt,
            const int a, const int b, const int c) {
        const int data_size = 17;
        char data[data_size];
        data[0] = identifier;

        //Little endian
        data[4] = (cnt & 0xFF000000) >> 24;
        data[3] = (cnt & 0xFF0000) >> 16;
        data[2] = (cnt & 0xFF00) >> 8;
        data[1] = (cnt & 0xFF) >> 0;

        data[8] = (a & 0xFF000000) >> 24;
        data[7] = (a & 0xFF0000) >> 16;
        data[6] = (a & 0xFF00) >> 8;
        data[5] = (a & 0xFF) >> 0;

        data[12] = (b & 0xFF000000) >> 24;
        data[11] = (b & 0xFF0000) >> 16;
        data[10] = (b & 0xFF00) >> 8;
        data[9] = (b & 0xFF) >> 0;

        data[16] = (c & 0xFF000000) >> 24;
        data[15] = (c & 0xFF0000) >> 16;
        data[14] = (c & 0xFF00) >> 8;
        data[13] = (c & 0xFF) >> 0;

        while (ConcurrentBuffer::Put(data, data_size) == false) {
        }
    }

    /** Put 2 numbers 4 bytes wide into the buffer.

    @param buffer     The buffer object to add the data to.
    @param identifier A single byte identifier for the element
    @param cnt        The element timestamp.
    @param a          Number 0
    @param b          Number 1
     */
    static void _2x4(const char identifier,
            const unsigned int cnt, const int a, const int b) {
        const int data_size = 13;
        char data[data_size];
        data[0] = identifier;

        //Little endian
        data[4] = (cnt & 0xFF000000) >> 24;
        data[3] = (cnt & 0xFF0000) >> 16;
        data[2] = (cnt & 0xFF00) >> 8;
        data[1] = (cnt & 0xFF) >> 0;

        data[8] = (a & 0xFF000000) >> 24;
        data[7] = (a & 0xFF0000) >> 16;
        data[6] = (a & 0xFF00) >> 8;
        data[5] = (a & 0xFF) >> 0;

        data[12] = (b & 0xFF000000) >> 24;
        data[11] = (b & 0xFF0000) >> 16;
        data[10] = (b & 0xFF00) >> 8;
        data[9] = (b & 0xFF) >> 0;

        while (ConcurrentBuffer::Put(data, data_size) == false) {
        }
    }

    /** Put a terminator ended string into the buffer.

    @param buffer     The buffer object to add the data to.
    @param identifier A single byte identifier for the element
    @param cnt        The element timestamp.
    @param string     The data to store.
    @param terminator All the data in @a string is stored up to, but not including,
                      this character.
     */
    static void _string(char identifier, unsigned int cnt,
            const char * string, char terminator) {
        const int data_size = 5;
        char data[data_size];
        data[0] = identifier;

        //Little endian
        data[4] = (cnt & 0xFF000000) >> 24;
        data[3] = (cnt & 0xFF0000) >> 16;
        data[2] = (cnt & 0xFF00) >> 8;
        data[1] = (cnt & 0xFF) >> 0;

        while (ConcurrentBuffer::PutWithString(data, data_size, string, terminator) == false) {
        }

    }



};



#endif // SRLM_PROPGCC_PIB_H__
