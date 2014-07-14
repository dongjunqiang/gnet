#ifndef GNET_NODE_H_
#define GNET_NODE_H_

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
class Node;
class Router;

#define DEFAULT_GW_PORT 8000

class NodeConnector : public Connector
{
public:
    NodeConnector(Node* node, int fd);
    virtual int OnRead(const char* buffer, int len);

private:
    Node* node_;
};

class NodeAcceptor : public Acceptor
{
public:
    NodeAcceptor(Node* node, const proto::TCP& addr);
    virtual void OnAccept(int fd);

private:
    Node* node_;
};

class Node
{
    enum State {
        S_INIT = 0,
        S_REG = 1,
        S_OK = 2,
    };

    friend class NodeConnector;
    friend class NodeAcceptor;

public:
    Node(const std::string& name, const std::string& master_host, int16_t port);
    ~Node();

    void Resume();

private:
    void main();

    void proc_gw_rsp(const proto::GWRsp& rsp);
    void proc_g2n_rsp(const proto::G2NRsp& rsp);
    void proc_m2g_rsp(const proto::M2GRsp& rsp);
    void proc_n2g_req(const proto::N2GReq& req);
    void proc_m2g_mod(const proto::M2GMod& mod);

    int send_pkg(NodeConnector* con, proto::PKG& pkg);
    void recv_pkg(NodeConnector* con, proto::PKG* pkg);

private:
    int status_;
    bool gw_;
    std::string name_;
    NodeConnector* master_con_;

    // for common node
    NodeConnector* gw_con_;

    // for gw node
    NodeAcceptor* gw_acc_;
    Reactor* reactor_;
    Coroutine* main_;

    // receive cache
    proto::PKG* recv_;
    NodeConnector* recv_con_;

    Router* router_;
};

}

#endif
