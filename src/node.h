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
        S_SYN = 1,
        S_ACK = 2,
        S_GW = 3,
        S_GW_SYN = 4,
        S_GW_ACK = 5,
    };

    friend class NodeConnector;
    friend class NodeAcceptor;

public:
    Node(const std::string& name, const std::string& master_host, int16_t port);
    ~Node();

    void Resume();

private:
    void main();

    int send_syn();
    int recv_ack();

    int send_pkg(NodeConnector* con, proto::PKG& pkg);
    void recv_pkg(proto::PKG* pkg);

private:
    int status_;
    bool gw_;
    std::string name_;
    NodeConnector* master_con_;
    NodeConnector* gw_con_;
    NodeAcceptor* gw_acc_;
    Reactor* reactor_;
    Coroutine* main_;
    proto::PKG* recv_;
};

}

#endif
