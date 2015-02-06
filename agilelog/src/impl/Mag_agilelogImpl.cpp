/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

#include "Mag_agilelogImpl.h"

#ifdef ANDROID
#include "android/log.h"
#endif

namespace MAGAGILELOG {

/*You can export AGILELOG_PATH=xxx to specify the path of the configure file*/
#define DEFAULT_CONFIG_FILE_PATH "/etc/mag"

MagAgileLog *MagAgileLog::sInstance = NULL;
pthread_mutex_t MagAgileLog::mMutex    = PTHREAD_MUTEX_INITIALIZER;

MagAgileLog *MagAgileLog::getInstance(){
    pthread_mutex_lock(&mMutex);
    if (NULL == sInstance){
        sInstance = new MagAgileLog;
    }
    pthread_mutex_unlock(&mMutex);
    return sInstance;
}

void MagAgileLog::destroy(){
    pthread_mutex_lock(&mMutex);
    if (NULL != sInstance){
        delete sInstance;
        sInstance = NULL;
    }
    pthread_mutex_unlock(&mMutex);
}

MagAgileLog::MagAgileLog(){
    mConfigFile.exists = 0;

    mConfigValue.config_output.type = OUTPUT_INVALID;
    mConfigValue.config_debug_level = DEBUG_LEVEL_WARNING;
    mConfigValue.config_timestamp_on = false;

    mConfigValue.moduleNum = 0;
    mConfigValue.pModules  = NULL;

    mpModuleHashT = NULL;
    mLogFile1 = -1;
    mLogFile2 = -1;
    mUsingLogFile = -1;
    mWriteSize = 0;
    pthread_mutex_init(&mLogMutex, NULL);

    init();
    /*printConfig();*/
}

MagAgileLog::~MagAgileLog(){
    printf("~MagAgileLog: enter!\n");
    pthread_mutex_lock(&mLogMutex);
    if (mLogFile1 > 0){
        printf("~MagAgileLog: close log file 1\n");
        close(mLogFile1);
    }

    if (mLogFile2 > 0){
        printf("~MagAgileLog: close log file 2\n");
        close(mLogFile2);
    }

    pthread_mutex_destroy(&mLogMutex);
    destroyMagStrHashTable(&mpModuleHashT);
    printf("~MagAgileLog: exit!\n");
}

Error_t MagAgileLog::parseGlobalConfigElement(XMLElement *ele){ 
    const char *type = ele->FirstChildElement( "output" )->Attribute("type");
    int logSize = 0;
    XMLElement *child;
    XMLError res;

    if(XML_SUCCESS == mXMLParsedDoc.ErrorID()){
        if (!strcmp(type, "file")){
            char filePath[512];
            mConfigValue.config_output.type = OUTPUT_FILE;  
            strcpy(mConfigValue.config_output.filePath, ele->FirstChildElement( "output" )->FirstChildElement("path")->Attribute("name"));

            sprintf(filePath, "%s_1.log", mConfigValue.config_output.filePath);
            mLogFile1 = open(filePath, O_CREAT | O_RDWR | O_TRUNC);
            if (-1 == mLogFile1){
                printf("Failed to open the log file 1: %s\n\n", filePath);
            }else{
                printf("Create the log file 1: %s -- OK!\n", filePath);
            }

            sprintf(filePath, "%s_2.log", mConfigValue.config_output.filePath);
            mLogFile2 = open(filePath, O_CREAT | O_RDWR | O_TRUNC);
            if (-1 == mLogFile2){
                printf("Failed to open the log file 2: %s\n\n", filePath);
            }else{
                printf("Create the log file 2: %s -- OK!\n", filePath);
            }

            mUsingLogFile = mLogFile1;
            mWriteSize = 0;
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

    if (ele->FirstChildElement( "log" )->Attribute("on", "true")){
        mConfigValue.log_on = true;
    }else{
        mConfigValue.log_on = false;
    }

    child = ele->FirstChildElement( "log_size" );
    if (child){
        res = child->QueryIntText( &logSize );
        if (XML_SUCCESS == res){
            mConfigValue.log_size = logSize * 1024 * 1024;
        }else{
            printf("failed to get the value of the element: log_size\n");
            return FAILURE;;
        }
    }else{
        printf("failed to get the element: log_size\n");
        return FAILURE;
    }
    return NO_ERROR;
}

Error_t MagAgileLog::parseModuleConfigElement(XMLElement *eleIn){
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

    mpModuleHashT = createMagStrHashTable(count);
    
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
        mpModuleHashT->addItem(mpModuleHashT, (void *)&mConfigValue.pModules[count], mConfigValue.pModules[count].moduleName);
        ++count;
	}
    return NO_ERROR;
}

Error_t MagAgileLog::parseXMLConfig(){
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

Error_t MagAgileLog::init(){
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

void MagAgileLog::printConfig(){
    ModuleConfig_t *pM;
    printf("\n\t------MagAgileLog Configuration------\n");
    printf("\tGlobal Config:\n");
    printf("\t  |-output\n");
    printf("\t      |-type = %d\n", mConfigValue.config_output.type);
    printf("\t      |-file path = %s\n", mConfigValue.config_output.filePath);
    printf("\t      |-port = %d\n", mConfigValue.config_output.port);
    printf("\t  |-debug level = %d\n", mConfigValue.config_debug_level);
    printf("\t  |-add timestamp = %d\n", mConfigValue.config_timestamp_on);
    printf("\t  |-add log = %d\n", mConfigValue.log_on);
    printf("\t  |-add log_size = %d MB\n", mConfigValue.log_size);
    printf("\tModules [%d]:\n", mConfigValue.moduleNum);

    for (int i = 0; i < mConfigValue.moduleNum; i++){
        pM = (ModuleConfig_t *)mpModuleHashT->getItem(mpModuleHashT, mConfigValue.pModules[i].moduleName);
        printf("\t  |-module: %s\n", pM->moduleName);
        printf("\t      |-debug on: %d\n", pM->debugOn);
        printf("\t      |-debug level: %d\n", pM->debugLevel);
    }

    
}
void MagAgileLog::WriteToLogcat(int prio, char level, const char *module, const char *buffer, void *thiz){
#ifdef ANDROID
    __android_log_print(prio, module, "%s", buffer);
#endif
}

void MagAgileLog::WriteToFile(int prio, char level, const char *module, const char *buffer, void *thiz){
    char logBuf[2048];
    int length;
    MagAgileLog *obj = static_cast<MagAgileLog *>(thiz);

    sprintf(logBuf, "[%s/%c]\t%s\n", module, level, buffer);
    length = strlen(logBuf);
    obj->mWriteSize += length;

    if (obj->mWriteSize > obj->mConfigValue.log_size){
        int ret;
        obj->mWriteSize = 0;
        if (obj->mUsingLogFile == obj->mLogFile1){
            lseek(obj->mLogFile2, 0, SEEK_SET);
            ret = ftruncate(obj->mLogFile2, 0);
            if (ret == -1){
                printf("failed to truncate the log file 2 to 0\n");
            }
            // else{
            //     printf("To truncate the log file 2 OK!\n");
            // }
            obj->mUsingLogFile = obj->mLogFile2;
        }else if (obj->mUsingLogFile == obj->mLogFile2){
            lseek(obj->mLogFile1, 0, SEEK_SET);
            ret = ftruncate(obj->mLogFile1, 0);
            if (ret == -1){
                printf("failed to truncate the log file 1 to 0\n");
            }
            // else{
            //     printf("To truncate the log file 1 OK!\n");
            // }
            obj->mUsingLogFile = obj->mLogFile1;
        }
    }
    
    if (obj->mUsingLogFile > 0)
        write(obj->mUsingLogFile, logBuf, length);
}

void MagAgileLog::WriteToStdOut(int prio, char level, const char *module, const char *buffer, void *thiz){
    char logBuf[2048];
    sprintf(logBuf, "[%s/%c]\t%s\n", module, level, buffer);
    printf("%s", logBuf);
}

void MagAgileLog::WriteToSocket(int prio, char level, const char *module, const char *buffer, void *thiz){

}


void MagAgileLog::printLog(int prio, const char *module, const char *caller, int line, const char *printData){
    char logBuf[2048];
    ModuleConfig_t *pM = NULL;
    struct timeval tv;
    struct timezone tz;
    struct tm *tm;

    pthread_mutex_lock(&mLogMutex);
    // printf("enter printLog: prio = %d, module = %s, printData = %s\n", prio, module, printData);
    if (!mConfigValue.log_on){
        goto out;
    }

    if(mConfigValue.moduleNum > 0){
        pM = (ModuleConfig_t *)mpModuleHashT->getItem(mpModuleHashT, module);
        //print nothing
        if (NULL != pM){
            if ((!pM->debugOn) || (prio < pM->debugLevel))
                goto out;
        }else{
            if (prio < mConfigValue.config_debug_level)
                goto out;
        }
    }else{
        if (prio < mConfigValue.config_debug_level)
            goto out;
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
    if (mWriteToLogFunc){
        mWriteToLogFunc(prio, getPriorityLevel(prio), module, logBuf, static_cast<void *>(this));  
    }

out:
    pthread_mutex_unlock(&mLogMutex); 
}

void MagAgileLog::setDefaultValue(){
    mConfigValue.config_output.type  = OUTPUT_STDOUT;
    mWriteToLogFunc = WriteToStdOut;
    mConfigValue.config_debug_level  = DEBUG_LEVEL_ERROR;
    mConfigValue.config_timestamp_on = false;

    mConfigValue.moduleNum = 0;
    mConfigValue.pModules  = NULL;
    mConfigValue.log_on = true;
}

char MagAgileLog::getPriorityLevel(int prio){
    switch (prio){
        case DEBUG_LEVEL_VERBOSE:
            return 'V';
        case DEBUG_LEVEL_DEBUG:
            return 'D';
        case DEBUG_LEVEL_INFO:
            return 'I';
        case DEBUG_LEVEL_WARNING:
            return 'W';
        case DEBUG_LEVEL_ERROR:
            return 'E';
        case DEBUG_LEVEL_FATAL:
            return 'E';
        default:
            return 'V';
    }
}

}

