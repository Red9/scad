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

class PIB{
public:



/** Put 3 numbers 2 bytes wide into the buffer.

@param buffer     The buffer object to add the data to.
@param identifier A single byte identifer for the element
@param cnt        The element timestamp.
@param a          Number 0, will be masked to 16 bits.
@param b          Number 1, will be masked to 16 bits.
@param c          Number 2, will be masked to 16 bits.
*/
static void _3x2(ConcurrentBuffer * buffer, char identifier, unsigned int cnt,
		unsigned int a, unsigned int b, unsigned int c);

/** Put 3 numbers 4 bytes wide into the buffer.

@param buffer     The buffer object to add the data to.
@param identifier A single byte identifer for the element
@param cnt        The element timestamp.
@param a          Number 0
@param b          Number 1
@param c          Number 2
*/
static void _3x4(ConcurrentBuffer * buffer, const char identifier,
                   const unsigned int cnt,
                   const int a, const int b, const int c);

/** Put 2 numbers 4 bytes wide into the buffer.

@param buffer     The buffer object to add the data to.
@param identifier A single byte identifer for the element
@param cnt        The element timestamp.
@param a          Number 0
@param b          Number 1
*/
static void _2x4(ConcurrentBuffer * buffer, const char identifier,
                   const unsigned int cnt, const int a, const int b);

/** Put a terminator ended string into the buffer.

@param buffer     The buffer object to add the data to.
@param identifier A single byte identifer for the element
@param cnt        The element timestamp.
@param string     The data to store.
@param terminator All the data in @a string is stored up to, but not including,
                  this character.
*/
static void _string(ConcurrentBuffer * buffer, char identifier, unsigned int cnt,
		const char * string, char terminator);



};



#endif // SRLM_PROPGCC_PIB_H__
