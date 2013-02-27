#ifndef SRLM_PROPGCC_CONCURRENTBUFFER_H_
#define SRLM_PROPGCC_CONCURRENTBUFFER_H_

#include <propeller.h>

class ConcurrentBuffer{




public:
	
	/**
	* @param timeout The maximum amount of time to wait for a Put() or Get(), in
	*                microseconds. Default is 1 second timeout.
	*/
	ConcurrentBuffer(int new_timeout=1000000);
	
	/** is atomic
	 *  @returns true if data is added to the buffer, false if timeout occurs
	 */
	bool Put(char data);
	bool Put(char data[], int size);
	
	
	/**
	* Will wait forever
	*/
	char Get();
	
	
	/**
	  * Resets Head pointer. Note: should not call when there are buffers in use:
	  * the tail will not be reset.
	  */
	static void ResetHead();
	
	
	/**
	 * @returns the number of free bytes (corrected difference of head and tail)
	 * Note: if there is nothing in the buffer, then it is kSize -1 (less one
	 * because it needs at least one empty space to tell if head and tail are
	 * different).
	 */
	int GetFree();

	static int GetkSize(){return kSize;};
private:

	unsigned int timeout;
	static const int kSize = 512; //Can be any size, doesn't have to be a multiple of 2
	volatile static char buffer [kSize];
	static bool initialized;
	
	
	volatile static int head; //Points to the next free byte
	int tail;                 //Points to the next byte to read
	
	void StoreByte(char data);
	
	/**
	 * @returns true if lock is acquired, false if timeout occurs.
	 */
	bool Lockset();
	void Lockclear();

public:
	// lock is made public so that it can be accessed for testing. Should not
	// be called directly otherwise!
	static int  lock;
	
};


#endif // SRLM_PROPGCC_CONCURRENTBUFFER_H_
