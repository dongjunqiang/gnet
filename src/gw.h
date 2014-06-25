#ifndef GNET_GW_H_
#define GNET_GW_H_

#include <gbase/net/reactor.h>
#include <gbase/net/connector.h>
#include <gbase/base/connbuffer.h>

#include "proto/comm.pb.h"

#include "service/svc.h"

class GW : public BaseSvc
{
public:
    GW(const std::string& name);
    virtual ~GW();

    virtual int InitImpl();
    virtual int ReloadImpl();
    virtual int PollImpl();

private:

    int build_master_link();
    int build_gw_host();

    static int master_read(int fd, const char* buffer, int len);
    static void master_close(int fd);

private:
    struct reactor_t* reactor_;

    GNET::Address master_addr_;
    struct connector_t* master_connector_;
    struct connbuffer_t* master_read_buffer_;
    struct connbuffer_t* master_write_buffer_;
};

#endif

