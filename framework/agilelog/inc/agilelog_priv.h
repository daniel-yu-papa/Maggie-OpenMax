#ifndef __AGILE_LOG_PRIV_H__
#define __AGILE_LOG_PRIV_H__

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>

#include "tinyxml2.h"
#include "hashTable.h"

using namespace tinyxml2;

namespace AGILELOG {

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

    int            moduleNum;
    ModuleConfig_t *pModules;
}ConfigTable_t;

typedef void (*fnWriteToLog)(int prio, const char *module, const char *buffer);

class AgileLog{
public:
    AgileLog();
    ~AgileLog();
    
    void printLog(int prio, const char *module, const char *caller, int line, const char *printData);

    static AgileLog *getInstance();
    
private:
    static AgileLog *sInstance;
    static pthread_mutex_t mMutex;
    
    Error_t init();
    void printConfig();
    void setDefaultValue();
    Error_t parseXMLConfig();
    Error_t parseGlobalConfigElement(XMLElement *ele);
    Error_t parseModuleConfigElement(XMLElement *ele);

    static void WriteToLogcat(int prio, const char *module, const char *buffer);
    static void WriteToFile(int prio, const char *module, const char *buffer);
    static void WriteToStdOut(int prio, const char *module, const char *buffer);
    static void WriteToSocket(int prio, const char *module, const char *buffer);
    
    LogConfigFile_t mConfigFile;
    ConfigTable_t mConfigValue;
    XMLDocument mXMLParsedDoc;
    HashTableHandle mpModuleHashT;

    fnWriteToLog mWriteToLogFunc;
};

}
#endif
