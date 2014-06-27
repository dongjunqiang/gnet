#include <assert.h>
#include <gbase/net/sock.h>

#include "src/address.h"
#include "src/gateio.h"

GateIO::GateIO(GateBase* gate)
      : reactor_(NULL)
      , name_("")
      , acc_(NULL)
      , gate_(gate)
{
    SOCK_INIT();

    reactor_ = reactor_create();
    assert(reactor_);
}

GateIO::~GateIO()
{
    RAW_CON_MAP::iterator it = raw_cons_.begin();
    while (it != raw_cons_.end()) {
        RemoveLink(it->first);
        it = raw_cons_.begin();
    }

    Stop();

    if (reactor_) {
        reactor_release(reactor_);
        reactor_ = NULL;
    }

    SOCK_RELEASE();
}

connector_t* GateIO::add_connector(sock_t sock)
{
    connector_t* con = connector_create(reactor_);
    assert(con);
    connector_set_read_func(con, GateIO::on_read, this);
    connector_set_close_func(con, GateIO::on_discon, this);
    connector_set_fd(con, sock);

    int ret = connector_start(con);
    assert(ret == 0);

    raw_cons_.insert(std::make_pair(sock, con));
    return con;
}

int GateIO::AddLink(const std::string& name, const std::string& address)
{
    connector_t* con = get_connector(name);
    if (con)
        return GNET::ERR_GATE_LINK_EXIST;

    GNET::Address addr;
    int ret = ParseAddress(address, addr);
    assert(ret == GNET::SUCCESS);

    sock_t sock = sock_tcp();
    sock_set_nonblock(sock);

    ret = sock_connect(sock, addr.tcp().host().c_str(), addr.tcp().port());
    if (ret < 0)
        return GNET::ERR_GATE_CANT_CONNECT;

    add_connector(sock);
    names_.insert(std::make_pair(sock, name));

    return GNET::SUCCESS;
}

int GateIO::RemoveLink(const std::string& name)
{
    connector_t* con = get_connector(name);
    if (!con)
        return GNET::ERR_GATE_LINK_NOT_FOUND;
    // remove index
    sock_t sock = connector_fd(con);
    raw_cons_.erase(sock);
    names_.erase(sock);
    name_cons_.erase(name);
    // release
    connector_stop(con);
    connector_release(con);
    return GNET::SUCCESS;
}

int GateIO::RemoveLink(sock_t sock)
{
    connector_t* con = get_connector(sock);
    if (!con)
        return GNET::ERR_GATE_LINK_NOT_FOUND;
    // remove index
    const std::string* name = get_name(sock);
    if (name) {
        name_cons_.erase(*name);
        names_.erase(sock);
    }
    raw_cons_.erase(sock);
    // release
    connector_stop(con);
    connector_release(con);
    return GNET::SUCCESS;
}

int GateIO::Start(const std::string& name, const std::string& address)
{
    if (acc_)
        return GNET::ERR_GATE_STARTED;

    GNET::Address addr;
    int ret = ParseAddress(address, addr);
    assert(ret == GNET::SUCCESS);

    struct sockaddr_in saddr;
    ret = sock_addr_aton(addr.tcp().host().c_str(), addr.tcp().port(), &saddr);
    assert(ret == 0);

    acc_ = acceptor_create(reactor_);
    assert(acc_);
    acceptor_set_read_func(acc_, GateIO::on_connect, this);
    ret = acceptor_start(acc_, (struct sockaddr*)(&saddr));
    return ret < 0 ? GNET::ERR_GATE_START_FAIL : GNET::SUCCESS;
}

int GateIO::Stop()
{
    if (acc_) {
        acceptor_stop(acc_);
        acceptor_release(acc_);
        acc_ = NULL;
    }
    return GNET::SUCCESS;
}

