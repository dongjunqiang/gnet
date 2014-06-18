#ifndef GNET_SID_H_
#define GNET_SID_H_

#include <gbase/logic/bus.h>

// int64_t sid
//
// -----------------------------------------------------
// |  16 bytes  |  16 bytes  |  16 bytes  |  16 bytes  |
// -----------------------------------------------------
// |            |            |    type    |  instance  |
// |  world id  | machine id |--------------------------
// |            |            |      bus address        |
// -----------------------------------------------------

typedef int64_t sid_t;

#define SID_BUS_ADDR(sid)   ((sid << 32) >> 32)

#define SID_WORLD(sid)      (sid >> 48)
#define SID_MACHINE(sid)    ((sid << 16) >> 48)
#define SID_TYPE(sid)       bus_addr_type(ID_BUS_ADDR(sid))
#define SID_INSTANCE(sid)   bus_addr_id(ID_BUS_ADDR(sid))

#define SID(world, machine, type, instance) \
    (((sid_t)(world) << 48) \
    | ((sid_t)(machine) << 32) \
    | ((sid_t)(type) << 16) \
    | ((sid_t)(instance)))

#define SID_RESERVED 0

#endif
