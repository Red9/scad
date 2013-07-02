#include "concurrentbuffer.h"

const int ConcurrentBuffer::kSize;
volatile char ConcurrentBuffer::buffer_[kSize];
volatile int ConcurrentBuffer::head_ = 0;
int ConcurrentBuffer::lock_ = -1;
unsigned int ConcurrentBuffer::timeout_;

ConcurrentBuffer::ConcurrentBuffer() {
    tail_ = 0;
}

bool ConcurrentBuffer::Start(int timeout_in_us) {

    Stop();

    timeout_ = (CLKFREQ / 1000000) * timeout_in_us;

    if (lock_ == -1) {
        lock_ = locknew();
        if (lock_ != -1) {
            return true;
        }
    }
    return false;
}

void ConcurrentBuffer::Stop() {
    if (lock_ >= 0) {
        lockret(lock_);
        lock_ = -1;
    }
}

void ConcurrentBuffer::StoreByte(char data) {
    buffer_[head_++] = data;
    if (head_ == kSize) {
        head_ = 0;
    }
}

bool ConcurrentBuffer::Lockset() {

    unsigned int end_CNT = CNT + timeout_;
    while (lockset(lock_)) {
        if ((int) (end_CNT - CNT) < 0) {
            return false;
        }
    }
    return true;
}

void ConcurrentBuffer::Lockclear() {
    lockclr(lock_);
}

bool ConcurrentBuffer::Put(char data) {
    if (Lockset() == false) {
        return false;
    }
    StoreByte(data);
    Lockclear();
    return true;
}

bool ConcurrentBuffer::Put(const char data[], const int size) {
    if (Lockset() == false) {
        return false;
    }
    for (int i = 0; i < size; i++) {
        StoreByte(data[i]);
    }
    Lockclear();
    return true;
}

bool ConcurrentBuffer::PutWithString(const char data[], const int size, const char * string, char terminator) {
    if (Lockset() == false) {
        return false;
    }
    for (int i = 0; i < size; i++) {
        StoreByte(data[i]);
    }

    do {
        StoreByte(*string);
    } while (*(string++) != terminator);

    Lockclear();
    return true;
}

char ConcurrentBuffer::Get() {
    while (head_ == tail_) {
    }

    char result = buffer_[tail_++];
    if (tail_ == kSize) {
        tail_ = 0;
    }
    return result;
}

int ConcurrentBuffer::Get(volatile char *& bytes) {
    //Read from tail.

    //Case: head == tail
    if (head_ == tail_) {
        return 0;
    }

    const int kHead = head_;
    int difference = 0;

    if (kHead > tail_) {
        difference = kHead - tail_;
    }
    if (kHead < tail_) {
        difference = kSize - tail_;
    }

    bytes = &buffer_[tail_];
    tail_ += difference;

    if (tail_ == kSize) {
        tail_ = 0;
    }

    return difference;

    //Case: head < tail (head has wrapped around, but tail hasn't yet.)
}

void ConcurrentBuffer::ResetTail(void) {
    if (Lockset() == true) {
        tail_ = head_;
        Lockclear();
    }

}

void ConcurrentBuffer::ResetHead(void) {
    head_ = 0;
}

int ConcurrentBuffer::GetFree(void) {
    const int kHead = head_;
    if (kHead >= tail_) { // no wrap-around
        return kSize - (kHead - tail_) - 1;
    } else { // wrap-around
        return tail_ - kHead - 1;
    }
}






















