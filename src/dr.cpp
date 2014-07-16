#include <google/protobuf/stubs/common.h>

#include "proto/gnet.pb.h"
#include "log.h"
#include "dr.h"

using namespace gnet;
using namespace google::protobuf;

bool DR::ntoh(const char* buffer, int& len, proto::PKG& pkg)
{
    int nhead = sizeof(int);
    if (len < nhead)
        return false;

    int nbody = *(const int*)(buffer + nhead);
    if (len < nhead + nbody)
        return false;

    const void* data = buffer + nhead;
    if (!pkg.ParseFromArray(data, nbody)) {
        error("%s", pkg.InitializationErrorString().c_str());
        assert(0);
    }

    len = nhead + nbody;
    return true;
}

bool DR::hton(const proto::PKG& pkg, char* buffer, int& len)
{
    int nwrite = pkg.ByteSize() + sizeof(int);
    if (len < nwrite)
        return false;

    *(int*)buffer = pkg.ByteSize();
    pkg.SerializeWithCachedSizesToArray((uint8*)buffer + sizeof(int));

    len = nwrite;
    return true;
}

