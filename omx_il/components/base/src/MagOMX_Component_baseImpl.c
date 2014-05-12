#include "MagOMX_Component_baseImpl.h"

#define LOOPER_NAME "MagOmxComponentBaseLooper"

AllocateClass(MagOmxComponentBase, MagOmxComponent);

static MagOmxComponentBase getBase(OMX_HANDLETYPE hComponent) {
    MagOmxComponentBase base;
    base = ooc_cast(hComponent, MagOmxComponentBase);
    return base;
}

static void MagOmxComponentBase_onMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponentBase base;
    OMX_U32 param;
    OMX_PTR cmd_data;

    if (!msg){
        AGILE_LOGE("msg is NULL!");
        return;
    }
    
    base = getBase(priv);

    if (!msg->findUInt32(msg, "param", &param)){
        AGILE_LOGE("failed to find the param!");
        return;
    }

    if (!msg->findPointer(msg, "cmd_data", &cmd_data)){
        AGILE_LOGE("failed to find the cmd_data!");
        return;
    }

    
    switch (msg->what(msg)) {
        case MagOmxComponentBase_CommandStateSet:
            thiz->onEmptyThisBuffer(msg);
            break;

        case MagOmxComponentBase_CommandFlush:
            thiz->onEmptyThisBuffer(msg);
            break;

        case MagOmxComponentBase_CommandPortDisable:
            thiz->onEmptyThisBuffer(msg);
            break;

        case MagOmxComponentBase_CommandPortEnable:
            thiz->onEmptyThisBuffer(msg);
            break;

        case MagOmxComponentBase_CommandMarkBuffer:
            thiz->onEmptyThisBuffer(msg);
            break;
            
        default:
            break;
    }
}

/********************************
 *Virtual member function implementation
 ********************************/
