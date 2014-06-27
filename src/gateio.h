#ifndef GNET_GATEIO_H_
#define GNET_GATEIO_H_

#include <map>
#include <string>
#include <functional>

#include <gbase/net/reactor.h>
#include <gbase/net/connector.h>
#include <gbase/net/acceptor.h>

#include "proto/gnet.pb.h"

class GateBase
{
public:
    virtual ~GateBase() {}

    virtual void OnData(const std::string&, const GNET::PKGData&) = 0;
    virtual void OnDiscon(const std::string&) = 0;
    virtual void OnBuild(const std::string&) = 0;
};

class GateIO
{
private:
    // sock <--> connector
    typedef std::map<sock_t, connector_t*> RAW_CON_MAP;
    // name <--> connector
    typedef std::map<std::string, connector_t*> NAME_CON_MAP;
    // sock <--> name
    typedef std::map<sock_t, std::string> NAME_MAP;

    // 实际传输的包
    struct IOPkg {
        struct IOHead {
            int size;
        } head;
        GNET::PKG pkg;
    };

public:
    GateIO(GateBase*);
    virtual ~GateIO();

    int Start(const std::string& name, const std::string& address);
    int Stop();

    int AddLink(const std::string& name, const std::string& address);
    int RemoveLink(const std::string& name);
    int RemoveLink(sock_t);

    int SendPkg(const std::string& name, const GNET::PKG& pkg);

    int Poll(int ms = 10);

private:
    static int on_read(sock_t, void*, const char* buffer, int len);
    static void on_discon(sock_t, void*);
    static int on_connect(sock_t, void*);

    void on_read_impl(sock_t, GNET::PKG&);
    void on_discon_impl(sock_t);
    void on_connect_impl(sock_t);

    connector_t* get_connector(const std::string& name) const;
    connector_t* get_connector(sock_t) const;

    const std::string* get_name(sock_t) const;

    connector_t* add_connector(sock_t sock);

private:
    reactor_t* reactor_;
    std::string name_;
    acceptor_t* acc_;
    GateBase* gate_;

    NAME_CON_MAP name_cons_;
    RAW_CON_MAP raw_cons_;
    NAME_MAP names_;
};

#endif

