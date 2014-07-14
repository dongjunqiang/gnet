#include <netinet/in.h>
#include <functional>

#include "coroutine.h"
#include "reactor.h"
#include "connector.h"
#include "acceptor.h"
#include "sock.h"
#include "log.h"

using namespace gnet;

#define ACCEPTOR_STACK (32 << 10)

Acceptor::Acceptor(Reactor* reactor, const std::string& host, int16_t port)
        : Handle(reactor)
{
    fd_ = SOCK::tcp();
    assert(fd_ > 0);

    SOCK::set_reuseaddr(fd_);
    SOCK::set_nonblock(fd_);

    struct sockaddr_in addr;
    int ret;
    ret = SOCK::addr_aton(host, port, &addr);
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
    Connector* con = new Connector(reactor_, fd);
    debug("connector %d start.", fd);
    con->Start();
}

