#include <sys/mman.h>
#include "src/coroutine.h"

using namespace gnet;

Coroutine::Coroutine(FUNC func, size_t stacksz)
         : status_(S_INIT)
         , func_(func)
         , stack_(NULL)
         , stacksz_(stacksz)
{
    stack_ = (SP)valloc(stacksz + RESERVED_SIZE);
    assert(stack_);

    int ret = mprotect(stack_, RESERVED_SIZE, PROT_NONE);
    assert(ret == 0);

    stack_ = stack_ + RESERVED_SIZE;

    static int idx = 0;
    id_ = ++ idx;

    Scheduler::Instance()->AddCoroutine(this);
}

Coroutine::~Coroutine()
{
    Scheduler::Instance()->RemoveCoroutine(this);

    stack_ -= RESERVED_SIZE;
    free(stack_);
}

void Coroutine::init_context()
{
    getcontext(&ctx_);
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stacksz_;

    ucontext_t* main = Scheduler::Instance()->GetMain();
    ctx_.uc_link = main;
    status_ = S_AWAKE;

    makecontext(&ctx_, (void (*)(void))Coroutine::routine, 0);
    swapcontext(main, &ctx_);
}

void Coroutine::Resume()
{
    switch (status_)
    {
        case S_INIT:
            init_context();
            break;
        case S_SLEEP:
            Scheduler::Instance()->SetCurrent(this);
            status_ = S_AWAKE;
            swapcontext(Scheduler::Instance()->GetMain(), &ctx_);
            break;
        default:
            assert(0);
    }
}

void Coroutine::routine()
{
    Scheduler* sched = Scheduler::Instance();
    Coroutine* cr = sched->GetCurrent();
    if (cr) {
        cr->func_();
        delete cr;
    }
}

////////////////////////////////////////////////////////////////

Scheduler::Scheduler()
         : current_(NULL)
{
}

Scheduler::~Scheduler()
{
    for (ITER_T it = units_.begin(); it != units_.end(); ++ it) {
        delete it->second;
        it->second = NULL;
    }
}

