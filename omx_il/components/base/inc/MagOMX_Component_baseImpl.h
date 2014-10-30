#ifndef __MAG_OMX_COMPONENT_BASE_IMPL_H__
#define __MAG_OMX_COMPONENT_BASE_IMPL_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Port_base.h"

typedef enum{
    MagOMX_Component_Video = 0,
    MagOMX_Component_Audio,
    MagOMX_Component_Subtitle,
    MagOMX_Component_Image,
    MagOMX_Component_Other,
    MagOMX_Component_Max
}MagOMX_Component_Type_t;

enum{
    MagOmxComponentImpl_CommandStateSetMsg,
    MagOmxComponentImpl_CommandFlushMsg,
    MagOmxComponentImpl_CommandPortDisableMsg,
    MagOmxComponentImpl_CommandPortEnableMsg,
    MagOmxComponentImpl_CommandMarkBufferMsg,

    MagOmxComponentImpl_PortDataFlowMsg = 10
    /*MagOmxComponentImpl_PortDataFlowMsg + port index: the data flow message for each port*/
};

/*the notification from the attached port*/
typedef enum{
    MagOMX_Component_Notify_StartTime = 0,
    MagOMX_Component_Notify_MediaTimeRequest,
    MagOMX_Component_Notify_ReferenceTimeUpdate
}MagOMX_Component_Notify_Type_t;

typedef OMX_ERRORTYPE (*doStateTransition)(OMX_IN OMX_HANDLETYPE hComponent);

static inline OMX_U32 toIndex(OMX_STATETYPE state){
    return ((OMX_U32)state - 1);
}


DeclareClass(MagOmxComponentImpl, MagOmxComponent);

Virtuals(MagOmxComponentImpl, MagOmxComponent) 
    OMX_COMPONENTTYPE *(*Create)(
                    OMX_IN OMX_HANDLETYPE hComponent, 
                    OMX_IN OMX_PTR pAppData,
                    OMX_IN OMX_CALLBACKTYPE *pCallbacks);
    
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

    OMX_ERRORTYPE (*MagOMX_Deinit)(
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*MagOMX_Reset)(
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

    OMX_ERRORTYPE (*MagOMX_ComponentRoleEnum)(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex);

    /*proceed the input data buffer and output the cooked data*/
    OMX_ERRORTYPE (*MagOMX_ProceedBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort);  

    /*Do av sync action*/
    OMX_ERRORTYPE  (*MagOMX_DoAVSync)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_TIME_MEDIATIMETYPE *mediaTime);

    /*Set av sync component status changing OR Set av sync scale changing*/
    OMX_ERRORTYPE  (*MagOMX_SetAVSyncStatus)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_TIME_UPDATETYPE type,
                    OMX_IN  OMX_S32 status);

    /*return OMX_ErrorUnsupportedSetting: the component doesn't support to be set as reference clock provider*/
    OMX_ERRORTYPE  (*MagOMX_SetRefClock)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *rc);

    /*the actions to be done by sub-component while adding the port*/
    OMX_ERRORTYPE  (*MagOMX_DoAddPortAction)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_U32 portIndex, 
                    OMX_IN  OMX_HANDLETYPE hPort);
    
    /*handle the notification from attached port*/
    OMX_ERRORTYPE  (*MagOMX_Notify)(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_IN MagOMX_Component_Notify_Type_t notifyIndex,
                    OMX_IN OMX_PTR pNotifyData);

    /*get the rendering delay value in microseconds*/
    OMX_ERRORTYPE  (*MagOMX_GetRenderDelay)(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U32 *pRenderDelay);

EndOfVirtuals;

ClassMembers(MagOmxComponentImpl, MagOmxComponent, \
    MagMessageHandle (*createMessage)(OMX_HANDLETYPE handle, OMX_U32 what);     \
    _status_t        (*getLooper)(OMX_HANDLETYPE handle);                       \

    MagMessageHandle (*createBufferMessage)(OMX_HANDLETYPE handle, OMX_U32 what);     \
    _status_t        (*getBufferLooper)(OMX_HANDLETYPE handle); 
    
    OMX_ERRORTYPE    (*setState)(OMX_HANDLETYPE handle, OMX_STATETYPE state);   \
    OMX_ERRORTYPE    (*flushPort)(OMX_HANDLETYPE handle, OMX_U32 port_index);   \
    OMX_ERRORTYPE    (*enablePort)(OMX_HANDLETYPE handle, OMX_U32 port_index);  \
    OMX_ERRORTYPE    (*disablePort)(OMX_HANDLETYPE handle, OMX_U32 port_index); \
    void             (*addPort)(MagOmxComponentImpl hComponent, OMX_U32 portIndex, OMX_HANDLETYPE hPort);  \
    OMX_HANDLETYPE   (*getPort)(MagOmxComponentImpl hComponent, OMX_U32 portIndex); \
    
    OMX_ERRORTYPE    (*sendEvents)(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_EVENTTYPE eEvent,
                        OMX_IN OMX_U32 nData1,
                        OMX_IN OMX_U32 nData2,
                        OMX_IN OMX_PTR pEventData);  \
    OMX_ERRORTYPE    (*sendEmptyBufferDoneEvent)(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer); \
    OMX_ERRORTYPE    (*sendFillBufferDoneEvent)(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer); \
    OMX_ERRORTYPE    (*setupPortDataFlow)(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN OMX_HANDLETYPE hInPort,
                        OMX_IN OMX_HANDLETYPE hOutPort); \

    /*the port notify the attached component*/
    OMX_ERRORTYPE    (*notify)(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN MagOMX_Component_Notify_Type_t notifyIndex,
                        OMX_IN OMX_PTR pNotifyData); \
)
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    MagLooperHandle  mBufferLooper;
    MagHandlerHandle mBufferMsgHandler;

    MagMessageHandle mCmdSetStateMsg;
    MagMessageHandle mCmdPortDisableMsg;
    MagMessageHandle mCmdPortEnableMsg;
    MagMessageHandle mCmdFlushMsg;
    MagMessageHandle mCmdMarkBufferMsg;

    OMX_STATETYPE    mState;
    OMX_EXTSTATETRANSTYPE mTransitionState;
    doStateTransition mStateTransitTable[5][5]; /*[current state][target state]*/
    
    MagMiniDBHandle  mParametersDB;
    RBTreeNodeHandle mPortTreeRoot;
    OMX_U32          mStartPortNumber;
    OMX_U32          mPorts;
    
    /*synchronize the state setting and OMX API calling*/
    MagMutexHandle   mhMutex;

    OMX_CALLBACKTYPE *mpCallbacks;
    OMX_PTR          mpAppData;

    MagMessageHandle *mPortDataMsgList;
    OMX_U32          mFlushingPorts;
EndOfClassMembers;


#endif
