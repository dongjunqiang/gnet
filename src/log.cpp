#include <time.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

using namespace gnet;

#define MAX_LOG_BUFFER 1024

void Log::DoLog(int level, struct timeval* now, const char* fmt, ...)
{
    if (level >= level_) {
        char buffer[MAX_LOG_BUFFER];

        struct tm now_tm;
        localtime_r((time_t*)&now->tv_sec, &now_tm);
        snprintf(buffer, sizeof(buffer),
           "[%d-%02d-%02d %02d:%02d:%02d:%06d][%s:%d:%s] ",
            now_tm.tm_year + 1900,
            now_tm.tm_mon + 1,
            now_tm.tm_mday,
            now_tm.tm_hour,
            now_tm.tm_min,
            now_tm.tm_sec,
            (int)now->tv_usec,
            __FILE__, __LINE__, __FUNCTION__);

        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buffer + strlen(buffer), MAX_LOG_BUFFER - 1 - strlen(buffer), fmt, ap);
        va_end(ap);
        strcat(buffer, "\n");

        write(fd_, buffer, strlen(buffer));
    }
}
