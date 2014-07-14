#include <stdlib.h>
#include <fcntl.h>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "proto/gnet.pb.h"
#include "src/router.h"

using namespace gnet;

int Router::Init()
{
    // TODO
    return proto::SUCCESS;
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

