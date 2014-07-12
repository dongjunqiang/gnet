#ifndef GNET_REACTOR_H_
#define GNET_REACTOR_H_

#include <sys/epoll.h>

namespace gnet {

#define EPOLL_SIZE 10240

class Handle;
class Coroutine;

class Reactor
{
public:
    Reactor();
    ~Reactor();

    int AddIn(Handle* handle, int fd) {
        return Add(handle, fd, EPOLLIN | EPOLLERR | EPOLLHUP);
    }
    int ModIn(Handle* handle, int fd) {
        return Mod(handle, fd, EPOLLIN | EPOLLERR | EPOLLHUP);
    }
    int ModInOut(Handle* handle, int fd) {
         return Mod(handle, fd, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP);
    }

    int Add(Handle* handle, int fd, int events);
    int Mod(Handle* handle, int fd, int events);
    int Del(int fd);

    void Start();

private:
    void main();

private:
    int fd_;
    struct epoll_event events_[EPOLL_SIZE];
    Coroutine* main_;
};

}

#endif
