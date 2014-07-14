#include <functional>
#include <google/protobuf/stubs/common.h>

#include "proto/gnet.pb.h"
#include "reactor.h"
#include "log.h"
#include "sock.h"
#include "coroutine.h"
#include "handle.h"
#include "node.h"

using namespace gnet;

////////////////////////////////////////////////////////

NodeConnector::NodeConnector(Node* node, int fd)
             : Connector(node->reactor_, fd)
             , node_(node)
{
}

int NodeConnector::OnRead(const char* buffer, int len)
{
    int nread = 0;
    while (true) {
        int nhead = sizeof(int);
        if (len < nhead) break;

        int nbody = *(const int*)(buffer + nread);
        if (len < nhead + nbody) break;

        const void* data = buffer + nread + nhead;
        proto::PKG* pkg = new proto::PKG;
        if (!pkg->ParseFromArray(data, nbody)) {
            error("%s", pkg->InitializationErrorString().c_str());
        } else {
            node_->recv_pkg(pkg);
            delete pkg;
        }

        len -= (nhead + nbody);
        nread += (nhead + nbody);
    }
    return nread;
}

///////////////////////////////////////////////////////////////////////////////

NodeAcceptor::NodeAcceptor(Node* node, const proto::TCP& addr)
            : Acceptor(node->reactor_, addr.host(), addr.port())
            , node_(node)
{
}

void NodeAcceptor::OnAccept(int fd)
{
    NodeConnector* con = new NodeConnector(node_, fd);
    debug("node connector %d start.", fd);
    con->Start();
}

///////////////////////////////////////////////////////////////////////////////

Node::Node(const std::string& name, const std::string& master_host, int16_t master_port)
    : status_(S_INIT)
    , gw_(false)
    , name_(name)
    , master_con_(NULL)
    , gw_con_(NULL)
    , gw_acc_(NULL)
    , reactor_(NULL)
    , main_(NULL)
    , recv_(NULL)
{
    reactor_ = new Reactor;
    assert(reactor_);

    int fd = SOCK::tcp();
    assert(fd > 0);
    int ret = SOCK::connect(fd, master_host, master_port);
    if (ret < 0) {
        error("connect to master[%s:%d] get %d", master_host.c_str(), master_port, ret);
        assert(0);
    }
    master_con_= new NodeConnector(this, fd);
    assert(master_con_);
    master_con_->Start();

    main_ = new Coroutine(std::bind(&Node::main, this), STACK_SIZE);
    assert(main_);
}

void Node::Resume()
{
    main_->Resume();
}

int Node::send_syn()
{
    proto::PKG pkg;
    pkg.mutable_head()->set_cmd(proto::CMD_SYN);
    pkg.mutable_head()->set_timestamp(time(NULL));
    pkg.mutable_syn()->set_name(name_);
    return send_pkg(master_con_, pkg);
}

int Node::recv_ack()
{
    assert(recv_ && recv_->head().cmd() == proto::CMD_ACK);
    const proto::PKGAck& ack = recv_->ack();
    if (ack.result() != true) {
        error("syn fail, maybe name[%s] duplicated", name_.c_str());
        return proto::ERR_NODE_ACK_FAIL;
    }

    // as gw
    if (ack.is_gw()) {
        NodeAcceptor* accept = new NodeAcceptor(this, ack.gw_addr());
        accept->Start();
        status_ = S_GW;
    }

    // connect to gw
    else {
        int fd = SOCK::tcp();
        assert(fd > 0);
        int ret = SOCK::connect(fd, ack.gw_addr().host(), ack.gw_addr().port());
        if (ret < 0) {
            error("connect to gw[%s:%d] get %d", ack.gw_addr().host().c_str(),
                ack.gw_addr().port(), ret);
            return proto::ERR_NODE_CONNECT_FAIL;
        }
        gw_con_ = new NodeConnector(this, fd);
        assert(gw_con_);
        gw_con_->Start();

        // TODO: send gw_syn
    }
    return 0;
}

int Node::send_pkg(NodeConnector* con, proto::PKG& pkg)
{
    debug("\n%s", pkg.DebugString().c_str());
    ::google::protobuf::uint8 buf[1024];
    int len = pkg.ByteSize() + sizeof(int);
    assert((int)sizeof(buf) >= len);
    *(int*)buf = pkg.ByteSize();
    pkg.SerializeWithCachedSizesToArray(buf + sizeof(int));
    return con->Send((const char*)buf, len);
}

void Node::recv_pkg(proto::PKG* pkg)
{
    assert(pkg);
    recv_ = pkg;
    debug("\n%s", pkg->DebugString().c_str());

    if (status_ == S_INIT && pkg->head().cmd() == proto::CMD_ACK)
        main_->Resume();
}

void Node::main()
{
    assert(status_ == S_INIT);

    int ret = send_syn();
    assert(ret == 0);
    status_ = S_SYN;

    // recv_ack, swap out
    reactor_->Resume();

    ret = recv_ack();
    assert(ret == 0);
    status_ = S_ACK;

    // swap out
    reactor_->Resume();
}



