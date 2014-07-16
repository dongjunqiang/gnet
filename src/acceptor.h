#ifndef GNET_ACCEPTOR_H_
#define GNET_ACCEPTOR_H_

#include <stdint.h>
#include <string>

#include "handle.h"

namespace gnet {
namespace proto { class TCP; }

class Actor;
class Acceptor : public Handle
{
public:
    Acceptor(Actor* actor, const proto::TCP& addr);
    virtual ~Acceptor();

    virtual void OnAccept(int fd);

private:
    void proc_in();
    void proc_out();

private:
    Actor* actor_;
};

}

#endif
