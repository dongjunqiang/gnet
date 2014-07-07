#include <sys/mman.h>
#include "src/coroutine.h"

using namespace gnet;

Coroutine::Coroutine(int id,  size_t stacksz, FUNC func)
         : id_(id)
         , status_(S_INIT)
         , func_(func)
         , stack_(NULL)
         , stacksz_(stacksz)
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
}

void Coroutine::start()
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

void Coroutine::resume()
{
    status_ = S_AWAKE;
    swapcontext(Scheduler::Instance()->GetMain(), &ctx_);
}

void Coroutine::yield()
{
    status_ = S_SLEEP;
    swapcontext(&ctx_, Scheduler::Instance()->GetMain());
}

void Coroutine::routine()
{
    Scheduler* sched = Scheduler::Instance();
    Coroutine* cr = sched->GetCurrent();
    if (cr) {
        cr->callback();
        sched->RemoveCoroutine(cr);
        delete cr;
    }
}

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

Coroutine* Scheduler::CreateCoroutine(Coroutine::FUNC func, size_t stacksz)
{
    static int idx = 0;
    Coroutine* co = new Coroutine(++ idx, stacksz, func);
    units_.insert(std::make_pair(idx, co));
    return co;
}

void Scheduler::RemoveCoroutine(Coroutine* cr)
{
    if (cr) {
        return RemoveCoroutine(cr->GetId());
    }
}

void Scheduler::RemoveCoroutine(int id)
{
    units_.erase(id);
}

void Scheduler::Resume(Coroutine* cr)
{
    assert(cr);

    switch (cr->GetStatus())
    {
        case Coroutine::S_INIT:
            cr->start();
            break;
        case Coroutine::S_SLEEP:
            current_ = cr;
            cr->resume();
            break;
        default:
            assert(0);
    }
}

void Scheduler::Yield(Coroutine* cr)
{
    current_ = NULL;
    cr->yield();
}

