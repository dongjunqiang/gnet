#ifndef GNET_FACTORY_H_
#define GNET_FACTORY_H_

#include "actor.h"

namespace gnet {

class Factory
{
public:
    static Actor* GenClient(const std::string& name, const std::string& host, int16_t port);
    static Actor* GenMaster(const std::string& name, const std::string& host, int16_t port);
};

}

#endif
