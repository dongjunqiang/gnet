#include <assert.h>

#include "router.h"

using namespace gnet;

void Router::Init(const proto::ROUTE& route, const std::string& name)
{
    route_ = route;
    assert(route_.has_root());
    load(&route_.root());
}

void Router::load(const proto::NODE* node)
{
    nodes_map_.insert(std::make_pair(node->name(), node));
    for (int i = 0; i < node->children_size(); ++ i) {
        parents_map_.insert(std::make_pair(&node->children(i), node));
        load(&node->children(i));
    }
}

const proto::NODE* Router::get_node(const std::string& name) const
{
    NODE_MAP_T::const_iterator it = nodes_map_.find(name);
    return it == nodes_map_.end() ? NULL : it->second;
}

const proto::NODE* Router::get_parent(const proto::NODE* node) const
{
    PARENT_MAP_T::const_iterator it = parents_map_.find(node);
    return it == parents_map_.end() ? NULL : it->second;
}

