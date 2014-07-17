#include <netinet/in.h>
#include <functional>

#include "proto/gnet.pb.h"
#include "coroutine.h"
#include "reactor.h"
#include "connector.h"
#include "sock.h"
#include "gnet.h"
#include "actor.h"
#include "acceptor.h"

using namespace gnet;

#define ACCEPTOR_STACK (32 << 10)

Acceptor::Acceptor(GNet* gnet, Actor* actor, const proto::TCP& tcp)
        : Handle(gnet)
        , actor_(actor)
{
    fd_ = SOCK::tcp();
    assert(fd_ > 0);

    SOCK::set_reuseaddr(fd_);
    SOCK::set_nonblock(fd_);

    struct sockaddr_in addr;
    int ret;
    ret = SOCK::addr_aton(tcp.host(), tcp.port(), &addr);
    assert(ret == 0);

    ret = SOCK::listen(fd_, (struct sockaddr*)&addr);
    assert(ret == 0);
}

Acceptor::~Acceptor()
{
}

void Acceptor::proc_in()
{
    while (true) {
        struct sockaddr addr;
        socklen_t len = sizeof(addr);
        int fd = accept(fd_, &addr, &len);
        if (fd > 0) {
            OnAccept(fd);
        }
        in_->Yield();
    }
}

void Acceptor::proc_out()
{
}

void Acceptor::OnAccept(int fd)
{
    Connector* con = new Connector(gnet_, actor_, fd);
    gdebug(gnet_, "connector %d start.", fd);
    con->Start();
}

