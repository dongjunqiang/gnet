#include <sys/mman.h>

#include "gnet.h"
#include "coroutine.h"

using namespace gnet;

Coroutine::Coroutine(GNet* gnet, FUNC func, size_t stacksz)
         : status_(S_INIT)
         , func_(func)
         , stack_(NULL)
         , stacksz_(stacksz)
         , gnet_(gnet)
{
    stack_ = (SP)valloc(stacksz + RESERVED_SIZE);
    assert(stack_);
    int ret = mprotect(stack_, RESERVED_SIZE, PROT_NONE);
    assert(ret == 0);
    stack_ = stack_ + RESERVED_SIZE;
}

Coroutine::~Coroutine()
{
    stack_ -= RESERVED_SIZE;
    free(stack_);
    gnet_->current_ = NULL;
}

void Coroutine::init_context()
{
    getcontext(&ctx_);
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stacksz_;
    ctx_.uc_link = &gnet_->main_;

    uintptr_t ptr = (uintptr_t)gnet_;
    makecontext(&ctx_, *(void (*)(void))Coroutine::main, (uint32_t)ptr,
        (uint32_t)(ptr >> 32));
}

void Coroutine::Resume()
{
    switch (status_)
    {
        case S_INIT:
            init_context();
        case S_SLEEP:
            gnet_->current_ = this;
            status_ = S_AWAKE;
            swapcontext(&gnet_->main_, &ctx_);
            break;
        default:
            assert(0);
    }
}

void Coroutine::Yield()
{
    status_ = S_SLEEP;
    swapcontext(&ctx_, &gnet_->main_);
    gnet_->current_ = NULL;
}

void Coroutine::main(uint32_t hi32, uint32_t low32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    GNet* gnet = (GNet*)ptr;
    if (gnet && gnet->current_) {
        gnet->current_->func_();
        delete gnet->current_;
    }
}

