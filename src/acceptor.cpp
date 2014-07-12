#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <assert.h>
#include <functional>

#include "coroutine.h"
#include "reactor.h"
#include "connector.h"
#include "acceptor.h"

using namespace gnet;

#define ACCEPTOR_STACK (32 << 10)

Acceptor::Acceptor(Reactor* reactor, const std::string& host, int16_t port)
        : Handle(reactor)
{
    int ret;
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd_ > 0);

    int optval = 1;
    socklen_t optlen = sizeof(optval);
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)(&optval), optlen);

    sockaddr_in addr;
    parse_address(&addr, host, port);

    ret = bind(fd_, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret == 0);

    ret = listen(fd_, 1024);
    assert(ret == 0);
}

Acceptor::~Acceptor()
{
}

void Acceptor::parse_address(sockaddr_in* addr, const std::string& host, int16_t port)
{
    std::string ipstr = host;
    struct hostent* ent = gethostbyname(host.c_str());
    if (ent) {
        char buf[16] = {0};
        sprintf(buf, "%u.%u.%u.%u",
            (unsigned char)ent->h_addr_list[0][0],
            (unsigned char)ent->h_addr_list[0][1],
            (unsigned char)ent->h_addr_list[0][2],
            (unsigned char)ent->h_addr_list[0][3]);
        ipstr = std::string(buf);
    }

    in_addr ip;
    int ret = inet_aton(ipstr.c_str(), &ip);
    assert(ret == 0);

    addr->sin_family = AF_INET;
    addr->sin_addr = ip;
    addr->sin_port = htons(port);
}

void Acceptor::proc_in()
{
    while (true) {
        struct sockaddr addr;
        socklen_t len = sizeof(addr);
        int fd = accept(fd_, &addr, &len);
        if (fd > 0) {
            Connector* con = new Connector(reactor_, fd);
            con->Start();
        }
        in_->Yield();
    }
}

void Acceptor::proc_out()
{
}

