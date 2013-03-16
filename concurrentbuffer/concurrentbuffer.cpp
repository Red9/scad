#include "concurrentbuffer.h"

const int     ConcurrentBuffer::kSize;
volatile char ConcurrentBuffer::buffer[kSize];
volatile int  ConcurrentBuffer::head = 0;
bool          ConcurrentBuffer::initialized = false;
int           ConcurrentBuffer::lock;

ConcurrentBuffer::ConcurrentBuffer(int new_timeout){
	tail = 0;
	
	//TODO(SRLM): How long is this timeout?
	timeout = (CLKFREQ/1000000)*new_timeout;
	
	if(!initialized){
		lock = locknew();
		if(lock != -1){
			initialized = true;
		}
	}
}

void ConcurrentBuffer::StoreByte(char data)
{
	buffer[head++] = data;
	if(head == kSize){
		head = 0;
	}
}

bool ConcurrentBuffer::Lockset(){

	unsigned int endCNT = CNT + timeout;
	while(lockset(lock)){
		if( (int)(endCNT - CNT) < 0){
			return false;
		}
	}
	return true;
}

void ConcurrentBuffer::Lockclear(){
	lockclr(lock);
} 

bool ConcurrentBuffer::Put(char data){
	if(Lockset() == false){
		return false;
	}
	StoreByte(data);
	Lockclear();
	return true;
}

bool ConcurrentBuffer::Put(const char data[], const int size){
	if(Lockset() == false){
		return false;
	}
	for(int i = 0; i < size; i++){
		StoreByte(data[i]);
	}
	Lockclear();
	return true;
}

bool ConcurrentBuffer::Put(const char data[], const int size, const char * string, char terminator){
	if(Lockset() == false){
		return false;
	}
	for(int i = 0; i < size; i++){
		StoreByte(data[i]);
	}
	
	do{
		StoreByte(*string);
	}while(*(string++) != terminator);
	
	Lockclear();
	return true;
}

char ConcurrentBuffer::Get(){
	while(head == tail){}
	
	char result = buffer[tail++];
	if(tail == kSize){
		tail = 0;
	}
	return result;
}

int ConcurrentBuffer::Get(volatile char *& bytes){
	//Read from tail.

	//Case: head == tail
	if(head == tail){
		return 0;
	}
	
	const int kHead = head;	
	int difference = 0;
	
	if(kHead > tail){
		difference = kHead - tail;
	}
	if(kHead < tail){
		difference = kSize - tail;
	}
	
	bytes = & buffer[tail];
	tail += difference;
	
	if(tail == kSize){
		tail = 0;
	}
	
	return difference;
	
	//Case: head < tail (head has wrapped around, but tail hasn't yet.
}

void ConcurrentBuffer::ResetHead(){
	head = 0;
}

int ConcurrentBuffer::GetFree(){
	const int kHead = head;
	if(kHead >= tail){ // no wrap-around
		return kSize - (kHead - tail)-1;
	}else{            // wrap-around
		return tail-kHead-1;
	}
}






















