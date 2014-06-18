#ifndef GNET_CONF_H_
#define GNET_CONF_H_

#include <map>
#include <string>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "proto/errcode.pb.h"
#include "conf/conf.pb.h"
#include "service/sid.h"

class Conf
{
    typedef std::map<sid_t, const GNET::CONF_SERVICE*> SID_MAP_T;
    typedef std::map<std::string, sid_t> NAME_MAP_T;
    typedef std::multimap<sid_t, sid_t> GW_TREE_T;

public:
    Conf() {}
    ~Conf() {}

    int Reload(const std::string& file);

    const GNET::CONF_GLOBAL* Global() const { return &cfg_.global(); }

    const GNET::CONF_SERVICE* GetServiceByName(const std::string& name) const;
    const GNET::CONF_SERVICE* GetServiceBySid(sid_t) const;

    sid_t GetSidByName(const std::string& name) const;

private:
    GNET::CONF cfg_;
    SID_MAP_T sids_;
    NAME_MAP_T names_;
    GW_TREE_T gws_;
};

#endif
