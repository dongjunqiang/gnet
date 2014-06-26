#ifndef GNET_GATE_H_
#define GNET_GATE_H_

#include <stdint.h>
#include <string>

#include "src/router.h"

struct Gate
{
public:
    Gate();
    virtual ~Gate();

    int Init(const std::string& name, const std::string& file);
    int Reload(const std::string& name, const std::string& file);

    int Poll();

private:
    int reg_to_parent();

private:
    std::string name_;
    Router* router_;
};

#endif

