#include "proto/errcode.pb.h"

#include "gw/gw.h"

// TODO: temporary

int main(int argc, char** argv)
{
    GW* gw = new GW("gw-m1");

    int ret = gw->Init();
    printf("gw init: %d\n", ret);
    if (ret != GNET::SUCCESS)
        return ret;

    while (true) {
        ret = gw->Poll();
        if (ret != GNET::SUCCESS) {
            printf("gw poll: %d\n", ret);
            return ret;
        } else if (ret != GNET::BUSY) {
            usleep(10);
        }
    }

    delete gw;
}
