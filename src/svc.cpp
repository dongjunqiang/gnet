#include <assert.h>
#include <sys/time.h>

#include "proto/errcode.pb.h"
#include "service/svc.h"

BaseSvc::BaseSvc(const std::string& name,
                 const std::string& file)
       : name_(name)
       , file_(file)
       , sid_(SID_RESERVED)
       , conf_(NULL)
       , timer_(NULL)
{
}

BaseSvc::~BaseSvc()
{
    if (timer_) {
        timer_release(timer_);
        timer_ = NULL;
    }
    if (conf_) {
        delete conf_;
        conf_ = NULL;
    }
}

int BaseSvc::Init()
{
    int ret = GNET::SUCCESS;

    conf_ = new Conf();
    ret = conf_->Init(file_);
    if (ret != GNET::SUCCESS) {
        return ret;
    }

    sid_ = conf_->GetSidByName(name_);
    assert(sid_ != SID_RESERVED);

    timer_ = timer_create_heap();
    assert(timer_);

    return InitImpl();
}

int BaseSvc::Reload()
{
    int ret = GNET::SUCCESS;

    ret = conf_->Reload(file_);
    if (ret != GNET::SUCCESS) {
        return ret;
    }

    // TODO: change sid would be support later
    sid_t backup = sid_;
    sid_ = conf_->GetSidByName(name_);
    assert(sid_ == backup);

    return ReloadImpl();
}

int BaseSvc::Poll()
{
    if (timer_) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        timer_poll(timer_, &tv);
    }

    return PollImpl();
}

