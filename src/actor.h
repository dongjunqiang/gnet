#ifndef GNET_ACTOR_H_
#define GNET_ACTOR_H_

#include <string>

#include "connector.h"
#include "acceptor.h"

namespace gnet {
namespace proto {
    class TCP;
    class PKG;
}
class Reactor;
class Coroutine;
class Actor;
class Router;

#define DEFAULT_GW_PORT 8000

class ActorConnector : public Connector
{
public:
    ActorConnector(Actor* node, int fd);
    virtual int OnRead(const char* buffer, int len);

private:
    Actor* actor_;
};

class ActorAcceptor : public Acceptor
{
public:
    ActorAcceptor(Actor* node, const proto::TCP& addr);
    virtual void OnAccept(int fd);

private:
    Actor* actor_;
};

class Actor
{
    friend class ActorConnector;
    friend class ActorAcceptor;

public:
    Actor(const std::string& name);
    virtual ~Actor();

    void Resume();

protected:
    virtual void main();

    void recv_pkg(ActorConnector* con, proto::PKG* pkg);
    int send_pkg(ActorConnector* con, proto::PKG& pkg);

protected:
    std::string name_;
    Coroutine* main_;
    Reactor* reactor_;

    // cache
    proto::PKG* recv_pkg_;
    ActorConnector* recv_con_;

    Router* router_;
};

/*
class Node
{
    enum State {
        S_INIT = 0,
        S_REG = 1,
        S_OK = 2,
    };

public:
    Node(const std::string& name, const std::string& master_host, int16_t port);
    ~Node();

private:
    void main();

    void proc_gw_rsp(const proto::GWRsp& rsp);
    void proc_g2n_rsp(const proto::G2NRsp& rsp);
    void proc_m2g_rsp(const proto::M2GRsp& rsp);
    void proc_n2g_req(const proto::N2GReq& req);
    void proc_m2g_mod(const proto::M2GMod& mod);

private:
    int status_;
    bool gw_;

    NodeConnector* master_con_;

    // for common node
    NodeConnector* gw_con_;

    // for gw node
    NodeAcceptor* gw_acc_;
};
*/

}

#endif
