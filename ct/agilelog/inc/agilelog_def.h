#ifndef __AGILE_LOG_DEF_H__
#define __AGILE_LOG_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum agilelog_LogPriority {
    AGILE_LOG_UNKNOWN = 0,
    AGILE_LOG_DEFAULT = 1,
    AGILE_LOG_VERBOSE = 2,
    AGILE_LOG_DEBUG   = 3,
    AGILE_LOG_INFO    = 4,
    AGILE_LOG_WARN    = 5,
    AGILE_LOG_ERROR   = 6,
    AGILE_LOG_FATAL   = 7,
}agilelog_LogPriority;

void Agile_printLog(int prio, const char *module, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif