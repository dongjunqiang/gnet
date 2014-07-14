#include <assert.h>
#include <string.h>
#include <errno.h>
#include <functional>

#include "coroutine.h"
#include "handle.h"
#include "reactor.h"

using namespace gnet;

Reactor::Reactor()
{
    fd_ = epoll_create(EPOLL_SIZE);
    assert(fd_ > 0);
    memset(events_, 0, sizeof(events_));

    main_ = new Coroutine(std::bind(&Reactor::main, this));
    assert(main_);
}

Reactor::~Reactor()
{
    close(fd_);
    delete main_;
}

void Reactor::Start()
{
    main_->Resume();
}

int Reactor::Add(Handle* handle, int fd, int events)
{
    struct epoll_event event;
    event.events = events;
    event.data.ptr = static_cast<void*>(handle);
    return epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event);
}

int Reactor::Mod(Handle* handle, int fd, int events)
{
    struct epoll_event event;
    event.events = events;
    event.data.ptr = static_cast<void*>(handle);
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event);
}

int Reactor::Del(int fd)
{
    struct epoll_event event;
    event.events = 0;
    return epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &event);
}

void Reactor::main()
{
    while (true) {
        int res = epoll_wait(fd_, events_, EPOLL_SIZE, 10);
        if (res < 0) {
            if (EINTR != errno) {
                assert(0);
            }
        } else if (0 == res) {
            main_->Yield();
        } else {
            for (int i = 0; i < res; ++ i) {
                Handle* handle = static_cast<Handle*>(events_[i].data.ptr);
                if ((EPOLLIN & events_[i].events) || (EPOLLHUP & events_[i].events)) {
                    handle->get_in()->Resume();
                }
                if (EPOLLOUT & events_[i].events) {
                    handle->get_out()->Resume();
                }
            }
        }
    }
}


