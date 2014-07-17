#ifndef GNET_ACTOR_H_
#define GNET_ACTOR_H_

#include <string>

namespace gnet {

namespace proto { class PKG; }
class Connector;
class GNet;

class Actor
{
public:
    Actor(GNet* gnet, const std::string& name);
    virtual ~Actor();

    void recv_pkg(Connector* con, proto::PKG* pkg);
    int send_pkg(Connector* con, proto::PKG& pkg);

protected:
    std::string name_;
    GNet* gnet_;

    // cache
    proto::PKG* recv_pkg_;
    Connector* recv_con_;
};

}

#endif
