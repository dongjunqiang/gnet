#include <assert.h>
#include <sys/time.h>

#include "proto/gnet.pb.h"
#include "src/gate.h"

Gate::Gate()
    : router_(NULL)
    , io_(NULL)
{
}

Gate::~Gate()
{
    if (router_) {
        delete router_;
        router_ = NULL;
    }
    if (io_) {
        delete io_;
        io_ = NULL;
    }
}

int Gate::Init(const std::string& name, const std::string& file)
{
    name_ = name;
    io_ = new GateIO(this);

    router_ = new Router;
    int ret = router_->Init(file);
    if (ret != GNET::SUCCESS)
        return ret;

    const GNET::NODE* self = router_->GetNodeByName(name_);
    assert(self);

    ret = io_->Start(name_, self->address());
    if (ret != GNET::SUCCESS)
        return ret;

    const GNET::NODE* parent = router_->GetParentNode(self);
    if (parent) {
        int ret = GNET::ERROR;
        while (ret != GNET::SUCCESS) {
            usleep(1000);
            ret = io_->AddLink(parent->name(), parent->address());
        }
    }
    return GNET::SUCCESS;
}

int Gate::Poll()
{
    // TODO:
    return GNET::SUCCESS;
}

void Gate::OnData(const std::string& name, const GNET::PKGData& data)
{
    // TODO:
}

void Gate::OnDiscon(const std::string& name)
{
    // TODO:
}

void Gate::OnBuild(const std::string& name)
{
    // TODO:
}

