#include <assert.h>

#include "proto/errcode.pb.h"
#include "service/service.h"

Service::Service(const std::string& name,
                 const std::string& file)
       : status_(S_INIT)
       , name_(name)
       , sid_(SID_RESERVED)
       , conf_(NULL)
       , bus_(NULL)
       , timer_(NULL)
{
    conf_ = new Conf();
    assert(conf_);

    Reload(file);

    timer_ = timer_create_heap();
    assert(timer_);
}

Service::~Service()
{
    timer_release(timer_);
    bus_release(bus_);
    delete conf_;
}

int Service::Reload(const std::string& file)
{
    // reload config
    conf_->Reload(file);

    // sid
    sid_t backup = sid_;
    sid_ = conf_->GetSidByName(name_);

    // bus related to sid
    if (sid_ != backup) {
        if (bus_) {
            bus_release(bus_);
        }
        const GNET::CONF_GLOBAL* global = conf_->Global();
        assert(global);
        bus_ = bus_create(global->buskey(), SID_BUS_ADDR(sid_));
        assert(bus_);
    }

    return GNET::SUCCESS;
}

