#ifndef GNET_HANDLE_H_
#define GNET_HANDLE_H_

namespace gnet {

#define STACK_SIZE (1 << 20)

class GNet;
class Coroutine;

class Handle
{
public:
    Handle(GNet* gnet);
    virtual ~Handle();

    Coroutine* get_in() const { return in_; }
    Coroutine* get_out() const { return out_; }

    void Start();

protected:
    virtual void proc_in() = 0;
    virtual void proc_out() = 0;

protected:
    GNet* gnet_;
    int fd_;
    Coroutine* in_;
    Coroutine* out_;
};

}

#endif
