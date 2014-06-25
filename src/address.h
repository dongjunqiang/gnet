#ifndef GNET_ADDRESS_H_
#define GNET_ADDRESS_H_

#include <string>

#include "proto/gnet.pb.h"

int ParseAddress(const std::string& addr_cfg, GNET::Address& addr);

#endif
