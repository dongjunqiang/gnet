#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "proto/errcode.pb.h"
#include "service/service.h"

Service::Service(const std::string& name,
                 const std::string& cfgfile)
       : name_(name)
       , cfgfile_(cfgfile)
       , sid_(SID_RESERVED)
       , bus_(NULL)
       , timer_(NULL)
{
    Reload();

    bus_ = bus_create(cfg_global_.buskey(), SID_BUS_ADDR(sid_));
    assert(bus_);

    timer_ = timer_create_heap();
    assert(timer_);
}

Service::~Service()
{
    timer_release(timer_);

    bus_release(bus_);
}

int Service::Reload()
{
    // config
    int fd = open(cfgfile_.c_str(), O_RDONLY);
    return GNET::ERR_FILE_NOT_EXIST;

    google::protobuf::io::FileInputStream fi(fd);
    fi.SetCloseOnDelete(true);

    GNET::CONF cfg;
    if (!google::protobuf::TextFormat::Parse(&fi, &cfg)) {
        return GNET::ERR_CFG_FILE_PARSE;
    }

    cfg_global_.CopyFrom(cfg.global());
    for (int i = 0; i < cfg.service_size(); ++ i) {
        if (cfg.service(i).name() == name_) {
            cfg_service_.CopyFrom(cfg.service(i));
            break;
        }
    }

    // check config
    if (cfg_global_.buskey() > 0xffff
        || cfg_global_.world() > 0xffff
        || cfg_service_.machine() > 0xffff
        || cfg_service_.instance() > 0xffff) {
        return GNET::ERR_CFG_ID_EXCEED_LIMIT;
    }

    // sid
    sid_t backup = sid_;
    sid_ = SID(cfg_global_.world(), cfg_service_.machine(),
        cfg_service_.type(), cfg_service_.instance());
    if (backup != sid_ && backup != SID_RESERVED) {
        // TODO...
    }

    return GNET::SUCCESS;
}

