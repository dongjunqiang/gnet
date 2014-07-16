#include <assert.h>
#include <unistd.h>

#include "proto/gnet.pb.h"
#include "sock.h"
#include "log.h"
#include "dr.h"
#include "factory.h"

using namespace gnet;

Actor* Factory::GenClient(const std::string& name, const std::string& host, int16_t port)
{
    int fd = SOCK::tcp();
    assert(fd > 0);

    // connect to master
    int ret = SOCK::connect(fd, host, port, 10);
    if (ret < 0) {
        error("connect to master[%s:%d] fail: %d", host.c_str(), port, ret);
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
            error("connection to master closed: %d", ret);
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

            // work as normal-client 
            if (pkg.gw().has_addr()) {
                // TODO:
            }
            // work as gw-client
            else {
                // TODO:
            }
            break;
        }
    }

    return NULL;
}

Actor* Factory::GenMaster(const std::string& name, const std::string& host, int16_t port)
{
    // TODO:
    return NULL;
}

