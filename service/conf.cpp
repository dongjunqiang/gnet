#include <stdlib.h>
#include <fcntl.h>

#include "service/conf.h"

int Conf::Init(const std::string& file)
{
    return Reload(file);
}

int Conf::Reload(const std::string& file)
{
    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) {
        return GNET::ERR_FILE_NOT_EXIST;
    }

    google::protobuf::io::FileInputStream fi(fd);
    fi.SetCloseOnDelete(true);

    cfg_.Clear();
    if (!google::protobuf::TextFormat::Parse(&fi, &cfg_)) {
        return GNET::ERR_CFG_FILE_PARSE;
    }

    // check global
    if (cfg_.world() > 0xFFFFF) {
        return GNET::ERR_CFG_ID_EXCEED_LIMIT;
    }

    sids_.clear();
    names_.clear();

    for (int i = 0; i < cfg_.service_size(); ++ i) {
        // check service
        if (cfg_.service(i).machine() > 0xFFFF
            || cfg_.service(i).instance() > 0xFFFF) {
            return GNET::ERR_CFG_ID_EXCEED_LIMIT;
        }

        // sid <--> cfg mapping
        // name <--> sid mapping
        sid_t sid = SID(cfg_.world(),
            cfg_.service(i).machine(),
            cfg_.service(i).type(),
            cfg_.service(i).instance());
        sids_[sid] = &cfg_.service(i);
        names_[cfg_.service(i).name()] = sid;
    }

    for (int i = 0; i < cfg_.service_size(); ++ i) {
        // master sid
        if (cfg_.service(i).type() == GNET::ST_MASTER) {
            master_sid_ = names_[cfg_.service(i).name()];
        }

        // gw tree mapping
        if (cfg_.service(i).type() == GNET::ST_GW) {
            const GNET::CONF_GW& gw = cfg_.service(i).gw();
            for (int j = 0; j < gw.children_size(); ++ j) {
                if (names_.find(gw.children(j)) == names_.end()) {
                    return GNET::ERR_CFG_NAME_NOT_FOUND;
                }
                sid_t parent = names_[cfg_.service(i).name()];
                sid_t child = names_[gw.children(j)];
                gws_.insert(std::make_pair(parent, child));
            }
        }
    }

    return GNET::SUCCESS;
}

const GNET::CONF_SERVICE* Conf::GetServiceByName(const std::string& name) const
{
    sid_t sid = GetSidByName(name);
    return sid == SID_RESERVED ? NULL : GetServiceBySid(sid);
}

const GNET::CONF_SERVICE* Conf::GetServiceBySid(sid_t sid) const
{
    SID_MAP_T::const_iterator it = sids_.find(sid);
    return it == sids_.end() ? NULL : it->second;
}

sid_t Conf::GetSidByName(const std::string& name) const
{
    NAME_MAP_T::const_iterator it = names_.find(name);
    return it == names_.end() ? SID_RESERVED : it->second;
}

