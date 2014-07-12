#include <assert.h>
#include <functional>

#include "reactor.h"
#include "coroutine.h"
#include "handle.h"

using namespace gnet;

Handle::Handle(Reactor* reactor)
{
    reactor_ = reactor;
    fd_ = -1;
    in_ = new Coroutine(std::bind(&Handle::proc_in, this), STACK_SIZE);
    out_ = new Coroutine(std::bind(&Handle::proc_out, this), STACK_SIZE);
    assert(in_ && out_);
}

Handle::~Handle()
{
    if (fd_ > 0) {
        reactor_->Del(fd_);
        close(fd_);
    }
    delete in_;
    delete out_;
}

void Handle::Start()
{
    assert(fd_ > 0);
    int ret = reactor_->AddIn(this, fd_);
    assert(ret == 0);
}
