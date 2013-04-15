#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "agilelog_priv.h"

#ifdef ANDROID
#include "cutils/log.h"
#endif

namespace AGILELOG {

#define DEFAULT_CONFIG_FILE_PATH "/system/etc"

AgileLog *AgileLog::sInstance = NULL;
pthread_mutex_t AgileLog::mMutex    = PTHREAD_MUTEX_INITIALIZER;

AgileLog *AgileLog::getInstance(){
    pthread_mutex_lock(&mMutex);
    if (NULL == sInstance){
        sInstance = new AgileLog;
    }
    pthread_mutex_unlock(&mMutex);
    return sInstance;
}

AgileLog::AgileLog(){
    mConfigFile.exists = 0;

    mConfigValue.config_output.type = OUTPUT_INVALID;
    mConfigValue.config_debug_level = DEBUG_LEVEL_WARNING;
    mConfigValue.config_timestamp_on = false;

    mConfigValue.moduleNum = 0;
    mConfigValue.pModules  = NULL;

    mpModuleHashT = NULL;
    
    init();
    printConfig();
}

AgileLog::~AgileLog(){

}

Error_t AgileLog::parseGlobalConfigElement(XMLElement *ele){ 
    const char *type = ele->FirstChildElement( "output" )->GetText();

    if(XML_SUCCESS == mXMLParsedDoc.ErrorID()){
        if (!strcmp(type, "file")){
            mConfigValue.config_output.type = OUTPUT_FILE;  
            strcpy(mConfigValue.config_output.filePath, ele->FirstChildElement( "output" )->FirstChildElement("path")->GetText());
            mWriteToLogFunc = WriteToFile;
        }else if (!strcmp(type, "logcat")){
            mConfigValue.config_output.type = OUTPUT_LOGCAT;
            mWriteToLogFunc = WriteToLogcat;
        }else if (!strcmp(type, "stdout")){
            mConfigValue.config_output.type = OUTPUT_STDOUT;
            mWriteToLogFunc = WriteToStdOut;
        }else if (!strcmp(type, "socket")){
            mConfigValue.config_output.type = OUTPUT_SOCKET;
            ele->FirstChildElement( "output" )->FirstChildElement("port")->QueryIntText(&mConfigValue.config_output.port);
            mWriteToLogFunc = WriteToSocket;
        }else{
            mConfigValue.config_output.type = OUTPUT_INVALID;
            mWriteToLogFunc            = NULL;
            return INVALID_OUTPUT;
        }
    }else{
        mConfigValue.config_output.type = OUTPUT_INVALID;
        mWriteToLogFunc            = NULL;
        return INVALID_OUTPUT;
    }

    if (ele->FirstChildElement( "debug" )->Attribute("level", "fatal")){
        mConfigValue.config_debug_level = DEBUG_LEVEL_FATAL;
    }else if (ele->FirstChildElement( "debug" )->Attribute("level", "error")){
        mConfigValue.config_debug_level = DEBUG_LEVEL_ERROR;
    }else if (ele->FirstChildElement( "debug" )->Attribute("level", "warning")){
        mConfigValue.config_debug_level = DEBUG_LEVEL_WARNING;
    }else if (ele->FirstChildElement( "debug" )->Attribute("level", "info")){
        mConfigValue.config_debug_level = DEBUG_LEVEL_INFO;
    }else if (ele->FirstChildElement( "debug" )->Attribute("level", "debug")){
        mConfigValue.config_debug_level = DEBUG_LEVEL_DEBUG;
    }else if (ele->FirstChildElement( "debug" )->Attribute("level", "verbose")){
        mConfigValue.config_debug_level = DEBUG_LEVEL_VERBOSE;
    }else{
        //if not matching any, set it to warning anyway!
        mConfigValue.config_debug_level = DEBUG_LEVEL_WARNING;
    }

    if (ele->FirstChildElement( "timestamp" )->Attribute("on", "true")){
        mConfigValue.config_timestamp_on = true;
    }else{
        mConfigValue.config_timestamp_on = false;
    }

    return NO_ERROR;
}

Error_t AgileLog::parseModuleConfigElement(XMLElement *eleIn){
    int count = 0;
    
    for( XMLNode* node = eleIn->FirstChild(); node; node = node->NextSibling() )
	{
		++count;
	}
    
    if(0 == count){
        printf("[%s]: no modules defined in xml", __FUNCTION__);
        return NO_MODULE;
    }
    
    mConfigValue.moduleNum = count;
    mConfigValue.pModules = (ModuleConfig_t *)malloc(count * sizeof(ModuleConfig_t));

    mpModuleHashT = new HashTable(count);
    
    count = 0;
    for( XMLNode* node = eleIn->FirstChild(); node; node = node->NextSibling() )
	{
		XMLElement* ele = node->ToElement();
        strcpy( mConfigValue.pModules[count].moduleName, ele->Attribute("name"));
        if (ele->FirstChildElement( "debug-switch" )->Attribute("on", "true")){
            mConfigValue.pModules[count].debugOn = true;
        }else{
            mConfigValue.pModules[count].debugOn = false;
        }

        if (ele->FirstChildElement( "debug-level" )->Attribute("level", "fatal")){
            mConfigValue.pModules[count].debugLevel = DEBUG_LEVEL_FATAL;
        }else if (ele->FirstChildElement( "debug-level" )->Attribute("level", "error")){
            mConfigValue.pModules[count].debugLevel = DEBUG_LEVEL_ERROR;
        }else if (ele->FirstChildElement( "debug-level" )->Attribute("level", "warning")){
            mConfigValue.pModules[count].debugLevel = DEBUG_LEVEL_WARNING;
        }else if (ele->FirstChildElement( "debug-level" )->Attribute("level", "info")){
            mConfigValue.pModules[count].debugLevel = DEBUG_LEVEL_INFO;
        }else if (ele->FirstChildElement( "debug-level" )->Attribute("level", "debug")){
            mConfigValue.pModules[count].debugLevel = DEBUG_LEVEL_DEBUG;
        }else if (ele->FirstChildElement( "debug-level" )->Attribute("level", "verbose")){
            mConfigValue.pModules[count].debugLevel = DEBUG_LEVEL_VERBOSE;
        }else{
            //if not matching any, set it to verbose anyway!
            mConfigValue.pModules[count].debugLevel = DEBUG_LEVEL_VERBOSE;
        }
        mpModuleHashT->buildHashTable((void *)&mConfigValue.pModules[count], mConfigValue.pModules[count].moduleName);
        
        ++count;
	}
    return NO_ERROR;
}

Error_t AgileLog::parseXMLConfig(){
    Error_t err = NO_ERROR;
    
    XMLElement* ele = mXMLParsedDoc.FirstChildElement()->FirstChildElement( "global_config" );

    if(XML_SUCCESS == mXMLParsedDoc.ErrorID())
        err = parseGlobalConfigElement(ele);
    else
        err = NO_MODULE;
    
    if (err != NO_ERROR){
        return err;
    }

    ele = mXMLParsedDoc.FirstChildElement()->FirstChildElement( "module_config" );

    if(XML_SUCCESS == mXMLParsedDoc.ErrorID())
        err = parseModuleConfigElement(ele);
    else
        err = NO_MODULE;
    
    return err;
}

Error_t AgileLog::init(){
    struct stat astat;
    Error_t err = NO_ERROR;
    
    sprintf(mConfigFile.name, "%s/agilelog.xml", 
             getenv("AGILELOG_PATH") ? getenv("AGILELOG_PATH") : DEFAULT_CONFIG_FILE_PATH);
    
    if(access(mConfigFile.name, R_OK)){
        printf("%s: can NOT access config file under %s\n. Using default setting!", __FUNCTION__, mConfigFile.name);
        mConfigFile.exists = 0;
    }else{
    	if (0 != stat(mConfigFile.name, &astat))
    	{
    		mConfigFile.exists = 0;
    	}else{
    	    mConfigFile.laseModifyTime = astat.st_ctime;
            mConfigFile.exists = 1;
        }
    }

    if (mConfigFile.exists){
        mXMLParsedDoc.LoadFile(mConfigFile.name);

        if(XML_SUCCESS != mXMLParsedDoc.ErrorID()){
            printf("%s: failed to load xml file [%s]. error = %d\n", __FUNCTION__, mConfigFile.name, mXMLParsedDoc.ErrorID());
            goto default_config;
        }

        err = parseXMLConfig();

        return err;
    }

default_config:
    setDefaultValue(); 
    return USE_DEFAULT_CONFIG;
}

void AgileLog::printConfig(){
    ModuleConfig_t *pM;
    printf("\n\t------AgileLog Configuration------\n");
    printf("\tGlobal Config:\n");
    printf("\t  |-output\n");
    printf("\t      |-type = %d\n", mConfigValue.config_output.type);
    printf("\t      |-file path = %s\n", mConfigValue.config_output.filePath);
    printf("\t      |-port = %d\n", mConfigValue.config_output.port);
    printf("\t  |-debug level = %d\n", mConfigValue.config_debug_level);
    printf("\t  |-add timestamp = %d\n", mConfigValue.config_timestamp_on);
    printf("\tModules [%d]:\n", mConfigValue.moduleNum);

    for (int i = 0; i < mConfigValue.moduleNum; i++){
        pM = (ModuleConfig_t *)mpModuleHashT->getHashItem(mConfigValue.pModules[i].moduleName);
        printf("\t  |-module: %s\n", pM->moduleName);
        printf("\t      |-debug on: %d\n", pM->debugOn);
        printf("\t      |-debug level: %d\n", pM->debugLevel);
    }

    
}
void AgileLog::WriteToLogcat(int prio, const char *module, const char *buffer){
#ifdef ANDROID
    android_printLog(prio, module, "%s", buffer);
#endif
}

void AgileLog::WriteToFile(int prio, const char *module, const char *buffer){

}

void AgileLog::WriteToStdOut(int prio, const char *module, const char *buffer){
    char logBuf[2048];
    sprintf(logBuf, "[%s]\t%s\n", module, buffer);
    printf("%s", logBuf);
}

void AgileLog::WriteToSocket(int prio, const char *module, const char *buffer){

}


void AgileLog::printLog(int prio, const char *module, const char *caller, int line, const char *printData){
    char logBuf[2048];
    ModuleConfig_t *pM = NULL;
    struct timeval tv;
    struct timezone tz;
    struct tm *tm;

    //printf("enter printLog: prio = %d, module = %s, printData = %s\n", prio, module, printData);
    
    if(mConfigValue.moduleNum > 0){
        pM = (ModuleConfig_t *)mpModuleHashT->getHashItem(module);
        //print nothing
        if (NULL != pM){
            if ((!pM->debugOn) || (prio < pM->debugLevel))
                return;
        }else{
            if (prio < mConfigValue.config_debug_level)
                return;
        }
    }else{
        if (prio < mConfigValue.config_debug_level)
            return;
    }    
    
    memset(logBuf, 0, 2048);
    if (mConfigValue.config_timestamp_on){
        gettimeofday(&tv, &tz);
        tm=localtime(&tv.tv_sec);

        sprintf(logBuf, "[%d-%d %d:%d:%d:%03ld [%s : %d]] ", tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, 
                        tm->tm_min, tm->tm_sec, (suseconds_t)(tv.tv_usec/1000), caller, line);
        
    }else{
        sprintf(logBuf, "[%s : %d] ", caller, line);
    }
    
    strcat(logBuf, printData);
    if (mWriteToLogFunc)
        mWriteToLogFunc(prio, module, logBuf);   
}

void AgileLog::setDefaultValue(){
    mConfigValue.config_output.type  = OUTPUT_STDOUT;
    mWriteToLogFunc = WriteToStdOut;
    mConfigValue.config_debug_level  = DEBUG_LEVEL_WARNING;
    mConfigValue.config_timestamp_on = false;

    mConfigValue.moduleNum = 0;
    mConfigValue.pModules  = NULL;
}

}

