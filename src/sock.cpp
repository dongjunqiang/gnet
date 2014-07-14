#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "sock.h"

using namespace gnet;

void SOCK::set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, NULL);
    assert(flags >= 0);

    int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assert(ret == 0);
}

void SOCK::set_reuseaddr(int fd)
{
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, optlen);
    assert(ret == 0);
}

void SOCK::set_nodelay(int fd)
{
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, optlen);
    assert(ret == 0);
}

void SOCK::set_sndbuf(int fd, int size)
{
    int ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size));
    assert(ret == 0);
}

void SOCK::set_rcvbuf(int fd, int size)
{
    int ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size));
    assert(ret == 0);
}

//  ipv4 only
int SOCK::addr_aton(const std::string& ipstr, int16_t port, struct sockaddr_in* addr)
{
    if (!addr) return -1;
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    if(inet_aton(ipstr.c_str(), &addr->sin_addr) < 0)
        return -1;
    addr->sin_port = htons(port);
    return 0;
}

//  ipv4 only
int SOCK::addr_ntoa(const struct sockaddr_in* addr, std::string& addrstr)
{
    if(!addr)
        return -1;
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s:%d",
        inet_ntoa(addr->sin_addr),
        ntohs(addr->sin_port));
    addrstr = buffer;
    return 0;
}

int SOCK::connect(int fd, const std::string& ipstr, int16_t port)
{
    set_nonblock(fd);
    set_sndbuf(fd);
    set_rcvbuf(fd);

    // parse address
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if(addr_aton(ipstr, port, &addr) < 0)
        return -1;

    // try connect
    int ret = ::connect(fd, (const struct sockaddr*)&addr, addr_len);
    if(ret < 0) {
        // connect fail
        if(EINPROGRESS != errno)
            return errno;

        // try select
        fd_set read_events, write_events, exec_events;
        FD_ZERO(&read_events);
        FD_SET(fd, &read_events);
        write_events = exec_events = read_events;
        struct timeval tv = {1, 0};
        // tv.tv_sec = 1;
        // tv.tv_usec = 0;
        ret = select(fd + 1, &read_events, &write_events, &exec_events, &tv);
        if(ret <= 0)
            return ret;
        if(!FD_ISSET(fd, &read_events) && !FD_ISSET(fd, &write_events))
            return -1;

        // make sure socket no error
        int err;
        socklen_t len = sizeof(err);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
        if(0 != err)
            return err;
    }
    return 0;
}

int SOCK::listen(int fd, struct sockaddr* addr)
{
    set_reuseaddr(fd);
    set_sndbuf(fd);
    set_rcvbuf(fd);

    int ret = ::bind(fd, addr, sizeof(struct sockaddr));
    return ret < 0 ? ret : (::listen(fd, 1024));
}

int SOCK::accept(int fd, struct sockaddr* addr)
{
    socklen_t len = sizeof(struct sockaddr);
    return ::accept(fd, addr, &len);
}

