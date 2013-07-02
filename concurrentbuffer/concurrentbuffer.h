/** A multithreaded circular buffer.
 * 
 * This class implements a multithreaded circular buffer. When an instance is
 * created, a single circular buffer is created. Instances can then add to the
 * global buffer, and read from it. 
 * 
 * @warning: you can have only one buffer per project. You may have multiple
 * views on it (as represented by an instance of this class), but there is one a 
 * single buffer for all the data. If you want more then you'll have to make 
 * copies of the class.
 * 
 * Future Feature List:
 *      - make a version of Get that has timeout, and a version to get strings
 *      - make the "Put" functions static.
 *      - add a semaphore system to automatically deallocate the lock when the last instance is destroyed.
 * @author SRLM (srlm@srlmproductions.com)
 * @date   2013-06-04
 * @version 1.0
 * 
 * Version History
 * + 1.0 Cleaned up tests, fixed kSize and getFree(), refactored to coding standards.
 * + 0.1 Initial creation
 */



#ifndef SRLM_PROPGCC_CONCURRENTBUFFER_H_
#define SRLM_PROPGCC_CONCURRENTBUFFER_H_

#include <propeller.h>

class ConcurrentBuffer {
public:

    ConcurrentBuffer();
    
    /**
     * @param new_timeout The maximum amount of time to wait for a @a Put(), in 
     *      microseconds. Default is 1 second timeout.
     * @return true if successfully started, false otherwise.
     */
    static bool Start(int timeout_in_us = 1000000);
    
    
    /** Stops the concurrent buffer. Should only be called when all there is no
     * chance of a Put (or a Get!).
     * 
     */
    static void Stop();

    /**Put a byte in the buffer.
     * 
     * This function is atomic: no matter how many instances of this class try to
     * put into the buffer at the same time, only one will be allowed access at a
     * time. The others wait for their turn.
     * 
     * @param data the byte of data to add.
     * @returns true if data is added to the buffer, false if timeout occurs
     */
    static bool Put(char data);

    /** Put an array of bytes in the buffer.
     * 
     * This function is atomic: no matter how many instances of this class try to 
     * put into the buffer at the same time, only one will be allowed access at a 
     * time. The others wait for their turn.
     * 
     * @param data The array of bytes to add.
     * @param length The number of bytes to add,
     * @returns true if data is added to the buffer, false if timeout occurs
     */
    static bool Put(const char data[], const int size);

    /** @a Put with a string appended to the end.
     * 
     * Will log at least one character. Logs terminator character as final char.
     * 
     * This function is useful for when there is some binary data (for example, a 
     * time stamp) and some textual data (for example, a GPS string). This function 
     * is like @a Put(array), but will append a string on the end.
     * 
     */
    static bool PutWithString(const char data[], const int size, const char * string, char terminator = '\0');


    /**Get a byte from the buffer. Will block forever
     * 
     * @warning @Get() must be called often enough so that the get operations keep 
     * up with the put() operations. Otherwise, data is lost (for this instance of 
     * the class.
     */
    char Get();

    /** Get an array of bytes from the buffer. Will not block.
     * 
     * Note that, if no bytes are added to the buffer, this function will return all 
     * bytes in at most 2 calls.
     * 
     * @warning @Get() must be called often enough so that the get operations keep 
     * up with the put() operations. Otherwise, data is lost (for this instance of 
     * the class.
     * 
     * @warning The contents of the array may change if another thread adds too 
     * much data to the buffer. If this function is used, the data must be consumed
     * as rapidly as possible.
     * 
     * @param bytes The pointer to redirect to point to the bytes.
     * @returns     The number of valid bytes in the @a bytes array. If no bytes 
     *                      are available, then 0 is returned.
     */
    int Get(volatile char *& bytes);

    /** Resets head pointer. This function affects all buffer instances!
     * 
     * @warning should not call when there are buffers instances: the tail will not be
     * reset.
     */
    static void ResetHead(void);
    
    /** "Clear" the buffer for this specific instance.
     * 
     */
    void ResetTail(void);


    /** Get the number of unused bytes in the buffer.
     * 
     * @warning This only returns the number of bytes free for this instance. Other 
     * instances of this class may have different numbers of bytes free.
     * 
     * @returns the number of free bytes (corrected difference of head and tail) 
     * Note: if there is nothing in the buffer, then it is equal to kSize
     */
    int GetFree();

    /** Get the total usable buffer size.
     * 
     * @returns the available size of the buffer.
     */
    static int GetkSize() {
        return kSize - 1; // -1 because one spot needs to be used for record keeping.
    };


    /** Get exclusive write access to the buffer.
     * 
     * Generally, these methods should not be used except in rare cases. One 
     * such case is to prevent (temporarily) any data being added to the buffer. 
     * In that case, @a Lockset() the static buffer instance, then do the 
     * protected things, then @a Lockclear().
     * 
     * @returns true if lock is acquired, false if timeout occurs.
     */
    static bool Lockset();
    static void Lockclear();

private:

    static const int kSize = 901; //Can be any size, doesn't have to be a multiple of 2
    static volatile char buffer_ [kSize];
    static bool initialized_;
    static volatile int head_; //Points to the next free byte
    static unsigned int timeout_;
    
    int tail_; //Points to the next byte to read

    static void StoreByte(char data);

    

public:
    /**
     * @warning lock is made public so that it can be accessed for testing. Should 
     * not be called directly otherwise!
     */
    static int lock_;

};


#endif // SRLM_PROPGCC_CONCURRENTBUFFER_H_
