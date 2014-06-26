#include <assert.h>
#include <sys/time.h>

#include "proto/gnet.pb.h"
#include "src/gate.h"

Gate::Gate()
    : router_(NULL)
{
}

Gate::~Gate()
{
    if (router_) {
        delete router_;
        router_ = NULL;
    }
}

int Gate::Init(const std::string& name, const std::string& file)
{
    name_ = name;

    router_ = new Router();
    int ret = router_->Init(file);
    if (ret != GNET::SUCCESS) return ret;

    return reg_to_parent();
}

int Gate::Reload(const std::string& name, const std::string& file)
{
    name_ = name;

    int ret = router_->Reload(file);
    if (ret != GNET::SUCCESS) return ret;

    return reg_to_parent();
}

int Gate::reg_to_parent()
{
    const GNET::NODE* self = router_->GetNodeByName(name_);
    if (!self) return GNET::ERR_GATE_NOT_FOUND;

    const GNET::NODE* parent = router_->GetParentNode(self);
    if (parent) {
        // TODO:
    }

    return GNET::SUCCESS;
}

int Gate::Poll()
{
    return GNET::SUCCESS;
}

