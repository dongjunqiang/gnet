#include "service/address.h"
#include "proto/errcode.pb.h"

#include "gw/gw.h"

GW::GW(const std::string& name)
  : BaseSvc(name, "../conf/gnet.conf")
{
}

GW::~GW()
{
}

int GW::InitImpl()
{
    const Conf* conf = GetConf();
    if (!conf) {
        return GNET::ERR_CFG_NULL;
    }

    sid_t master = GetConf()->GetMasterSid();
    const GNET::CONF_SERVICE* cm = GetConf()->GetServiceBySid(master);
    if (!cm || !cm->has_master()) {
        return GNET::ERR_CFG_NO_MASTER;
    }

    const std::string& cfg_addr = cm->master().cfg_listen_addr();
    GNET::Address addr;
    int ret = ParseAddress(cfg_addr, addr);
    if (ret != GNET::SUCCESS) {
        return ret;
    }

    // TODO: connect

    return GNET::SUCCESS;
}

