#include "agilelog_priv.h"
#include <stdarg.h>

using namespace AGILELOG;

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_BUF_SIZE 1024

void Agile_printLog(int prio, const char *module, const char *caller, int line, const char *fmt, ...){
    va_list ap;
    char buf[LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    AgileLog::getInstance()->printLog(prio, module, caller, line, buf);
}

#ifdef __cplusplus
}
#endif


