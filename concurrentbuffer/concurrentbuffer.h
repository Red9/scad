/** A multithreaded circular buffer.

This class implements a multithreaded circular buffer. When an instance is
created, a single circular buffer is created. Instances can then add to the
global buffer, and read from it. 

@warning: you can have only one buffer per project. You may have multiple views
on it (as represented by an instance of this class), but there is one a single
buffer for all the data.

@author SRLM (srlm@srlmproductions.com)
@data   2013-01-01
*/



#ifndef SRLM_PROPGCC_CONCURRENTBUFFER_H_
#define SRLM_PROPGCC_CONCURRENTBUFFER_H_

#include <propeller.h>

class ConcurrentBuffer{




public:
	
/**
@param new_timeout The maximum amount of time to wait for a @a Put() or Get(),
                   in microseconds. Default is 1 second timeout.
*/
ConcurrentBuffer(int new_timeout=1000000);

/**Put a byte in the buffer.

This function is atomic: no matter how many instances of this class try to put
into the buffer at the same time, only one will be allowed access at a time. The
others wait for their turn.

@param data the byte of data to add.
@returns true if data is added to the buffer, false if timeout occurs
*/
bool Put(char data);

/** Put an array of bytes in the buffer.

This function is atomic: no matter how many instances of this class try to put
into the buffer at the same time, only one will be allowed access at a time. The
others wait for their turn.
@param data The array of bytes to add.
@param length The number of bytes to add,
@returns true if data is added to the buffer, false if timeout occurs
*/
bool Put(const char data[], const int size);

/**
Will log at least one character. Logs terminator character as final char.
*/
bool Put(const char data[], const int size, const char * string, char terminator = '\0');


/**Get a byte from the buffer. Will block forever

@warning @Get() must be called often enough so that the get operations keep up
         with the put() operations. Otherwise, data is lost (for this instance
         of the class.

@todo(SRLM): make a version of Get that has timeout, and a version to get strings
*/
char Get();

/** Get an array of bytes from the buffer. Will not block.

Note that, if no bytes are added to the buffer, this function will return all
bytes in at most 2 calls.

@warning @Get() must be called often enough so that the get operations keep up
         with the put() operations. Otherwise, data is lost (for this instance
         of the class.
         
@warning The contents of the array may change if another thread adds too much
         data to the buffer. If this function is used, the data must be consumed
         as rapidly as possible.

@param bytes    The pointer to redirect to point to the bytes.
@returns        The number of valid bytes in the @a bytes array. If no bytes
                are available, then 0 is returned.
*/
int Get(volatile char *& bytes);

/**
Resets Head pointer.
@warning should not call when there are buffers in use: the tail will not be
reset.
*/
static void ResetHead();


/** Get the number of bytes free.

@warning This only returns the number of bytes free for this instance. Other
instances of this class may have different numbers of bytes free.

@todo(SRLM): I think this should automatically account for the -1 difference...

@returns the number of free bytes (corrected difference of head and tail) Note:
if there is nothing in the buffer, then it is kSize -1 (less one because it
needs at least one empty space to tell if head and tail are different).
*/
int GetFree();

/** Get the buffer size.

@returns the size of the buffer.
*/
static int GetkSize(){return kSize;};
private:

unsigned int timeout;
static const int kSize = 512; //Can be any size, doesn't have to be a multiple of 2
volatile static char buffer [kSize];
static bool initialized;


volatile static int head; //Points to the next free byte
int tail;                 //Points to the next byte to read

void StoreByte(char data);

/** @returns true if lock is acquired, false if timeout occurs.
 */
bool Lockset();
void Lockclear();

public:
/**
@warning lock is made public so that it can be accessed for testing. Should not
be called directly otherwise!
*/
static int  lock;

};


#endif // SRLM_PROPGCC_CONCURRENTBUFFER_H_
