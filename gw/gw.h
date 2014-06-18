#ifndef GNET_GW_H_
#define GNET_GW_H_

#include "service/svc.h"

class GW : public BaseSvc
{
public:
    GW(const std::string& name);
    virtual ~GW();

    virtual int InitImpl();

    virtual int ReloadImpl()
    {
        return GNET::SUCCESS;
    }

    virtual int PollImpl()
    {
        return GNET::SUCCESS;
    }
};

#endif
