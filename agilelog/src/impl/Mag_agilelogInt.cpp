#include "Mag_agilelogImpl.h"
#include <stdarg.h>
#include <stdio.h>

using namespace MAGAGILELOG;

#define LOG_BUF_SIZE 1024

static MagAgileLog *gpAgile = NULL;

#ifdef __cplusplus
extern "C" {
#endif

void Mag_agilelogPrint(int prio, const char *module, const char *caller, int line, const char *fmt, ...){
    va_list ap;
    char buf[LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    if (gpAgile)
        gpAgile->printLog(prio, module, caller, line, buf);
}

void Mag_agilelogCreate(){
    gpAgile = MagAgileLog::getInstance();
}

void Mag_agilelogDestroy(){
	MagAgileLog::destroy();
    gpAgile = NULL;
}

#ifdef __cplusplus
}
#endif

