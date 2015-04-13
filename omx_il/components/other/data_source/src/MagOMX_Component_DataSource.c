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

#include "MagOMX_Component_DataSource.h"
#include "MagOMX_Port_buffer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompDS"

#define DATA_SOURCE_LOOPER_NAME        "CompDataSrcLooper"

AllocateClass(MagOmxComponentDataSource, MagOmxComponentImpl);

static void onReadDataMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent       root;
    MagOmxComponentImpl   base;
    MagOmxComponentDataSource thiz;
    OMX_ERRORTYPE         ret;         
    MagOmxPort            port;
    MagOmxPortBuffer      portBuf;
    OMX_HANDLETYPE        hPort;
    OMX_U32               cmd;
    RBTreeNodeHandle      n;

    root = ooc_cast(priv, MagOmxComponent);
    base = ooc_cast(priv, MagOmxComponentImpl);
    thiz = ooc_cast(priv, MagOmxComponentDataSource);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        break;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentDataSource_ReadDataMsg:
        {
            OMX_BUFFERHEADERTYPE *destbufHeader;

            destbufHeader = MagOmxPortVirtual(port)->GetOutputBuffer(port);

            if (MagOmxComponentDataSourceVirtual(thiz)->MagOMX_DataSource_Read){
                MagOmxComponentDataSourceVirtual(thiz)->MagOMX_DataSource_Read(thiz, destbufHeader);
                MagOmxPortVirtual(port)->sendOutputBuffer(port, destbufHeader);
            }else{
                COMP_LOGE(root, "pure virtual function MagOMX_DataSource_Read() should be overrided");
                return;
            }

            msg->postMessage(msg, 0);
            break;
        }

        default:
            COMP_LOGE(root, "wrong message %d received!");
            break;
    }
}

