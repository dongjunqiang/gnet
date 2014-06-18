#ifndef GNET_SERVICE_H_
#define GNET_SERVICE_H_

#include <stdint.h>
#include <string>

#include <gbase/logic/bus.h>
#include <gbase/base/timer.h>

#include "service/sid.h"
#include "service/conf.h"

class Service
{
public:
    Service(const std::string& name,
            const std::string& file = "../conf/gnet.conf");
    virtual ~Service();

    enum {
       S_INIT = 0,
       S_TRAINING,
       S_STANDBY,
    };

    int Reload(const std::string& file);

    typedef void (*MsgHandleFunc)(sid_t, const char*, size_t);
    int Recv(MsgHandleFunc func);

    int Send(sid_t sid, const char* data, size_t len);

    sid_t sid() const { return sid_; }

private:
    int status_;
    std::string name_;
    std::string cfgfile_;
    sid_t sid_;

    Conf* conf_;
    struct bus_t* bus_;
    struct timerheap_t* timer_;
};

#endif

