#include <functional>

#include "coroutine.h"
#include "reactor.h"
#include "buffer.h"
#include "log.h"
#include "sock.h"
#include "connector.h"

using namespace gnet;

#define CONNECTOR_STACK (64 << 10)

Connector::Connector(Reactor* reactor, int fd)
         : Handle(reactor)
         , rbuf_(NULL)
         , wbuf_(NULL)
{
    SOCK::set_nonblock(fd);
    SOCK::set_nodelay(fd);

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
    if (wbuf_->wlen() < len) {
        error("connector %d write buffer full", fd_);
        return -1;
    }
    wbuf_->Write(len, buffer);
    reactor_->ModInOut(this, fd_);
    return 0;
}

void Connector::proc_in()
{
    int res = -1;
    while (true) {
        if (rbuf_->wlen() <= 0) {
            error("connector %d read buffer full", fd_);
            in_->Yield();
        } else {
            res = read(fd_, rbuf_->wbuf(), rbuf_->wlen());
            if (res < 0) {
                if (EAGAIN == errno || EINTR == errno) {
                    in_->Yield();
                } else {
                    error("connector %d read get %d", fd_, errno);
                    delete this;
                    return;
                }
            } else if (res == 0) {
                debug("connector %d disconnect", fd_);
                delete this;
                return;
            } else {
                rbuf_->Write(res);
                // TODO: read (rbuf_->Rbuf(), rbuf_->Rlen())
            }
        }
    }
}

void Connector::proc_out()
{
    int res;
    while (true) {
        if (wbuf_->rlen() <= 0) {
            reactor_->ModIn(this, fd_);
            out_->Yield();
        } else {
            res = write(fd_, wbuf_->rbuf(), wbuf_->rlen());
            if (res < 0) {
                if (EAGAIN != errno && EINTR != errno) {
                    error("connector %d write get %d", fd_, errno);
                    delete this;
                    return;
                }
            } else if (res == 0) {
                debug("connector %d disconnect", fd_);
                delete this;
                return;
            } else {
                wbuf_->Read(res);
                if (wbuf_->rlen() == 0) {
                    reactor_->ModIn(this, fd_);
                    out_->Yield();
                }
            }
        }
    }
}

