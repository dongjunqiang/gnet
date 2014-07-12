#ifndef GNET_REACTOR_H_
#define GNET_REACTOR_H_

#include <sys/epoll.h>

namespace gnet {

#define EPOLL_SIZE 10240

class Coroutine;

class Reactor
{
public:
    Reactor();
    ~Reactor();

    int AddIn(Coroutine* co, int fd) {
        return Add(co, fd, EPOLLIN | EPOLLERR | EPOLLHUP);
    }
    int AddInOut(Coroutine* co, int fd) {
         return Add(co, fd, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP);
    }

    int Add(Coroutine* co, int fd, int events);
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
