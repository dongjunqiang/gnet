#ifndef GNET_ROUTER_H_
#define GNET_ROUTER_H_

#include <map>
#include <string>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "proto/gnet.pb.h"

class Router
{
    typedef std::map<std::string, const GNET::NODE*> MAP_T;
    typedef std::multimap<std::string, std::string> TREE_T;

public:
    Router() {}
    ~Router() {}

    int Init(const std::string& file);
    int Reload(const std::string& file);

    const GNET::NODE* GetNodeByName(const std::string& name) const;
    const GNET::NODE* GetRootNode() const;

private:
    GNET::TABLE table_;
    MAP_T nodes_map_;
    TREE_T nodes_tree_;
};

#endif
