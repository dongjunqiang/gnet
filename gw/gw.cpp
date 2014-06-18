#include <gbase/net/sock.h>

#include "service/address.h"
#include "proto/errcode.pb.h"

#include "gw/gw.h"

GW::GW(const std::string& name)
  : BaseSvc(name, "../conf/gnet.conf")
  , reactor_(NULL)
  , master_connector_(NULL)
  , master_read_buffer_(NULL)
  , master_write_buffer_(NULL)
{
}

GW::~GW()
{
    reactor_release(reactor_);
    connector_release(master_connector_);
    connbuffer_release(master_read_buffer_);
    connbuffer_release(master_write_buffer_);
}

int GW::InitImpl()
{
    const Conf* conf = GetConf();
    if (!conf) {
        return GNET::ERR_CFG_NULL;
    }

    reactor_ = reactor_create();
    if (!reactor_) {
        return GNET::ERR_REACTOR_FAIL;
    }

    master_read_buffer_ = connbuffer_create(64 << 10, malloc, free);
    master_write_buffer_ = connbuffer_create(64 << 10, malloc, free);
    if (!master_read_buffer_ || !master_write_buffer_) {
        return GNET::ERR_CONNBUFFER_FAIL;
    }

    master_connector_ = connector_create(reactor_, master_read,
        master_close, master_read_buffer_, master_write_buffer_);
    if (!master_connector_) {
        return GNET::ERR_CONNECTOR_FAIL;
    }

    int ret = build_master_link();
    if (ret != GNET::SUCCESS) {
        return ret;
    }

    ret = build_gw_host();
    if (ret != GNET::SUCCESS) {
        return ret;
    }

    return GNET::SUCCESS;
}

int GW::PollImpl()
{
    int ret = reactor_dispatch(reactor_, 10);
    if (ret == 0) return GNET::BUSY;

    return GNET::SUCCESS;
}

int GW::ReloadImpl()
{
    // TODO:

    return GNET::SUCCESS;
}

int GW::build_master_link()
{
    sid_t master = GetConf()->GetMasterSid();
    const GNET::CONF_SERVICE* cm = GetConf()->GetServiceBySid(master);
    if (!cm || !cm->has_master()) {
        return GNET::ERR_CFG_NO_MASTER;
    }

    const std::string& cfg_addr = cm->master().cfg_listen_addr();
    master_addr_.Clear();
    int ret = ParseAddress(cfg_addr, master_addr_);
    if (ret != GNET::SUCCESS) {
        return ret;
    }

    // connect to master
    sock_t sock = sock_tcp();
    sock_set_nonblock(sock);
    ret = sock_connect(sock, master_addr_.tcp().host().c_str(),
        master_addr_.tcp().port());
    if (ret < 0) {
        return GNET::ERR_GW_CONNECT_MASTER;
    }
    connector_set_fd(master_connector_, sock);

    // start connector
    ret = connector_start(master_connector_);
    if (ret < 0) {
        return GNET::ERR_GW_MASTER_START;
    }

    return GNET::SUCCESS;
}

int GW::build_gw_host()
{
    // TODO:
    return GNET::SUCCESS;
}

int GW::master_read(int fd, const char* buffer, int len)
{
    // TODO
    return -1;
}

void GW::master_close(int fd)
{
}

