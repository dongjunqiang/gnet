#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <functional>

#include "coroutine.h"
#include "reactor.h"
#include "buffer.h"
#include "connector.h"

using namespace gnet;

#define CONNECTOR_STACK (64 << 10)

Connector::Connector(Reactor* reactor, int fd)
         : Handle(reactor)
         , rbuf_(NULL)
         , wbuf_(NULL)
{
    // non block
    int flags = fcntl(fd, F_GETFL, NULL);
    assert(flags >= 0);
    int res = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assert(res == 0);

    // no delay
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    res = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, optlen);
    assert(res == 0);

    fd_ = fd;

    rbuf_ = new Buffer;
    wbuf_ = new Buffer;
    assert(rbuf_ && wbuf_);
}

Connector::~Connector()
{
    delete rbuf_;
    delete wbuf_;
}

int Connector::Send(const char* buffer, int len)
{
    if (wbuf_->Wlen() < len)
        return -1;
    wbuf_->Write(len, buffer);
    reactor_->ModInOut(this, fd_);
    return 0;
}

void Connector::proc_in()
{
    int res = -1;
    while (true) {
        if (rbuf_->Wlen() <= 0) {
            // TODO: full
        }
        res = read(fd_, rbuf_->Wbuf(), rbuf_->Wlen());
        if (res < 0) {
            if (EAGAIN == errno || EINTR == errno) {
                in_->Yield();
            } else {
                // TODO: fail
                delete this;
                return;
            }
        } else if (res == 0) {
            // TODO: disconnect
            delete this;
            return;
        } else {
            rbuf_->Write(res);
            // TODO: read (rbuf_->Rbuf(), rbuf_->Rlen())
        }
    }
}

void Connector::proc_out()
{
    int res;
    while (true) {
        if (wbuf_->Rlen() <= 0) {
            reactor_->ModIn(this, fd_);
            out_->Yield();
        } else {
            res = write(fd_, wbuf_->Rbuf(), wbuf_->Rlen());
            if (res < 0) {
                if (EAGAIN != errno && EINTR != errno) {
                   // TODO: fail
                    delete this;
                    return;
                }
            } else if (res == 0) {
                // TODO: disconnect
                delete this;
                return;
            } else {
                wbuf_->Read(res);
                if (wbuf_->Rlen() == 0) {
                    reactor_->ModIn(this, fd_);
                }
            }
        }
    }
}

