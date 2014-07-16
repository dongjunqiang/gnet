#include <utility>

#include "proto/gnet.pb.h"
#include "reactor.h"
#include "log.h"
#include "sock.h"
#include "coroutine.h"
#include "handle.h"
#include "router.h"
#include "dr.h"
#include "actor.h"

using namespace gnet;

////////////////////////////////////////////////////////

ActorConnector::ActorConnector(Actor* actor, int fd)
              : Connector(actor->reactor_, fd)
              , actor_(actor)
{
}

int ActorConnector::OnRead(const char* buffer, int len)
{
    int ntotal = 0;
    while (true) {
        int nread = len;
        proto::PKG* pkg = new proto::PKG;
        if (!DR::ntoh(buffer, nread, *pkg))
            break;

        actor_->recv_pkg(this, pkg);
        actor_->Resume();
        delete pkg;

        len -= nread; 
        ntotal += nread;
    }
    return ntotal;
}

///////////////////////////////////////////////////////////////////////////////

ActorAcceptor::ActorAcceptor(Actor* actor, const proto::TCP& addr)
             : Acceptor(actor->reactor_, addr.host(), addr.port())
             , actor_(actor)
{
}

void ActorAcceptor::OnAccept(int fd)
{
    ActorConnector* con = new ActorConnector(actor_, fd);
    debug("node connector %d start.", fd);
    con->Start();
}

///////////////////////////////////////////////////////////////////////////////

Actor::Actor(const std::string& name)
     : name_(name)
     , main_(NULL)
     , reactor_(NULL)
     , recv_pkg_(NULL)
     , recv_con_(NULL)
     , router_(NULL)
{
    reactor_ = new Reactor;
    assert(reactor_);

    main_ = new Coroutine(std::bind(&Actor::main, this), STACK_SIZE);
    assert(main_);

    router_ = new Router;
    assert(router_);
}

Actor::~Actor()
{
    delete router_;
    delete main_;
    delete reactor_;
}

void Actor::Resume()
{
    main_->Resume();
}

int Actor::send_pkg(ActorConnector* con, proto::PKG& pkg)
{
    debug("send pkg: \n%s", pkg.DebugString().c_str());
    char buf[1024];
    int len = sizeof(buf);
    bool res = DR::hton(pkg, buf, len);
    assert(res);
    return con->Send((const char*)buf, len);
}

void Actor::recv_pkg(ActorConnector* con, proto::PKG* pkg)
{
    assert(pkg);
    recv_pkg_ = pkg;
    recv_con_ = con;
    debug("recv pkg: \n%s", pkg->DebugString().c_str());
}

void Actor::main()
{
    while (true) {
        // TODO: process pkg
        reactor_->Resume();
    }
}

/*
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
}

Node::~Node()
{
    if (master_con_) delete master_con_;
    if (gw_con_) delete gw_con_;
    if (gw_acc_) delete gw_acc_;
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
*/
