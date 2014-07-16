#include <utility>

#include "proto/gnet.pb.h"
#include "reactor.h"
#include "log.h"
#include "sock.h"
#include "coroutine.h"
#include "handle.h"
#include "router.h"
#include "dr.h"
#include "connector.h"
#include "acceptor.h"
#include "actor.h"

using namespace gnet;

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

int Actor::send_pkg(Connector* con, proto::PKG& pkg)
{
    debug("send pkg: \n%s", pkg.DebugString().c_str());
    char buf[1024];
    int len = sizeof(buf);
    bool res = DR::hton(pkg, buf, len);
    assert(res);
    return con->Send((const char*)buf, len);
}

void Actor::recv_pkg(Connector* con, proto::PKG* pkg)
{
    assert(pkg && con);
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
