#include <functional>

#include "proto/gnet.pb.h"
#include "coroutine.h"
#include "reactor.h"
#include "buffer.h"
#include "log.h"
#include "sock.h"
#include "dr.h"
#include "actor.h"
#include "gnet.h"
#include "connector.h"

using namespace gnet;

#define CONNECTOR_STACK (64 << 10)

Connector::Connector(GNet* gnet, Actor* actor, int fd)
         : Handle(gnet)
         , rbuf_(NULL)
         , wbuf_(NULL)
         , actor_(actor)
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
        gerror(gnet_, "connector %d write buffer full", fd_);
        return -1;
    }
    wbuf_->Write(len, buffer);
    gnet_->reactor_->ModInOut(this, fd_);
    return 0;
}

void Connector::proc_in()
{
    int res = -1;
    while (true) {
        if (rbuf_->wlen() <= 0) {
            gerror(gnet_, "connector %d read buffer full", fd_);
            in_->Yield();
            continue;
        }

        res = read(fd_, rbuf_->wbuf(), rbuf_->wlen());
        if (res < 0) {
            if (EAGAIN == errno || EINTR == errno) {
                in_->Yield();
                continue;
            }
            gerror(gnet_, "connector %d read get %d", fd_, errno);
            OnDisconnect();
            delete this;
            return;
        }

        if (res == 0) {
            OnDisconnect();
            delete this;
            return;
        }

        rbuf_->Write(res);
        int nread = OnRead(rbuf_->rbuf(), rbuf_->rlen());
        assert(nread >= 0 && nread <= rbuf_->rlen());
        rbuf_->Read(nread);
    }
}

void Connector::proc_out()
{
    int res;
    while (true) {

        if (wbuf_->rlen() <= 0) {
            gnet_->reactor_->ModIn(this, fd_);
            out_->Yield();
            continue;
        }

        res = write(fd_, wbuf_->rbuf(), wbuf_->rlen());
        if (res < 0) {
            if (EAGAIN != errno && EINTR != errno) {
                gerror(gnet_, "connector %d write get %d", fd_, errno);
                OnDisconnect();
                delete this;
                return;
            } else {
                out_->Yield();
                continue;
            }
        }

        if (res == 0) {
            OnDisconnect();
            delete this;
            return;
        }

        // write success
        wbuf_->Read(res);
    }
}

int Connector::OnRead(const char* buffer, int len)
{
    int ntotal = 0;
    while (true) {
        int nread = len;
        proto::PKG* pkg = new proto::PKG;
        if (!DR::ntoh(buffer, nread, *pkg))
            break;

        actor_->recv_pkg(this, pkg);
        delete pkg;

        len -= nread;
        ntotal += nread;
    }
    return ntotal;
}

void Connector::OnDisconnect()
{
    gdebug(gnet_, "connector[%d] disconnect", fd_);
}

