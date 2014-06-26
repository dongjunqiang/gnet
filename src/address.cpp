#include "src/address.h"

#define TCP_HEAD "tcp://"

int ParseAddress(const std::string& addr_cfg,
                 GNET::Address& addr)
{
    if (addr_cfg.find(TCP_HEAD) == 0) {
        std::string host_port = addr_cfg.substr(strlen(TCP_HEAD));
        unsigned pos = host_port.find(':');
        if (pos == std::string::npos || pos == 0 || pos == host_port.size()) {
            return GNET::ERR_ADDRESS_INVALID;
        }
        addr.mutable_tcp()->set_host(host_port.substr(0, pos));
        std::string port = host_port.substr(pos + 1);
        addr.mutable_tcp()->set_port(atoi(port.c_str()));
        return GNET::SUCCESS;
    }

    // others not support
    return GNET::ERR_ADDRESS_NOT_SUPPORT;
}