static OMX_ERRORTYPE virtual_MagOmxComponentBase_GetComponentVersion(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STRING pComponentName,
                OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                OMX_OUT OMX_UUIDTYPE* pComponentUUID){

    pSpecVersion->s.nVersionMajor = kVersionMajor;
    pSpecVersion->s.nVersionMinor = kVersionMinor;
    pSpecVersion->s.nRevision     = kVersionRevision;
    pSpecVersion->s.nStep         = kVersionStep;

    return MagOmxComponentBaseVirtual(getBase(hComponent))->MagOMX_GetComponentVersion(hComponent, 
                                                                                       pComponentName, 
                                                                                       pComponentVersion, 
                                                                                       pComponentUUID);
    
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_SendCommand(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_COMMANDTYPE Cmd,
                OMX_IN  OMX_U32 nParam1,
                OMX_IN  OMX_PTR pCmdData){

    MagOmxComponentBase base;
    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    
    if (Cmd == OMX_CommandStateSet){
        if ( !base->mCmdSetStateMsg ){
            base->mCmdSetStateMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandStateSet);  
        }
        base->mCmdSetStateMsg->setUInt32(base->mCmdSetStateMsg, "param", nParam1);
        base->mCmdSetStateMsg->setPointer(base->mCmdSetStateMsg, "cmd_data", pCmdData);

        base->mCmdSetStateMsg->postMessage(base->mCmdSetStateMsg, 0);
    }else if (Cmd == OMX_CommandFlush){
        if ( !base->mCmdFlushMsg ){
            base->mCmdFlushMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandFlush);  
        }
        base->mCmdFlushMsg->setUInt32(base->mCmdFlushMsg, "param", nParam1);
        base->mCmdFlushMsg->setPointer(base->mCmdFlushMsg, "cmd_data", pCmdData);
        
        base->mCmdFlushMsg->postMessage(base->mCmdFlushMsg, 0);
    }else if (Cmd == OMX_CommandPortDisable){
        if ( !base->mCmdPortDisableMsg ){
            base->mCmdPortDisableMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandPortDisable);  
        }
        base->mCmdPortDisableMsg->setUInt32(base->mCmdPortDisableMsg, "param", nParam1);
        base->mCmdPortDisableMsg->setPointer(base->mCmdPortDisableMsg, "cmd_data", pCmdData);

        base->mCmdPortDisableMsg->postMessage(base->mCmdPortDisableMsg, 0);
    }else if (Cmd == OMX_CommandPortEnable){
        if ( !base->mCmdPortEnableMsg ){
            base->mCmdPortEnableMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandPortEnable);  
        }
        base->mCmdPortEnableMsg->setUInt32(base->mCmdPortEnableMsg, "param", nParam1);
        base->mCmdPortEnableMsg->setPointer(base->mCmdPortEnableMsg, "cmd_data", pCmdData);

        base->mCmdPortEnableMsg->postMessage(base->mCmdPortEnableMsg, 0);
    }else if (Cmd == OMX_CommandMarkBuffer){
        if ( !base->mCmdMarkBufferMsg ){
            base->mCmdMarkBufferMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandMarkBuffer);  
        }
        base->mCmdMarkBufferMsg->setUInt32(base->mCmdMarkBufferMsg, "param", nParam1);
        base->mCmdMarkBufferMsg->setPointer(base->mCmdMarkBufferMsg, "cmd_data", pCmdData);

        base->mCmdMarkBufferMsg->postMessage(base->mCmdMarkBufferMsg, 0);
    }else{
        AGILE_LOGE("invalid command: 0x%x", Cmd);
        return OMX_ErrorBadParameter;
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_GetParameter(
                OMX_IN  MagOmxComponent hComponent, 
                OMX_IN  OMX_INDEXTYPE nParamIndex,  
                OMX_INOUT OMX_PTR pComponentParameterStructure){
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;
    
    MAG_ASSERT("It is pure virtual function and must be overrided");
    return ret;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_SetParameter(
                OMX_IN  MagOmxComponent hComponent, 
                OMX_IN  OMX_INDEXTYPE nIndex,
                OMX_IN  OMX_PTR pComponentParameterStructure){
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;
    
    MAG_ASSERT("It is pure virtual function and must be overrided");
    return ret;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_GetConfig(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_INOUT OMX_PTR pComponentConfigStructure){
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;
    
    MAG_ASSERT("It is pure virtual function and must be overrided");
    return ret;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_SetConfig(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_IN  OMX_PTR pComponentConfigStructure){
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;
    
    MAG_ASSERT("It is pure virtual function and must be overrided");
    return ret;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_GetExtensionIndex(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_STRING cParameterName,
                OMX_OUT OMX_INDEXTYPE* pIndexType){

}


static OMX_ERRORTYPE virtual_MagOmxComponentBase_GetState(
                OMX_IN  MagOmxComponent hComponent,
                OMX_OUT OMX_STATETYPE* pState){
    
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_ComponentTunnelRequest(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_U32 nPort,
                OMX_IN  OMX_HANDLETYPE hTunneledComp,
                OMX_IN  OMX_U32 nTunneledPort,
                OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup){

}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_UseBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer){
    MagOmxPort port = hComponent->getPort(hComponent, nPortIndex);
    if (NULL != port){
        return port->UseBuffer(port, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_AllocateBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes){
    MagOmxPort port = hComponent->getPort(hComponent, nPortIndex);
    if (NULL != port){
        return port->AllocateBuffer(port, ppBuffer, pAppPrivate, nSizeBytes);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_FreeBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxPort port = hComponent->getPort(hComponent, nPortIndex);
    if (NULL != port){
        return port->FreeBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_EmptyThisBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    /*TODO: add the port enable and component state judgement*/
    MagOmxPort port = hComponent->getPort(hComponent, pBuffer->nInputPortIndex);
    if ((NULL != port) && (port->mPortDef.eDir == OMX_DirInput)){
        return port->sendBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_FillThisBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    /*TODO: add the port enable and component state judgement*/
    MagOmxPort port = hComponent->getPort(hComponent, pBuffer->nOutputPortIndex);
    if ((NULL != port) && (port->mPortDef.eDir == OMX_DirOutput)){
        return port->sendBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_SetCallbacks(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                OMX_IN  OMX_PTR pAppData){
    hComponent->mCallbacks = pCallbacks;
    hComponent->mAppData   = pAppData;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_ComponentDeInit(
                OMX_IN  MagOmxComponent hComponent){

}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_UseEGLImage(
                OMX_IN  MagOmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN void* eglImage){

}

static OMX_ERRORTYPE virtual_MagOmxComponentBase_ComponentRoleEnum(
                OMX_IN  MagOmxComponent hComponent,
         		OMX_OUT OMX_U8 *cRole,
         		OMX_IN OMX_U32 nIndex){

}

/*Member functions*/

static MagMessageHandle MagOmxComponentBase_createMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentBase hComponent = NULL;
    
    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentBase);
        
    hComponent->getLooper(handle);
    
    MagMessageHandle msg = createMagMessage(hComponent->mLooper, what, hComponent->mMsgHandler->id(hComponent->mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxComponentBase_getLooper(OMX_HANDLETYPE handle){
    MagOmxComponentBase hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentBase);
    
    if ((NULL != hComponent->mLooper) && (NULL != hComponent->mMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mLooper){
        hComponent->mLooper = createLooper(LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mLooper);
    }
    
    if (NULL != hComponent->mLooper){
        if (NULL == hComponent->mMsgHandler){
            hComponent->mMsgHandler = createHandler(hComponent->mLooper, MagOmxComponentBase_onMessageReceived, handle);

            if (NULL != hComponent->mMsgHandler){
                hComponent->mLooper->registerHandler(hComponent->mLooper, hComponent->mMsgHandler);
                hComponent->mLooper->start(hComponent->mLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}


/*Class Constructor/Destructor*/
static void MagOmxComponentBase_initialize(Class this){
    /*Override the base component pure virtual functions*/
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetComponentVersion    = virtual_MagOmxComponentBase_GetComponentVersion;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SendCommand            = virtual_MagOmxComponentBase_SendCommand;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetParameter           = virtual_MagOmxComponentBase_GetParameter;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SetParameter           = virtual_MagOmxComponentBase_SetParameter;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetConfig              = virtual_MagOmxComponentBase_GetConfig;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SetConfig              = virtual_MagOmxComponentBase_SetConfig;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetExtensionIndex      = virtual_MagOmxComponentBase_GetExtensionIndex;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetState               = virtual_MagOmxComponentBase_GetState;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.ComponentTunnelRequest = virtual_MagOmxComponentBase_ComponentTunnelRequest;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.UseBuffer              = virtual_MagOmxComponentBase_UseBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.AllocateBuffer         = virtual_MagOmxComponentBase_AllocateBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.FreeBuffer             = virtual_MagOmxComponentBase_FreeBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.EmptyThisBuffer        = virtual_MagOmxComponentBase_EmptyThisBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.FillThisBuffer         = virtual_MagOmxComponentBase_FillThisBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SetCallbacks           = virtual_MagOmxComponentBase_SetCallbacks;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.ComponentDeInit        = virtual_MagOmxComponentBase_ComponentDeInit;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.UseEGLImage            = virtual_MagOmxComponentBase_UseEGLImage;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.ComponentRoleEnum      = virtual_MagOmxComponentBase_ComponentRoleEnum;

    /*pure virtual functions to be overrided by sub-components*/
    MagOmxComponentBaseVtableInstance.MagOMX_GetComponentVersion    = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_SendCommand            = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_GetParameter           = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_SetParameter           = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_GetConfig              = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_SetConfig              = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_GetExtensionIndex      = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_GetState               = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_ComponentTunnelRequest = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_UseBuffer              = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_AllocateBuffer         = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_FreeBuffer             = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_EmptyThisBuffer        = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_FillThisBuffer         = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_SetCallbacks           = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_ComponentDeInit        = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_UseEGLImage            = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_ComponentRoleEnum      = NULL;
}

static void MagOmxComponentBase_constructor(MagOmxComponentBase thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(MagOmxComponentBase));
    chain_constructor(MagOmxComponentBase, thiz, params);

    thiz->createMessage     = MagOmxComponentBase_createMessage;
    thiz->getLooper         = MagOmxComponentBase_getLooper;

    thiz->mLooper     = NULL;
    thiz->mMsgHandler = NULL;

    thiz->mCmdSetStateMsg    = NULL;
    thiz->mCmdPortDisableMsg = NULL;
    thiz->mCmdPortEnableMsg  = NULL;
    thiz->mCmdFlushMsg       = NULL;
    thiz->mCmdMarkBufferMsg  = NULL;
}

static void MagOmxComponentBase_destructor(MagOmxComponentBase thiz, MagOmxComponentBaseVtable vtab){
    AGILE_LOGV("Enter!");
}


