#include <stdio.h>
#ifdef __linux
#include "simple_logger_info.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>

// Beijing time: UTC+8
static int time_zone = +8;

// calculate time zone and set it 
void set_time_zone()
{
    time_t raw_time;
    struct tm t_utc;
    struct tm t_local;
    time(&raw_time);
    gmtime_r(&raw_time, &t_utc);
    localtime_r(&raw_time, &t_local);
    time_zone = t_local.tm_hour - t_utc.tm_hour;
    if (time_zone < -12) {
        time_zone += 24;
    }
    if (time_zone > 12) {
        time_zone -= 24;
    }
    // printf("time zone %d", time_zone);
}

inline void get_current_time_str(char* time_str)
{
    // static char time_str[32];
    
#if 1
    time_t raw_time;
    struct tm timeinfo;
    time(&raw_time);
    // to make it faster
    // do not use localtime, use gmtime instead
    // timeinfo = gmtime(&raw_time);
    gmtime_r(&raw_time, &timeinfo);
    timeinfo.tm_hour += time_zone;
    if (timeinfo.tm_hour < 0) {
        timeinfo.tm_mday -= 1;
        timeinfo.tm_hour += 24;
    }
    if(timeinfo.tm_hour>24){
        timeinfo.tm_mday += 1;
        timeinfo.tm_hour -= 24;
    }
    // timeinfo = localtime(&raw_time);
    //example: [2020/1/22 12:13:14]
    snprintf(time_str, LOG_TIME_STR_LENGTH, "[%04d/%02d/%02d %02d:%02d:%02d]",
        timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
        timeinfo.tm_hour , timeinfo.tm_min, timeinfo.tm_sec);
#endif
    // snprintf(time_str, LOG_TIME_STR_LENGTH, "[2020/1/22 12:13:14]");
    // strftime(time_str, LOG_TIME_STR_LENGTH, "[%Y/%m/%d %H:%M:%S]", timeinfo);
    return;
    // return time_str;
}

inline int get_pid()
{
    return getpid();
}

inline unsigned long inline get_tid()
{
    return pthread_self();
}

#endif