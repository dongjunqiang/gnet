#ifndef GNET_ROUTER_H_
#define GNET_ROUTER_H_

#include <map>
#include <string>

namespace gnet {
namespace proto {
    class NODE;
    class ROUTE;
}

// 路由表
class Router
{
    // name <--> node
    typedef std::map<std::string, const proto::NODE*> NODE_MAP_T;
    // node <--> node's parent
    typedef std::map<const proto::NODE*, const proto::NODE*> PARENT_MAP_T;

public:
    Router() {}
    ~Router() {}

    void Init(const proto::ROUTE& route, const std::string& name);

    const proto::NODE* get_node(const std::string& name) const;
    const proto::NODE* get_parent(const proto::NODE*) const;

    const proto::NODE* get_root() const { return &route_.root(); }

    const proto::ROUTE& get_route() const { return route_; }

private:
    void load(const proto::NODE*);

private:
    proto::ROUTE route_;
    NODE_MAP_T nodes_map_;
    PARENT_MAP_T parents_map_;
};

}

#endif
