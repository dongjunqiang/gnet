#include <stdlib.h>
#include <fcntl.h>

#include "src/router.h"

int Router::Init(const std::string& file)
{
    return Reload(file);
}

void Router::do_mapping(const GNET::NODE* node)
{
    if (node) {
        nodes_map_.insert(std::make_pair(node->name(), node));
        for (int i = 0; i < node->children_size(); ++ i) {
            parents_map_.insert(std::make_pair(&node->children(i), node));
            do_mapping(&node->children(i));
        }
    }
}

int Router::Reload(const std::string& file)
{
    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) {
        return GNET::ERR_CFG_NOT_EXIST;
    }

    google::protobuf::io::FileInputStream fi(fd);
    fi.SetCloseOnDelete(true);

    root_.Clear();
    if (!google::protobuf::TextFormat::Parse(&fi, &root_)) {
        return GNET::ERR_CFG_PARSE;
    }

    nodes_map_.clear();
    parents_map_.clear();
    do_mapping(&root_);

    return GNET::SUCCESS;
}

const GNET::NODE* Router::GetNodeByName(const std::string& name) const
{
    NODE_MAP_T::const_iterator it = nodes_map_.find(name);
    return it == nodes_map_.end() ? NULL : it->second;
}

const GNET::NODE* Router::GetParentNode(const GNET::NODE* node) const
{
    PARENT_MAP_T::const_iterator it = parents_map_.find(node);
    return it == parents_map_.end() ? NULL : it->second;
}

