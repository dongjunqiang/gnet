#ifndef GNET_H_
#define GNET_H_

#include <string>
#include <stdint.h>
#include <ucontext.h>

#include "reactor.h"
#include "router.h"

namespace gnet {

class Client;
class Master;
class Coroutine;
struct GNet
{
    GNet(const std::string& master_host, int16_t master_port)
    {
        master_host_ = master_host;
        master_port_ = master_port;

        reactor_ = new Reactor;
        router_ = new Router;
        current_ = NULL;
    }

    ~GNet()
    {
        delete router_;
        delete reactor_;
    }

    Client* CreateClient(const std::string& name);
    Master* CreateMaster(const std::string& name);

    Reactor* reactor_;
    Router* router_;
    Coroutine* current_;
    ucontext_t main_;

    std::string master_host_;
    int16_t master_port_;
};

}

#endif