int GateIO::SendPkg(const std::string& name, const GNET::PKG& pkg)
{
    connector_t* con = get_connector(name);
    if (!con)
        return GNET::ERR_GATE_LINK_NOT_FOUND;

    static ::google::protobuf::uint8 buffer[64 * 1024];
    int size = (int)sizeof(buffer);
    int head_size = (int)sizeof(IOPkg::IOHead);
    int body_size = pkg.ByteSize();

    if (head_size + body_size > size)
        return GNET::ERR_GATE_PKG_TOO_BIG;

    IOPkg::IOHead head;
    head.size = size;
    memcpy(buffer, &head, head_size);
    pkg.SerializeWithCachedSizesToArray(buffer + head_size);

    int ret = connector_send(con, (char*)buffer, head_size + body_size);
    return ret ? GNET::ERR_GATE_SEND_FAIL : GNET::SUCCESS;
}

int GateIO::Poll(int ms)
{
    int ret = reactor_dispatch(reactor_, 10);
    return ret == 0 ? GNET::BUSY : GNET::SUCCESS;
}

int GateIO::on_read(sock_t sock, void* arg, const char* buffer, int len)
{
    GateIO* io = static_cast<GateIO*>(arg);
    int shift = 0;
    if (io) {
        static IOPkg iopkg;
        int head_size = (int)sizeof(iopkg.head);
        while (len > head_size) {

            // head
            memcpy(&iopkg.head, buffer, head_size);
            iopkg.pkg.Clear();
            len -= head_size;

            // body
            if (len > iopkg.head.size) {
                shift += head_size;
                if (iopkg.pkg.ParseFromArray(buffer + shift, iopkg.head.size)) {
                    io->on_read_impl(sock, iopkg.pkg);
                }
                len -= iopkg.head.size;
                shift += iopkg.head.size;
            }
        }
    }
    return shift;
}

void GateIO::on_discon(sock_t sock, void* arg)
{
    GateIO* io = static_cast<GateIO*>(arg);
    if (io) {
        io->on_discon_impl(sock);
    }
}

int GateIO::on_connect(sock_t sock, void* arg)
{
    GateIO* io = static_cast<GateIO*>(arg);
    if (io) {
        io->on_connect_impl(sock);
    }
    return 0;
}

void GateIO::on_read_impl(sock_t sock, GNET::PKG& pkg)
{
    connector_t* con = get_connector(sock);
    if (!con) return;

    switch ((int)pkg.cmd()) {

        case GNET::CMD_DATA: {
            const std::string* name = get_name(sock);
            assert(name);
            gate_->OnData(*name, pkg.data());
            break;
        }

        case GNET::CMD_SYN: {
            const std::string* found = get_name(sock);
            assert(found == NULL);

            const std::string& name = pkg.syn().name();
            name_cons_.insert(std::make_pair(name, con));
            names_.insert(std::make_pair(sock, name));
            gate_->OnBuild(name);
            break;
        }

        case GNET::CMD_ACK:
            // TODO:
        case GNET::CMD_ROUTE:
            // TODO:
            break;
    }
}

void GateIO::on_discon_impl(sock_t sock)
{
    connector_t* con = get_connector(sock);
    if (!con) return;

    const std::string* name = get_name(sock);
    if (name) {
        gate_->OnDiscon(*name);
    }
}

void GateIO::on_connect_impl(sock_t sock)
{
    connector_t* con = get_connector(sock);
    assert(!con);

    add_connector(sock);
}

connector_t* GateIO::get_connector(const std::string& name) const
{
    NAME_CON_MAP::const_iterator it = name_cons_.find(name);
    return it == name_cons_.end() ? NULL : it->second;
}

connector_t* GateIO::get_connector(sock_t sock) const
{
    RAW_CON_MAP::const_iterator it = raw_cons_.find(sock);
    return it == raw_cons_.end() ? NULL : it->second;
}

const std::string* GateIO::get_name(sock_t sock) const
{
    NAME_MAP::const_iterator it = names_.find(sock);
    return it == names_.end() ? NULL : &it->second;
}

