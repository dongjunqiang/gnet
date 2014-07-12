#ifndef GNET_ACCEPTOR_H_
#define GNET_ACCEPTOR_H_

#include <stdint.h>
#include <string>

namespace gnet {

class Coroutine;
class Reactor;
class Acceptor 
{
public:
    Acceptor(Reactor* reactor, const std::string& host, int16_t port);
    virtual ~Acceptor();

private:
    void parse_address(sockaddr_in* addr, const std::string& host, int16_t port);
    void main();

private:
    Reactor* reactor_;
    int fd_;
    Coroutine* main_;
};

}

#endif
