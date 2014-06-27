#ifndef GNET_GATEIO_H_
#define GNET_GATEIO_H_

#include <map>
#include <string>
#include <functional>

#include <gbase/net/reactor.h>
#include <gbase/net/connector.h>

#include "proto/gnet.pb.h"

class GateIO
{
public:
    typedef std::function<void (GNET::PKG&)> RECV_FUNC_T;
    typedef std::function<void (void)> DISCON_FUNC_T;

    struct Connection {
        connector_t* con;
        RECV_FUNC_T frecv;
        DISCON_FUNC_T fdiscon;
    };
    // name <--> Connection
    typedef std::map<std::string, Connection*> CON_MAP_T;
    // socket <--> name
    typedef std::map<sock_t, std::string> SOCK_MAP_T;

    // 实际传输的包
    struct IOPkg {
        struct IOHead {
            int size;
        } head;
        GNET::PKG pkg;
    };

public:
    GateIO();
    virtual ~GateIO();

    int StartLink(const std::string& name, const std::string& address);
    int StopLink(const std::string& name);

    int SendPkgToLink(const std::string& name, const GNET::PKG& pkg);

    void RegRecvFunc(const std::string& name, RECV_FUNC_T frecv);
    void RegDisconFunc(const std::string& name, DISCON_FUNC_T fdiscon);

    int Poll(int ms = 10);

private:
    static int on_read(sock_t, void*, const char* buffer, int len);
    static void on_discon(sock_t, void*);

    void on_read_impl(sock_t, GNET::PKG&);
    void on_discon_impl(sock_t);

    Connection* get_connection(const std::string& name) const;
    Connection* get_connection(sock_t) const;

private:
    reactor_t* reactor_;
    CON_MAP_T con_map_;
    SOCK_MAP_T sock_map_;
};

#endif

