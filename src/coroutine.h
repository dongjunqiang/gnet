#ifndef GNET_COROUTINE_H_
#define GNET_COROUTINE_H_

#include <ucontext.h>
#include <sys/mman.h>

namespace gnet {

class CoroutineManager;

class Coroutine
{
public:
    enum {
        S_DEAD = 0,
        S_INIT = 1,
        S_AWAKE = 2,
        S_SLEEP = 3,
    };

    typedef std::function<void (void)> FUNC;
    typedef char* SP;

    friend class CroutineManager;
       
private:
    Coroutine(CoroutineManager* manager, FUNC func) : func_(func),
                                                    , manager_(manager)
    {}

    virtual ~Coroutine() {}


private:
    int status_;
    FUNC func_;
    SP stack_;
    ucontext_t ctx_;
    CoroutineManager* manager_; 
};

class CoroutineManager
{
public:
    CoroutineManager(size_t stack_size) : stack_size_(stack_size)
                                        , current_(-1)
    {
    }

    virtual ~CoroutineManager() {}

    Coroutine* CreateCoroutine(Coroutine::FUNC func)
    {
        Coroutine* co = new Coroutine(func);
    }


private:
    ucontext_t main_;
    size_t stack_size_;
    int current_;
    ::std::vector<Coroutine*> units_;
};


}

#endif

