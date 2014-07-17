#ifndef GNET_COROUTINE_H_
#define GNET_COROUTINE_H_

#include <functional>
#include <map>

#include <assert.h>
#include <ucontext.h>
#include <unistd.h>

namespace gnet {

class GNet;
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

    Coroutine(GNet* gnet, FUNC func, size_t stacksz = (32 << 10));
    virtual ~Coroutine();

    int get_status() const { return status_; }

    void Resume();
    void Yield();

private:
    void init_context();
    static void main(uint32_t hi32, uint32_t low32);

private:
    int status_;
    FUNC func_;
    SP stack_;
    size_t stacksz_;
    ucontext_t ctx_;
    GNet* gnet_;
};

}

#endif

