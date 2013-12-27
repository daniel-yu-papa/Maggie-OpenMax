#ifndef __AGILE_LOG_H__
#define __AGILE_LOG_H__

#include "agilelog_def.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#ifndef MODULE_TAG
#define MODULE_TAG "NULL"
#endif

#ifdef __cplusplus
inline char *CPPFuncName(const char *prettyFunction, const char *func){
    char *str = strdup(prettyFunction);
    char *tmp1 = strtok(str, ":");
    char *result = strrchr(tmp1, ' ');
    if (NULL == result)
        result = tmp1;
    sprintf(result, "%s::%s", result, func);
    return result;
} 
    #undef __FUNCTION_NAME__
    #define __FUNCTION_NAME__  CPPFuncName(__PRETTY_FUNCTION__, __FUNCTION__)
#else
    #undef __FUNCTION_NAME__
    #define __FUNCTION_NAME__  __FUNCTION__
#endif


#ifndef AGILE_LOGV
#define AGILE_LOGV(...) ((void)AGILE_LOG(LOG_VERBOSE, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGD
#define AGILE_LOGD(...) ((void)AGILE_LOG(LOG_DEBUG, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGI
#define AGILE_LOGI(...) ((void)AGILE_LOG(LOG_INFO, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGW
#define AGILE_LOGW(...) ((void)AGILE_LOG(LOG_WARN, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGE
#define AGILE_LOGE(...) ((void)AGILE_LOG(LOG_ERROR, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOG_FATAL
#define AGILE_LOG_FATAL(...) ((void)AGILE_LOG(LOG_FATAL, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOG
#define AGILE_LOG(priority, module, ...) \
    Agile_printLog(AGILE_##priority, module, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#endif

#define MODULE_TAG_D(tag) \
    "#ifdef MODULE_TAG"  \
    "#undef MODULE_TAG"  \
    "#endif"             \
    "#define MODULE_TAG ##tag"

#endif