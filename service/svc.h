#ifndef GNET_SVC_H_
#define GNET_SVC_H_

#include <stdint.h>
#include <string>

#include <gbase/logic/bus.h>
#include <gbase/base/timer.h>

#include "service/sid.h"
#include "service/conf.h"

struct BaseSvc
{
public:
    BaseSvc(const std::string& name,
            const std::string& file);
    virtual ~BaseSvc();

    int Init();
    int Reload();
    int Poll();

protected:
    virtual int InitImpl() = 0;
    virtual int ReloadImpl() = 0;
    virtual int PollImpl() = 0;

    const Conf* GetConf() const { return conf_; }
    sid_t GetSid() const { return sid_; }

private:
    std::string name_;
    std::string file_;
    sid_t sid_;
    Conf* conf_;
    struct timerheap_t* timer_;
};

#endif

