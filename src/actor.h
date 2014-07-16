#ifndef GNET_ACTOR_H_
#define GNET_ACTOR_H_

#include <string>

namespace gnet {
namespace proto {
    class TCP;
    class PKG;
}
class Reactor;
class Connector;
class Acceptor;
class Coroutine;
class Actor;
class Router;

class Actor
{
public:
    Actor(const std::string& name);
    virtual ~Actor();

    void Resume();

    Reactor* get_reactor() const { return reactor_; }

    void recv_pkg(Connector* con, proto::PKG* pkg);
    int send_pkg(Connector* con, proto::PKG& pkg);

protected:
    virtual void main();

protected:
    std::string name_;
    Coroutine* main_;
    Reactor* reactor_;

    // cache
    proto::PKG* recv_pkg_;
    Connector* recv_con_;

    Router* router_;
};

}

#endif
