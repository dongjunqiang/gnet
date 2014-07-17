#ifndef GNET_CLIENT_H_
#define GNET_CLIENT_H_

#include <map>
#include <string>
#include "actor.h"

namespace gnet {
namespace proto { class PKG; }

class Connector;
class Acceptor;
class Client : public Actor
{
public:
    Client(GNet* gnet,
           const std::string& name,
           const proto::TCP& parent_addr,
           bool is_gw = false);

    virtual ~Client();

private:
    bool is_gw_;
    Connector* parent_con_;

    // gw acceptor
    Acceptor* gw_acc_;
    // gw link with other gws
    std::map<std::string, Connector*> gw_buddy_;
    // gw link with its children
    std::map<std::string, Connector*> gw_children_;
};

}

#endif
