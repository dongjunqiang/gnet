#ifndef GNET_SOCK_H_
#define GNET_SOCK_H_

#include <sys/socket.h>
#include <sys/types.h>

#include <string>

namespace gnet {

struct SOCK
{
    static int tcp() { return socket(AF_INET, SOCK_STREAM, 0); }
    static int udp() { return socket(AF_INET, SOCK_DGRAM, 0); }

    static int addr_aton(const std::string& ipstr, int16_t port, struct sockaddr_in* addr);
    static int addr_ntoa(const struct sockaddr_in* addr, std::string& addrstr);

    // sec = 0 means block connect
    static int connect(int fd, const std::string& ipstr, int16_t port, int sec = 1);
    static int listen(int fd, struct sockaddr* addr);
    static int accept(int fd, struct sockaddr* addr);

    static void set_nonblock(int fd);
    static void set_reuseaddr(int fd);
    static void set_nodelay(int fd);
    static void set_sndbuf(int fd, int size = (64 << 10));
    static void set_rcvbuf(int fd, int size = (64 << 10));
};

}

#endif
