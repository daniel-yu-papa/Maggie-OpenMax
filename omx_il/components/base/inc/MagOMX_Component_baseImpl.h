#ifndef __MAG_OMX_COMPONENT_BASE_H__
#define __MAG_OMX_COMPONENT_BASE_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

enum{
    MagOmxComponentBase_CommandStateSet,
    MagOmxComponentBase_CommandFlush,
    MagOmxComponentBase_CommandPortDisable,
    MagOmxComponentBase_CommandPortEnable,
    MagOmxComponentBase_CommandMarkBuffer,
};


DeclareClass(MagOmxComponentBase, MagOmxComponent);

Virtuals(MagOmxComponentBase, MagOmxComponent) 
    OMX_ERRORTYPE (*MagOMX_GetComponentVersion)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_STRING pComponentName,
                    OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID);

    OMX_ERRORTYPE (*MagOMX_SendCommand)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_COMMANDTYPE Cmd,
                    OMX_IN  OMX_U32 nParam1,
                    OMX_IN  OMX_PTR pCmdData);
    
    OMX_ERRORTYPE (*MagOMX_GetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*MagOMX_SetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*MagOMX_GetConfig)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_INOUT OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*MagOMX_SetConfig)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_IN  OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*MagOMX_GetExtensionIndex)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_STRING cParameterName,
                    OMX_OUT OMX_INDEXTYPE* pIndexType);

    
    OMX_ERRORTYPE (*MagOMX_GetState)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_STATETYPE* pState);

    OMX_ERRORTYPE (*MagOMX_ComponentTunnelRequest)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_U32 nPort,
                    OMX_IN  OMX_HANDLETYPE hTunneledComp,
                    OMX_IN  OMX_U32 nTunneledPort,
                    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup); 

    OMX_ERRORTYPE (*MagOMX_UseBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes,
                    OMX_IN OMX_U8* pBuffer);

    OMX_ERRORTYPE (*MagOMX_AllocateBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes);

    OMX_ERRORTYPE (*MagOMX_FreeBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_U32 nPortIndex,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*MagOMX_EmptyThisBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*MagOMX_FillThisBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*MagOMX_SetCallbacks)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                    OMX_IN  OMX_PTR pAppData);

    OMX_ERRORTYPE (*MagOMX_ComponentDeInit)(
                    OMX_IN  OMX_HANDLETYPE hComponent,);

    OMX_ERRORTYPE (*MagOMX_UseEGLImage)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN void* eglImage);

    OMX_ERRORTYPE (*MagOMX_ComponentRoleEnum)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
             		OMX_OUT OMX_U8 *cRole,
             		OMX_IN OMX_U32 nIndex);

EndOfVirtuals;

ClassMembers(MagOmxComponentBase, MagOmxComponent, \
    MagMessageHandle (*createMessage)(OMX_HANDLETYPE handle, OMX_U32 what); \
    _status_t (*getLooper)(OMX_HANDLETYPE handle); \  
)
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    MagMessageHandle mCmdSetStateMsg;
    MagMessageHandle mCmdPortDisableMsg;
    MagMessageHandle mCmdPortEnableMsg;
    MagMessageHandle mCmdFlushMsg;
    MagMessageHandle mCmdMarkBufferMsg;
    
EndOfClassMembers;

#endif
