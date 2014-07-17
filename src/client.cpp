#include <assert.h>

#include "proto/gnet.pb.h"
#include "acceptor.h"
#include "connector.h"
#include "sock.h"
#include "log.h"
#include "gnet.h"
#include "client.h"

using namespace gnet;

Client::Client(GNet* gnet, const std::string& name, const proto::TCP& parent, bool is_gw)
      : Actor(gnet, name)
      , is_gw_(is_gw)
      , parent_con_(NULL)
      , gw_acc_(NULL)
{
    int fd = SOCK::tcp();
    assert(fd > 0);
    int ret = SOCK::connect(fd, parent.host(), parent.port());
    if (ret < 0) {
        gerror(gnet_, "connect parent[%s:%d]: %d", parent.host().c_str(), parent.port(), ret);
        assert(0);
    }

    // add connector to epoll
    parent_con_ = new Connector(gnet_, this, fd);
    parent_con_->Start();

    // add acceptor to epoll
    if (is_gw_) {

    }
}

Client::~Client()
{
}

