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
            reactor_->Resume();
        } else {
            res = read(fd_, rbuf_->wbuf(), rbuf_->wlen());
            if (res < 0) {
                if (EAGAIN == errno || EINTR == errno) {
                    reactor_->Resume();
                } else {
                    error("connector %d read get %d", fd_, errno);
                    OnDisconnect();
                    delete this;
                    return;
                }
            } else if (res == 0) {
                OnDisconnect();
                delete this;
                return;
            } else {
                rbuf_->Write(res);
                int nread = OnRead(rbuf_->rbuf(), rbuf_->rlen());
                assert(nread >= 0 && nread <= rbuf_->rlen());
                rbuf_->Read(nread);
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
            reactor_->Resume();
        } else {
            res = write(fd_, wbuf_->rbuf(), wbuf_->rlen());
            if (res < 0) {
                if (EAGAIN != errno && EINTR != errno) {
                    error("connector %d write get %d", fd_, errno);
                    OnDisconnect();
                    delete this;
                    return;
                }
            } else if (res == 0) {
                OnDisconnect();
                delete this;
                return;
            } else {
                wbuf_->Read(res);
                if (wbuf_->rlen() == 0) {
                    reactor_->ModIn(this, fd_);
                    reactor_->Resume();
                }
            }
        }
    }
}

int Connector::OnRead(const char* buffer, int len)
{
    debug("connector[%d] recv %d bytes", fd_, len);
    return len;
}

void Connector::OnDisconnect()
{
    debug("connector[%d] disconnect", fd_);
}

