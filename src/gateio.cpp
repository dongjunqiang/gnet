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
            Connection* con = it2->second;
            if (con && con->con) {
                connector_stop(con->con);
            }
        }
    }
    for (CON_MAP_T::iterator it = con_map_.begin(); it != con_map_.end(); ++ it) {
        Connection* con = it->second;
        if (con) {
            if (con->con) {
                connector_release(con->con);
            }
            delete con;
        }
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

int GateIO::StartLink(const std::string& name, const std::string& address)
{
    if (!reactor_) {
        reactor_ = reactor_create();
        assert(reactor_);
    }

    int ret;
    CON_MAP_T::iterator it = con_map_.find(name);
    if (it == con_map_.end()) {

        // new connection
        connector_t* con = connector_create(reactor_);
        assert(con);
        connector_set_read_func(con, GateIO::on_read, this);
        connector_set_close_func(con, GateIO::on_discon, this);

        Connection* connection = new Connection;
        connection->con = con;
        connection->frecv = NULL;
        connection->fdiscon = NULL;

        // add index
        con_map_.insert(std::make_pair(name, connection));
        it = con_map_.find(name);
    }

    Connection* con = it->second;
    if (connector_fd(con->con) == INVALID_SOCK) {

        // parse address
        GNET::Address addr;
        ret = ParseAddress(address, addr);
        assert(ret == GNET::SUCCESS);

        // connect
        sock_t sock = sock_tcp();
        sock_set_nonblock(sock);
        ret = sock_connect(sock, addr.tcp().host().c_str(), addr.tcp().port());
        if (ret < 0) {
            return GNET::ERR_GATE_CANT_CONNECT;
        }
        connector_set_fd(con->con, sock);

        // start connector
        ret = connector_start(con->con);
        if (ret < 0) {
            return GNET::ERR_GATE_CANT_START_LINK;
        }

        // add index
        sock_map_.insert(std::make_pair(sock, name));
    }

    return GNET::SUCCESS;
}

int GateIO::StopLink(const std::string& name)
{
    Connection* con = get_connection(name);
    if (!con)
        return GNET::ERR_GATE_LINK_NOT_FOUND;

    if (con->con) {
        sock_t fd = connector_fd(con->con);
        if (fd != INVALID_SOCK) {
            sock_map_.erase(fd);
        }
        connector_stop(con->con);
        connector_release(con->con);
    }
    delete con;
    con_map_.erase(name);

    return GNET::SUCCESS;
}

int GateIO::Poll(int ms)
{
    int ret = reactor_dispatch(reactor_, 10);
    if (ret == 0) return GNET::BUSY;
    return GNET::SUCCESS;
}

void GateIO::RegRecvFunc(const std::string& name, RECV_FUNC_T frecv)
{
    CON_MAP_T::iterator it = con_map_.find(name);
    if (it != con_map_.end()) {
        Connection* con = it->second;
        if (con)
            con->frecv = frecv;
    }
}

void GateIO::RegDisconFunc(const std::string& name, DISCON_FUNC_T fdiscon)
{
    CON_MAP_T::iterator it = con_map_.find(name);
    if (it != con_map_.end()) {
        Connection* con = it->second;
        if (con)
            con->fdiscon = fdiscon;
    }
}

int GateIO::on_read(sock_t fd, void* arg, const char* buffer, int len)
{
    GateIO* io = static_cast<GateIO*>(arg);
    if (io) {
        io->on_read_impl(fd, buffer, len);
    }
    // 自动排干
    return len;
}

void GateIO::on_discon(sock_t fd, void* arg)
{
    GateIO* io = static_cast<GateIO*>(arg);
    if (io) {
        io->on_discon_impl(fd);
    }
}

void GateIO::on_read_impl(sock_t fd, const char* buffer, int len)
{
    Connection* con = get_connection(fd);
    if (con && con->frecv) {
        con->frecv(buffer, len);
    }
}

void GateIO::on_discon_impl(sock_t fd)
{
    Connection* con = get_connection(fd);
    if (con && con->fdiscon) {
        con->fdiscon();
    }
}

GateIO::Connection* GateIO::get_connection(const std::string& name) const
{
    CON_MAP_T::const_iterator it = con_map_.find(name);
    return it == con_map_.end() ? NULL : it->second;
}

GateIO::Connection* GateIO::get_connection(sock_t fd) const
{
    SOCK_MAP_T::const_iterator it = sock_map_.find(fd);
    return it == sock_map_.end() ? NULL : get_connection(it->second);
}

