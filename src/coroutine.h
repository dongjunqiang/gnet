#ifndef GNET_COROUTINE_H_
#define GNET_COROUTINE_H_

#include <functional>
#include <map>

#include <assert.h>
#include <ucontext.h>
#include <unistd.h>

#include "src/singleton.h"

namespace gnet {

class Scheduler;

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

    #define RESERVED_SIZE getpagesize()

    Coroutine(FUNC func, size_t stacksz = (32 << 10));
    virtual ~Coroutine();

    int GetId() const { return id_; }
    int GetStatus() const { return status_; }

    void Resume();
    void Yield();

private:
    void init_context();

    static void routine();

private:
    int id_;
    int status_;
    FUNC func_;
    SP stack_;
    size_t stacksz_;
    ucontext_t ctx_;
};

class Scheduler : public Singleton<Scheduler>
{
    typedef ::std::map<int, Coroutine*> MAP_T;
    typedef MAP_T::iterator ITER_T;
    typedef MAP_T::const_iterator CITER_T;

    friend class Coroutine;

public:
    Scheduler();
    ~Scheduler();

private:

    void AddCoroutine(Coroutine* cr) {
        if (cr) {
            units_.insert(std::make_pair(cr->GetId(), cr));
        }
    }

    void RemoveCoroutine(Coroutine* cr) {
        if (cr) {
            units_.erase(cr->GetId());
        }
    }

    Coroutine* GetCurrent() const { return current_; }
    void SetCurrent(Coroutine* cr) { current_ = cr; }

    ucontext_t* GetMain() { return &main_; }

private:
    ucontext_t main_;
    Coroutine* current_;
    MAP_T units_;
};

}

#endif

