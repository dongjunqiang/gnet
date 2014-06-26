#ifndef GNET_ROUTER_H_
#define GNET_ROUTER_H_

#include <map>
#include <string>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "proto/gnet.pb.h"

// 路由表
class Router
{
    // name <--> node
    typedef std::map<std::string, const GNET::NODE*> NODE_MAP_T;
    // node <--> node's parent
    typedef std::map<const GNET::NODE*, const GNET::NODE*> PARENT_MAP_T;

public:
    Router() {}
    ~Router() {}

    int Init(const std::string& file);
    int Reload(const std::string& file);

    const GNET::NODE* GetNodeByName(const std::string& name) const;
    const GNET::NODE* GetParentNode(const GNET::NODE*) const;
    const GNET::NODE* GetRootNode() const { return &root_; }

private:
    void do_mapping(const GNET::NODE*);

private:
    GNET::NODE root_;
    NODE_MAP_T nodes_map_;
    PARENT_MAP_T parents_map_;
};

#endif
