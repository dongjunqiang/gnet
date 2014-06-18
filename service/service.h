#ifndef GNET_SERVICE_H_
#define GNET_SERVICE_H_

#include <stdint.h>
#include <string>

#include <gbase/logic/bus.h>
#include <gbase/base/timer.h>

#include "conf/conf.pb.h"
#include "service/sid.h"

class Service
{
public:
    Service(const std::string& name,
            const std::string& cfgfile = "../conf/gnet.conf");
    virtual ~Service();

    int Reload();

    int Poll(int ms);
    int Send();
    int Recv();

    sid_t sid() const { return sid_; }
    void set_sid(sid_t id) { sid_ = id; }

    const GNET::CONF_SERVICE& cfg_service() const { return cfg_service_; }
    const GNET::CONF_GLOBAL& cfg_global() const { return cfg_global_; }

private:
    std::string name_;

    std::string cfgfile_;
    GNET::CONF_SERVICE cfg_service_;
    GNET::CONF_GLOBAL cfg_global_;
    sid_t sid_;

    struct bus_t* bus_;
    struct timerheap_t* timer_;
};

#endif
