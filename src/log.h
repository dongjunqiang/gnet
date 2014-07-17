#ifndef GNET_LOG_H_
#define GNET_LOG_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>

namespace gnet {

#define DEBUG 0
#define ERROR 1

class Log
{
public:
    Log() : fd_(fileno(stdout)), level_(DEBUG) {}
    ~Log() {}

    void set_level(int level) { level_ = level; }
    void DoLog(int level, struct timeval* now, const char* fmt, ...);

private:
    int fd_;
    int level_;
};

#define log(log_inst, level, fmt, ...) \
    do { \
        struct timeval now; \
        gettimeofday(&now, NULL); \
        log_inst->DoLog(level, &now, fmt, ##__VA_ARGS__); \
    } while (0)

}

#endif
