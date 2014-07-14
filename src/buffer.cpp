#include <string.h>
#include <assert.h>

#include "buffer.h"

using namespace gnet;

Buffer::Buffer(int size)
      : rpos_(0)
      , wpos_(0)
      , size_(size)
      , data_(NULL)
{
    data_ = new char[size];
    assert(data_);
}

Buffer::~Buffer()
{
    delete data_;
}

int Buffer::Read(int len, char* dst)
{
    if (len < 0)
        return -1;
    if (len > rlen())
        return Read(rlen(), dst);
    if (dst)
        memcpy(dst, rbuf(), len);
    rpos_ += len;
    // drift
    if (rpos_ > (size_ >> 1)) {
        const char* shift = rbuf();
        memcpy(data_, shift, rlen());
        wpos_ -= rpos_;
        rpos_ = 0;
    }
    return len;
}

int Buffer::Peek(int len, char* dst)
{
    if (len < 0)
        return -1;
    if (len > rlen())
        return Peek(rlen(), dst);
    if (dst)
        memcpy(dst, rbuf(), len);
    return len;
}

int Buffer::Write(int len, const char* src)
{
    if (len < 0)
        return -1;
    if (wlen() < len)
        return Write(wlen(), src);
    if (src)
        memcpy(wbuf(), src, len);
    wpos_ += len;
    return len;
}

void Buffer::Reset()
{
    rpos_ = wpos_ = 0;
}
