#include <utility>
#include <google/protobuf/stubs/common.h>

#include "proto/gnet.pb.h"
#include "reactor.h"
#include "log.h"
#include "sock.h"
#include "coroutine.h"
#include "handle.h"
#include "router.h"
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
            node_->recv_pkg(this, pkg);
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
    , router_(NULL)
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

    router_ = new Router;
}

Node::~Node()
{
    if (router_) delete router_;
    if (master_con_) delete master_con_;
    if (gw_con_) delete gw_con_;
    if (gw_acc_) delete gw_acc_;
    if (reactor_) delete reactor_;
    if (main_) delete main_;
}

void Node::Resume()
{
    main_->Resume();
}

void Node::proc_gw_rsp(const proto::GWRsp& rsp)
{
    // gw, register to master
    if (!rsp.has_addr()) {
        gw_ = true;

        proto::TCP addr;
        addr.set_host("localhost"); // TODO:
        addr.set_port(DEFAULT_GW_PORT);
        NodeAcceptor* accept = new NodeAcceptor(this, addr);
        accept->Start();
        debug("start as gw[%s:%d]", addr.host().c_str(), addr.port());

        proto::PKG pkg;
        pkg.mutable_head()->set_cmd(proto::CMD_G2M_REQ);
        pkg.mutable_head()->set_timestamp(time(NULL));
        proto::G2MReq* req = pkg.mutable_g2m_req();
        *req->mutable_addr() = addr;
        req->set_name(name_);
        int ret = send_pkg(master_con_, pkg);
        assert(ret == 0);
    }

    // node, register to gw
    else {
        int fd = SOCK::tcp();
        assert(fd > 0);
        int ret = SOCK::connect(fd, rsp.addr().host(), rsp.addr().port());
        if (ret < 0) {
            error("connect to gw[%s:%d] get %d", rsp.addr().host().c_str(),
                rsp.addr().port(), ret);
            assert(0);
        }
        gw_con_ = new NodeConnector(this, fd);
        assert(gw_con_);
        gw_con_->Start();

        proto::PKG pkg;
        pkg.mutable_head()->set_cmd(proto::CMD_N2G_REQ);
        pkg.mutable_head()->set_timestamp(time(NULL));
        proto::N2GReq* req = pkg.mutable_n2g_req();
        req->set_name(name_);
        ret = send_pkg(gw_con_, pkg);
        assert(ret == 0);
    }

    status_ = S_REG;
}

void Node::proc_g2n_rsp(const proto::G2NRsp& rsp)
{
    // TODO
}

void Node::proc_m2g_rsp(const proto::M2GRsp& rsp)
{
    // TODO
}

void Node::proc_n2g_req(const proto::N2GReq& req)
{
    // TODO:
}

void Node::proc_m2g_mod(const proto::M2GMod& mod)
{
    // TODO:
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

void Node::recv_pkg(NodeConnector* con, proto::PKG* pkg)
{
    assert(pkg);
    recv_ = pkg;
    recv_con_ = con;
    debug("\n%s", pkg->DebugString().c_str());

    main_->Resume();
}

void Node::main()
{
    assert(status_ == S_INIT);

    // request gw
    proto::PKG pkg;
    pkg.mutable_head()->set_cmd(proto::CMD_GW_REQ);
    pkg.mutable_head()->set_timestamp(time(NULL));
    int ret = send_pkg(master_con_, pkg);
    assert(ret == 0);
    reactor_->Resume();

    // start work
    while (true) {
        switch (recv_->head().cmd()) {
        case proto::CMD_GW_RSP:
            if (status_ != S_INIT || recv_con_ != master_con_) {
                error("get invalid GW_RSP, ignore");
            } else {
                proc_gw_rsp(recv_->gw_rsp());
            }
            break;

        case proto::CMD_G2N_RSP:
            if (gw_ || status_ != S_REG || recv_con_ != gw_con_) {
                error("get invalid G2N_RSP, ignore"); 
            } else {
                proc_g2n_rsp(recv_->g2n_rsp());
            }
            break;

        case proto::CMD_M2G_RSP:
            if (!gw_ || status_ != S_REG || recv_con_ != master_con_) {
                error("get invalid M2G_RSP, ignore");
            } else {
                proc_m2g_rsp(recv_->m2g_rsp());
            }
            break;

        case proto::CMD_N2G_REQ:
            if (!gw_ || status_ != S_OK) {
                error("get invalid N2G_REQ, ignore");
            } else {
                proc_n2g_req(recv_->n2g_req());
            }
            break;

        case proto::CMD_M2G_MOD:
            if (!gw_ || status_ != S_OK || recv_con_ != master_con_) {
                error("get invalid M2G_MOD, ignore");
            } else {
                proc_m2g_mod(recv_->m2g_mod());
            }
            break;

        default:
            error("unrecognize cmd %d", recv_->head().cmd());
            break;
        }
        main_->Yield();
    }
}

