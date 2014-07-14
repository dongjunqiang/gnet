#ifndef GNET_ACCEPTOR_H_
#define GNET_ACCEPTOR_H_

#include <stdint.h>
#include <string>

#include "handle.h"

namespace gnet {

class Reactor;
class Acceptor : public Handle
{
public:
    Acceptor(Reactor* reactor, const std::string& host, int16_t port);
    virtual ~Acceptor();

private:
    void proc_in();
    void proc_out();
};

}

#endif
