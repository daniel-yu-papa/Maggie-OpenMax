#ifndef __MAG_AGILE_LOG_IMPL_H__
#define __MAG_AGILE_LOG_IMPL_H__

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>

#include "tinyxml2.h"
#include "Mag_hashTable.h"

using namespace tinyxml2;

namespace MAGAGILELOG {

typedef enum {
    NO_ERROR = 0,
    FAILURE  = 1,
    INVALID_OUTPUT = 2,
    USE_DEFAULT_CONFIG = 3,
    NO_MODULE = 4,
}Error_t;

typedef enum {
    OUTPUT_LOGCAT  = 0x00,
    OUTPUT_FILE    = 0x01,
    OUTPUT_STDOUT  = 0x02,
    OUTPUT_SOCKET  = 0x04,
    OUTPUT_INVALID = 0x08
}OutputType_t;

typedef enum {
    DEBUG_LEVEL_FATAL   = 7,
    DEBUG_LEVEL_ERROR   = 6,
    DEBUG_LEVEL_WARNING = 5,
    DEBUG_LEVEL_INFO    = 4,
    DEBUG_LEVEL_DEBUG   = 3,
    DEBUG_LEVEL_VERBOSE = 2
}DebugLevel_t;


typedef struct {
    char name[256];
    time_t laseModifyTime;
    int exists;
}LogConfigFile_t;

typedef struct {
    OutputType_t type;
    char filePath[16];
    int  port;
}Output_t;

typedef struct {
    char          moduleName[64];
    bool          debugOn;
    DebugLevel_t  debugLevel;
}ModuleConfig_t;

typedef struct {
    Output_t     config_output;
    DebugLevel_t config_debug_level;
    bool         config_timestamp_on;
    bool         log_on;
    int          log_size;

    int            moduleNum;
    ModuleConfig_t *pModules;
}ConfigTable_t;

typedef void (*fnWriteToLog)(int prio, char level, const char *module, const char *buffer, void *thiz);

class MagAgileLog{
public:
    MagAgileLog();
    ~MagAgileLog();
    
    void printLog(int prio, const char *module, const char *caller, int line, const char *printData);

    static MagAgileLog *getInstance();
    static void destroy();
    
    int mLogFile1; 
    int mLogFile2;
    int mUsingLogFile;
    unsigned int mWriteSize;
    
    LogConfigFile_t mConfigFile;
private:
    static MagAgileLog *sInstance;
    static pthread_mutex_t mMutex;
    
    Error_t init();
    void printConfig();
    void setDefaultValue();
    Error_t parseXMLConfig();
    Error_t parseGlobalConfigElement(XMLElement *ele);
    Error_t parseModuleConfigElement(XMLElement *ele);

    static void WriteToLogcat(int prio, char level, const char *module, const char *buffer, void *thiz);
    static void WriteToFile(int prio, char level, const char *module, const char *buffer, void *thiz);
    static void WriteToStdOut(int prio, char level, const char *module, const char *buffer, void *thiz);
    static void WriteToSocket(int prio, char level, const char *module, const char *buffer, void *thiz);
    
    char getPriorityLevel(int prio);

    ConfigTable_t mConfigValue;
    XMLDocument mXMLParsedDoc;
    HashTableHandle mpModuleHashT;

    fnWriteToLog mWriteToLogFunc;
};

}
#endif
