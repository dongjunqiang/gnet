#ifndef GNET_BUFFER_H_
#define GNET_BUFFER_H_

#include <stdlib.h>

namespace gnet {

class Buffer
{
public:
    Buffer(int size = (1 << 20));
    ~Buffer();

    int Read(int len, char* dst = NULL);
    int Peek(int len, char* dst = NULL);
    int Write(int len, const char* src = NULL);

    void Reset();

    int Rlen() { return wpos_ > rpos_ ? wpos_ - rpos_ : 0; }
    int Wlen() { return size_ - wpos_; }
    char* Rbuf() { return data_ + rpos_; }
    char* Wbuf() { return data_ + wpos_; }

private:
    int rpos_;
    int wpos_;
    int size_;
    char* data_;
};

}

#endif
