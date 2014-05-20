#ifndef __MAG_OMX_COMPONENT_BASE_H__
#define __MAG_OMX_COMPONENT_BASE_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

typedef enum{
    MagOMX_Component_Video = 0,
    MagOMX_Component_Audio,
    MagOMX_Component_Subtitle,
    MagOMX_Component_Image,
    MagOMX_Component_Other,
    MagOMX_Component_Max,
}MagOMX_Component_Type_t;

enum{
    MagOmxComponentBase_CommandStateSet,
    MagOmxComponentBase_CommandFlush,
    MagOmxComponentBase_CommandPortDisable,
    MagOmxComponentBase_CommandPortEnable,
    MagOmxComponentBase_CommandMarkBuffer,
};

typedef OMX_ERRORTYPE (*doStateTransition)(OMX_IN OMX_HANDLETYPE hComponent);

inline OMX_U32 toIndex(OMX_STATETYPE state){
    return ((OMX_U32)state - 1);
}


DeclareClass(MagOmxComponentBase, MagOmxComponent);

Virtuals(MagOmxComponentBase, MagOmxComponent) 
    OMX_COMPONENTTYPE *(*Create)(
                    OMX_IN OMX_HANDLETYPE hComponent, 
                    OMX_IN OMX_PTR pAppData,
                    OMX_IN OMX_CALLBACKTYPE *pCallbacks);

    OMX_ERRORTYPE (*sendEvents)(
                        OMX_IN MagOmxComponentBase hComponent,
                        OMX_IN OMX_EVENTTYPE eEvent,
                        OMX_IN OMX_U32 nData1,
                        OMX_IN OMX_U32 nData2,
                        OMX_IN OMX_PTR pEventData);

    OMX_ERRORTYPE (*sendEmptyBufferDoneEvent)(
                        OMX_IN MagOmxComponentBase hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*sendFillBufferDoneEvent)(
                        OMX_IN MagOmxComponentBase hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
    
    /*Pure virtual functions. Must be overrided by sub-components*/
    OMX_ERRORTYPE (*MagOMX_GetComponentUUID)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID);

    OMX_ERRORTYPE (*MagOMX_Prepare)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*MagOMX_Preroll)(
                    OMX_IN  OMX_HANDLETYPE hComponent);
    
    OMX_ERRORTYPE (*MagOMX_Start)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*MagOMX_Stop)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*MagOMX_Pause)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*MagOMX_Resume)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*MagOMX_FreeResources)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    MagOMX_Component_Type_t (*MagOMX_getType)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*MagOMX_GetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);
                    
    OMX_ERRORTYPE (*MagOMX_SetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);
    
EndOfVirtuals;

ClassMembers(MagOmxComponentBase, MagOmxComponent, \
    MagMessageHandle (*createMessage)(OMX_HANDLETYPE handle, OMX_U32 what);  \
    _status_t        (*getLooper)(OMX_HANDLETYPE handle);                           \  

    OMX_ERRORTYPE    (*setState)(OMX_HANDLETYPE handle, OMX_STATETYPE state);   \
    OMX_ERRORTYPE    (*flushPort)(OMX_HANDLETYPE handle, OMX_U32 port_index);   \
    OMX_ERRORTYPE    (*enablePort)(OMX_HANDLETYPE handle, OMX_U32 port_index);  \
    OMX_ERRORTYPE    (*disablePort)(OMX_HANDLETYPE handle, OMX_U32 port_index); \
    void             (*addPort)(MagOmxComponentBase hComponent, OMX_U32 portIndex, OMX_HANDLETYPE hPort);       \
    MagOmxPort       (*getPort)(MagOmxComponentBase hComponent, OMX_U32 portIndex); \
)
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    MagMessageHandle mCmdSetStateMsg;
    MagMessageHandle mCmdPortDisableMsg;
    MagMessageHandle mCmdPortEnableMsg;
    MagMessageHandle mCmdFlushMsg;
    MagMessageHandle mCmdMarkBufferMsg;

    OMX_STATETYPE    mState;
    doStateTransition mStateTransitTable[5][5]; /*[current state][target state]*/
    
    MagMiniDBHandle  mParametersDB;
    RBTreeNodeHandle mPortsTable;
    OMX_U32          mPortsNumber;
    
    MagMutexHandle   mhParamMutex;

    OMX_CALLBACKTYPE *mpCallbacks;
    OMX_PTR          mpAppData;
EndOfClassMembers;


#endif
