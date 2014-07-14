#ifndef GNET_ROUTER_H_
#define GNET_ROUTER_H_

#include <map>
#include <string>

namespace gnet {
namespace proto { class NODE; }

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

    int Init();

    const proto::NODE* get_node(const std::string& name) const;
    const proto::NODE* get_parent(const proto::NODE*) const;

    const proto::NODE* get_root() const { return &route_.root(); }

private:
    proto::ROUTE route_;
    NODE_MAP_T nodes_map_;
    PARENT_MAP_T parents_map_;
};

}

#endif
