#ifndef GNET_GATE_H_
#define GNET_GATE_H_

#include <stdint.h>
#include <string>

#include "src/router.h"
#include "src/gateio.h"

struct Gate : public GateBase
{
public:
    Gate();
    virtual ~Gate();

    int Init(const std::string& name, const std::string& file);

    int Poll();

    virtual void OnData(const std::string&, const GNET::PKGData&);
    virtual void OnDiscon(const std::string&);
    virtual void OnBuild(const std::string&);

private:
    std::string name_;
    Router* router_;
    GateIO* io_;
};

#endif

