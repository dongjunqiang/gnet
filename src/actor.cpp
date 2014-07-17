#include "proto/gnet.pb.h"
#include "log.h"
#include "sock.h"
#include "handle.h"
#include "dr.h"
#include "connector.h"
#include "acceptor.h"
#include "gnet.h"
#include "actor.h"

using namespace gnet;

Actor::Actor(GNet* gnet, const std::string& name)
     : name_(name)
     , gnet_(gnet)
     , recv_pkg_(NULL)
     , recv_con_(NULL)
{
}

Actor::~Actor()
{
}

int Actor::send_pkg(Connector* con, proto::PKG& pkg)
{
    gdebug("send pkg: \n%s", pkg.DebugString().c_str());
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
    gdebug("recv pkg: \n%s", pkg->DebugString().c_str());
}


