#include <assert.h>
#include <unistd.h>

#include "proto/gnet.pb.h"
#include "sock.h"
#include "log.h"
#include "dr.h"
#include "client.h"
#include "coroutine.h"
#include "gnet.h"

using namespace gnet;

Client* GNet::CreateClient(const std::string& name)
{
    int fd = SOCK::tcp();
    assert(fd > 0);

    // connect to master
    int ret = SOCK::connect(fd, master_host_, master_port_, 10);
    if (ret < 0) {
        gerror("connect to master[%s:%d]: %d", master_host_.c_str(), master_port_, ret);
        return NULL;
    }

    // wait gw info from master
    char buffer[1024];
    int nbytes = 0;
    int nsize = sizeof(buffer);
    while (true) {
        ret = read(fd, buffer + nbytes, nsize);
        if (ret < 0 && (EAGAIN == errno || EINTR == errno))
            usleep(10);
        else if (ret <= 0) {
            gerror("connection to master closed: %d", ret);
            return NULL;
        } else {
            nbytes += ret;
            nsize -= ret;

            // dr parse
            proto::PKG pkg;
            int len = nbytes;
            if (!DR::ntoh(buffer, len, pkg)) {
                continue;
            }
            assert(pkg.head().cmd() == proto::CMD_GW);

            // close socket
            close(fd);

            // work as normal-client
            if (pkg.gw().has_addr()) {
                Client* client = new Client(this, name, pkg.gw().addr());
                return client;
            }
            // work as gw-client
            else {
                proto::TCP master;
                master.set_host(master_host_);
                master.set_port(master_port_);
                Client* client = new Client(this, name, master, true);
                return client;
            }
            break;
        }
    }
    return NULL;
}

Master* GNet::CreateMaster(const std::string& name)
{
    return NULL;
}
