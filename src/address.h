#ifndef GNET_ADDRESS_H_
#define GNET_ADDRESS_H_

#include <string>

namespace gnet {
namespace proto { class Address; }

int ParseAddress(const std::string& addr_cfg, proto::Address& addr);

}

#endif
