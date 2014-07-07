#include <assert.h>
#include <sys/time.h>

#include "proto/gnet.pb.h"
#include "src/gate.h"

using namespace gnet;

Gate::Gate()
    : router_(NULL)
    // , io_(NULL)
{
}

Gate::~Gate()
{
    if (router_) {
        delete router_;
        router_ = NULL;
    }
    /*
    if (io_) {
        delete io_;
        io_ = NULL;
    }
    */
}

int Gate::Init(const std::string& name, const std::string& file)
{
    /*
    name_ = name;
    io_ = new GateIO(this);

    router_ = new Router;
    int ret = router_->Init(file);
    if (ret != proto::SUCCESS)
        return ret;

    const proto::NODE* self = router_->GetNodeByName(name_);
    assert(self);

    ret = io_->Start(name_, self->address());
    if (ret != proto::SUCCESS)
        return ret;

    const proto::NODE* parent = router_->GetParentNode(self);
    if (parent) {
        int ret = proto::ERROR;
        while (ret != proto::SUCCESS) {
            usleep(1000);
            ret = io_->AddLink(parent->name(), parent->address());
        }
    }
    */
    return proto::SUCCESS;
}

int Gate::Poll()
{
    // TODO:
    return proto::SUCCESS;
}

void Gate::OnData(const std::string& name, const proto::PKGData& data)
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

