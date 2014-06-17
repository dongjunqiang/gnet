#ifndef GNET_SERVICE_H_
#define GNET_SERVICE_H_

#include <stdint.h>
#include <string>

#include <gbase/include/logic/bus.h>
#include <gbase/include/base/timer.h>

#include "conf/conf.pb.h"
#include "service/sid.h"

class Service
{
public:
    Service(const std::string& cfgfile, const std::string& name);
    virtual ~Service();

    int reload();

    int poll(int ms);
    int send();
    int recv();

    sid_t sid() const { return sid; }
    void set_sid(sid_t id) { sid = id; }

    const GNET::CONF& conf() const { return cfg; }

private:
    std::string cfgfile;
    GNET::CONF cfg;
    sid_t sid;

    struct bus_t* bus;
    struct timerheap_t* timer;
};

#endif
