#ifndef GNET_LOG_H_
#define GNET_LOG_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>

#include "singleton.h"

namespace gnet {

#define DEBUG 0
#define ERROR 1

class Log : public Singleton<Log>
{
protected:
    Log() : fd_(fileno(stdout)), level_(DEBUG) {}
    ~Log() {}

public:
    void set_level(int level) { level_ = level; }

    void DoLog(int level, struct timeval* now, const char* fmt, ...);

private:
    int fd_;
    int level_;
};

#define log(level, fmt, ...) \
    do { \
        struct timeval now; \
        gettimeofday(&now, NULL); \
        Log::Instance()->DoLog(level, &now, fmt, ##__VA_ARGS__); \
    } while (0)

#define debug(fmt, ...) log(DEBUG, fmt, ##__VA_ARGS__)
#define error(fmt, ...) log(ERROR, fmt, ##__VA_ARGS__)

}

#endif
