#include "concurrentbuffer.h"

const int     ConcurrentBuffer::kSize;
volatile char ConcurrentBuffer::buffer[kSize];
volatile int  ConcurrentBuffer::head = 0;
bool          ConcurrentBuffer::initialized = false;
int           ConcurrentBuffer::lock;

ConcurrentBuffer::ConcurrentBuffer(int new_timeout){
	tail = 0;
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

bool ConcurrentBuffer::Put(char data[], int size){
	if(Lockset() == false){
		return false;
	}
	for(int i = 0; i < size; i++){
		StoreByte(data[i]);
	}
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






