static MagOMX_Component_Type_t virtual_MagOmxComponentDataSource_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Other;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
	return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){
	MagOmxComponentDataSource thiz;
    MagOmxComponent      root;
    OMX_ERRORTYPE        ret = OMX_ErrorNone;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentDataSource);
    root = ooc_cast(hComponent, MagOmxComponent);

    switch (nIndex){
    	case OMX_IndexParamExtDataSourceSetting:
        {
	    	OMX_DATA_SOURCE_SETTING *param;

            param = (OMX_DATA_SOURCE_SETTING *)pComponentParameterStructure;
            thiz->mUrl = mag_strdup(param->url);
            COMP_LOGD(root, "url: %s", thiz->mUrl);
        }
    		break;

        case OMX_IndexConfigExtSeekData:
        {
            OMX_CONFIG_SEEKDATABUFFER *seekConfig;

            seekConfig = (OMX_CONFIG_SEEKDATABUFFER *)pComponentParameterStructure;

            if (MagOmxComponentDataSourceVirtual(thiz)->MagOMX_DataSource_Seek){
                seekConfig->sCurPos = MagOmxComponentDataSourceVirtual(thiz)->MagOMX_DataSource_Seek(thiz, seekConfig->sOffset, seekConfig->sWhence);
            }else{
                COMP_LOGE(root, "pure virtual function MagOMX_DataSource_Seek() should be overrided");
                return OMX_ErrorNotImplemented;
            }
        }
            break;

    	default:
    		break;
    }

	return ret;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentDataSource_TearDownTunnel(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_U32 portIdx){
    MagOmxComponentDataSource  clkComp;
    MagOmxComponentImpl   clkCompImpl;
    MagOmxComponent       clkCompRoot; 
    OMX_HANDLETYPE        clkPort;

    AGILE_LOGV("enter!");
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_Prepare(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentDataSource thiz;
    MagOmxComponent       root;
    OMX_ERRORTYPE         ret = OMX_ErrorNone;
    OMX_U32               bp_size_time;
    OMX_U32               buffer_pool_size;

    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentDataSource);
    root = ooc_cast(hComponent, MagOmxComponent);

    if (MagOmxComponentDataSourceVirtual(thiz)->MagOMX_DataSource_Init){
        MagOmxComponentDataSourceVirtual(thiz)->MagOMX_DataSource_Init(thiz, thiz->mUrl);
    }else{
        COMP_LOGE(root, "pure virtual function MagOMX_DataSource_Init() should be overrided");
        return OMX_ErrorNotImplemented;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_Preroll(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_Stop(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_Start(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentDataSource thiz;

    thiz = ooc_cast(hComponent, MagOmxComponentDataSource);
    if (thiz->mReadDataMsg == NULL){
        thiz->mReadDataMsg = thiz->createReadDataMessage(thiz, MagOmxComponentDataSource_ReadDataMsg);
    }

    thiz->mReadDataMsg->postMessage(thiz->mReadDataMsg, 0);
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_Pause(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_Resume(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_Flush(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDataSource_ProceedBuffer(
                                        OMX_IN  OMX_HANDLETYPE hComponent, 
                                        OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                                        OMX_IN  OMX_HANDLETYPE hDestPort){
   
    return OMX_ErrorNone;
}

static MagMessageHandle MagOmxComponentDataSource_createReadDataMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentDataSource hComponent = NULL;
    MagMessageHandle msg;

    if (NULL == handle){
        return NULL;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentDataSource);
    hComponent->getReadDataLooper(handle);
    
    msg = createMagMessage(hComponent->mReadDataLooper, what, hComponent->mReadDataMsgHandler->id(hComponent->mReadDataMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxComponentDataSource_getReadDataLooper(OMX_HANDLETYPE handle){
    MagOmxComponentDataSource hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentDataSource);
    
    if ((NULL != hComponent->mReadDataLooper) && (NULL != hComponent->mReadDataMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mReadDataLooper){
        hComponent->mReadDataLooper = createLooper(DATA_SOURCE_LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mReadDataLooper);
    }
    
    if (NULL != hComponent->mReadDataLooper){
        if (NULL == hComponent->mReadDataMsgHandler){
            hComponent->mReadDataMsgHandler = createHandler(hComponent->mReadDataLooper, onReadDataMessageReceived, handle);

            if (NULL != hComponent->mReadDataMsgHandler){
                hComponent->mReadDataLooper->registerHandler(hComponent->mReadDataLooper, hComponent->mReadDataMsgHandler);
                hComponent->mReadDataLooper->start(hComponent->mReadDataLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", DATA_SOURCE_LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

/*Class Constructor/Destructor*/
static void MagOmxComponentDataSource_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_getType           = virtual_MagOmxComponentDataSource_getType;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter      = virtual_MagOmxComponentDataSource_GetParameter;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter      = virtual_MagOmxComponentDataSource_SetParameter;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_TearDownTunnel    = virtual_MagOmxComponentDataSource_TearDownTunnel;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_Prepare           = virtual_MagOmxComponentDataSource_Prepare;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_Preroll           = virtual_MagOmxComponentDataSource_Preroll;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_Stop              = virtual_MagOmxComponentDataSource_Stop;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_Start             = virtual_MagOmxComponentDataSource_Start;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_Pause             = virtual_MagOmxComponentDataSource_Pause;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_Resume            = virtual_MagOmxComponentDataSource_Resume;
    MagOmxComponentDataSourceVtableInstance.MagOmxComponentImpl.MagOMX_Flush             = virtual_MagOmxComponentDataSource_Flush;
}

static void MagOmxComponentDataSource_constructor(MagOmxComponentDataSource thiz, const void *params){
    OMX_U32 i;

    AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentDataSource));
    chain_constructor(MagOmxComponentDataSource, thiz, params);

    thiz->mReadDataLooper        = NULL;
    thiz->mReadDataMsgHandler    = NULL;
    thiz->mReadDataMsg           = NULL;
    
    thiz->getReadDataLooper      = MagOmxComponentDataSource_getReadDataLooper;
    thiz->createReadDataMessage  = MagOmxComponentDataSource_createReadDataMessage;
}

static void MagOmxComponentDataSource_destructor(MagOmxComponentDataSource thiz, MagOmxComponentDataSourceVtable vtab){
	AGILE_LOGV("Enter!");

    destroyMagMessage(&thiz->mReadDataMsg);
    destroyHandler(&thiz->mReadDataMsgHandler);
    destroyLooper(&thiz->mReadDataLooper);
}