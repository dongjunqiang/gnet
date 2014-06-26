#ifndef GNET_GATEIO_H_
#define GNET_GATEIO_H_

#include <map>
#include <string>

#include <gbase/net/reactor.h>
#include <gbase/net/connector.h>

#include "proto/gnet.pb.h"

class GateIO
{
public:
    // node name <--> connection
    typedef std::map<std::string, connector_t*> CON_MAP_T;

    // socket <--> name 
    typedef std::map<sock_t, std::string> SOCK_MAP_T;

    typedef std::function<void (const std::string&, const char*, int)> RECV_FUNC_T;
    typedef std::function<void (const std::string&)> DISCON_FUNC_T;

    GateIO();
    virtual ~GateIO();

    int BuildLink(GNET::NODE* node);

    int Poll(int ms = 10);

private:
    static int on_read(sock_t, void*, const char* buffer, int len);
    static void on_close(sock_t, void*);

private:
    reactor_t* reactor_;
    CON_MAP_T con_map_;
    SOCK_MAP_T sock_map_;
};

#endif

