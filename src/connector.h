#ifndef GNET_CONNECTOR_H_
#define GNET_CONNECTOR_H_

#include "handle.h"

namespace gnet {

class Reactor;
class Buffer;

class Connector : public Handle
{
public:
    Connector(Reactor* reactor, int fd);
    virtual ~Connector();

    // return 0, success
    // return < 0, fail
    int Send(const char* buffer, int len);

    // return bytes should be read
    virtual int OnRead(const char* buffer, int len);

    virtual void OnDisconnect();

private:
    void proc_in();
    void proc_out();

private:
    Buffer* rbuf_;
    Buffer* wbuf_;
};

}

#endif
