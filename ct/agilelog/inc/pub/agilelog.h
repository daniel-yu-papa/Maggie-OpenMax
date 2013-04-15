#ifndef __AGILE_LOG_H__
#define __AGILE_LOG_H__

#include "agilelog_def.h"
#include <stdarg.h>

#ifndef MODULE_TAG
#define MODULE_TAG "NULL"
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
    Agile_printLog(AGILE_##priority, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif


#endif