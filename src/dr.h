#ifndef GNET_DR_H_
#define GNET_DR_H_

namespace gnet {
namespace proto { class PKG; }

class DR
{
public:
    static bool ntoh(const char* buffer, int& len, proto::PKG& pkg);
    static bool hton(const proto::PKG& pkg, char* buffer, int& len);
};

}

#endif
