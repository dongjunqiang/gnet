#include <assert.h>
#include <gbase/net/sock.h>

#include "src/address.h"
#include "src/gateio.h"

GateIO::GateIO()
      : reactor_(NULL)
{
    SOCK_INIT();
}

GateIO::~GateIO()
{
    // clean up index & connector
    for (SOCK_MAP_T::iterator it = sock_map_.begin(); it != sock_map_.end(); ++ it) {
        CON_MAP_T::iterator it2 = con_map_.find(it->second);
        if (it2 != con_map_.end()) {
            connector_t* con = it2->second;
            connector_stop(con);
        }
    }
    for (CON_MAP_T::iterator it = con_map_.begin(); it != con_map_.end(); ++ it) {
        connector_t* con = it->second;
        connector_release(con);
        it->second = NULL;
    }
    con_map_.clear();
    sock_map_.clear();

    // clean up reactor
    if (reactor_) {
        reactor_release(reactor_);
        reactor_ = NULL;
    }

    SOCK_RELEASE();
}

int GateIO::BuildLink(GNET::NODE* node)
{
    if (!reactor_) {
        reactor_ = reactor_create();
        assert(reactor_);
    }

    int ret;
    CON_MAP_T::iterator it = con_map_.find(node->name());
    if (it == con_map_.end()) {

        // new connection
        connector_t* con = connector_create(reactor_);
        assert(con);
        connector_set_read_func(con, GateIO::on_read, this);
        connector_set_close_func(con, GateIO::on_close, this);

        // add index
        con_map_.insert(std::make_pair(node->name(), con));
        it = con_map_.find(node->name());
    }

    connector_t* con = it->second;
    if (connector_fd(con) == INVALID_SOCK) {

        // parse address
        GNET::Address addr;
        ret = ParseAddress(node->address(), addr);
        assert(ret == GNET::SUCCESS);

        // connect
        sock_t sock = sock_tcp();
        sock_set_nonblock(sock);
        ret = sock_connect(sock, addr.tcp().host().c_str(), addr.tcp().port());
        if (ret < 0) {
            return GNET::ERR_GATE_CANT_CONNECT;
        }
        connector_set_fd(con, sock);

        // start connector
        ret = connector_start(con);
        if (ret < 0) {
            return GNET::ERR_GATE_CANT_START_LINK;
        }
 
        // add index
        sock_map_.insert(std::make_pair(sock, node->name()));
   }

    return GNET::SUCCESS;
}

int GateIO::Poll(int ms)
{
    int ret = reactor_dispatch(reactor_, 10);
    if (ret == 0) return GNET::BUSY;
    return GNET::SUCCESS;
}

int GateIO::on_read(sock_t fd, void* arg, const char* buffer, int len)
{
    return -1;
}

void GateIO::on_close(sock_t fd, void* arg)
{
}

