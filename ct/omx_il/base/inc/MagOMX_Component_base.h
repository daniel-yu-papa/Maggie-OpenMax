#ifndef __OMX_COMPONENT_BASE_H__
#define __OMX_COMPONENT_BASE_H__

#include <pthread.h>

#include "OMX_ClassMagic.h"

#define OMXVERSIONMAJOR    1
#define OMXVERSIONMINOR    1
#define OMXVERSIONREVISION 2
#define OMXVERSIONSTEP     0

typedef struct omx_sub_component_callbacks_obj{
    OMX_ERRORTYPE (*getParameter)(OMX_IN OMX_INDEXTYPE nParamIndex,
                        OMX_INOUT OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*setParameter)(OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_IN OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*setConfig)(OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_INOUT OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*getConfig)(OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_INOUT OMX_PTR pComponentConfigStructure);
}OMXSubComponentCallbacks_t;
/*
* Child component private data structure
*  |-----------------------------------------|
*  | OMXComponentPrivateBase_t *root                  |
*  | OMXComponentPrivateChild_t *child                  |
*  | OMXComponentPrivateChildChild_t *child-child    |
*  | others.....                                                    |
*  |------------------------------------------|
*/

CLASS(OMXComponentPrivateBase_t)
#define OMXComponentPrivateBase_t_FIELDS \
    OMXCompInternalState_t state;  /*the component internal state, which is different from the OMX_STATETYPE by adding transition states*/ \
    pthread_mutex_t   stateTransMutex; /*the mutex used to protect the component state operations*/\    
    OMX_CALLBACKTYPE  callbacks; /* active component callbacks. */ \
    MagMsgChannelHandle cmdChannel; /*command message dispatching logic*/ \
    OMXSubComponentCallbacks_t *baseComp_callbacks; /*the registered callbacks by the child component*/ \
    /* used for Component state transition from Loaded to Idle */ \
    MagEventHandle Event_OMX_AllocateBufferDone;    /* the event is triggered after the OMX_AllocateBuffer() called from IL client is done */  \
    MagEventHandle Event_OMX_UseBufferDone;         /* the event is triggered after the OMX_UserBuffer() called from IL clien is done */  \
    MagEventGroupHandle EventGroup_PortBufferAllocated; /* either Event_OMX_AllocateBufferDone or Event_OMX_UseBufferDone is triggered */ \
    \
    List_t portListHead; /* link all added ports in the list */ 
ENDCLASS(OMXComponentPrivateBase_t)


typedef struct{
    OMX_HANDLETYPE  hComp;
    OMX_COMMANDTYPE Cmd;
    OMX_U32         nParam;
    OMX_PTR         pCmdData;
}OMXCommandMsg_t;

OMX_ERRORTYPE OMXComponent_Base_SetCallbacks(OMX_HANDLETYPE hComp, OMXSubComponentCallbacks_t *cb);

inline void OMXComponentSetHeader(OMX_PTR header, OMX_U32 size){
  OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
  *((OMX_U32*)header) = size;

  ver->s.nVersionMajor = OMXVERSIONMAJOR;
  ver->s.nVersionMinor = OMXVERSIONMINOR;
  ver->s.nRevision     = OMXVERSIONREVISION;
  ver->s.nStep         = OMXVERSIONSTEP;
}

inline OMXComponentPort_base_t *getPortFromBufferHeader(OMX_IN OMX_BUFFERHEADERTYPE* pBuffer, OMX_OUT OMX_DIRTYPE *portDir){
    if(pBuffer->pInputPortPrivate){
        *portDir = OMX_DirInput;
        return (OMXComponentPort_base_t *)pBuffer->pInputPortPrivate;
    }else if(pBuffer->pOutputPortPrivate){
        *portDir = OMX_DirOutput;
        return (OMXComponentPort_base_t *)pBuffer->pOutputPortPrivate;
    }else{
        *portDir = OMX_DirMax;
        return NULL;
    }
}


#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

typedef struct{
    OMX_COMMANDTYPE Cmd;
    OMX_U32         nParam;
    OMX_PTR         pCmdData;
}MagOmxCmdMsg_t;


DeclareClass(OmxComponent, Base);

Virtuals(OmxComponent, Base) 
    OMX_ERRORTYPE (*GetComponentVersion)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_OUT OMX_STRING pComponentName,
                    OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                    OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID);

    OMX_ERRORTYPE (*SendCommand)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_COMMANDTYPE Cmd,
                    OMX_IN  OMX_U32 nParam1,
                    OMX_IN  OMX_PTR pCmdData);

    OMX_ERRORTYPE (*GetParameter)(
                    OMX_IN  OmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*SetParameter)(
                    OMX_IN  OmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*GetConfig)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_INOUT OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*SetConfig)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_IN  OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*GetExtensionIndex)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_STRING cParameterName,
                    OMX_OUT OMX_INDEXTYPE* pIndexType);

    
    OMX_ERRORTYPE (*GetState)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_OUT OMX_STATETYPE* pState);

    OMX_ERRORTYPE (*ComponentTunnelRequest)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_U32 nPort,
                    OMX_IN  OMX_HANDLETYPE hTunneledComp,
                    OMX_IN  OMX_U32 nTunneledPort,
                    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup); 

    OMX_ERRORTYPE (*UseBuffer)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes,
                    OMX_IN OMX_U8* pBuffer);

    OMX_ERRORTYPE (*AllocateBuffer)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes);

    OMX_ERRORTYPE (*FreeBuffer)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_U32 nPortIndex,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*EmptyThisBuffer)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*FillThisBuffer)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*SetCallbacks)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                    OMX_IN  OMX_PTR pAppData);

    OMX_ERRORTYPE (*ComponentDeInit)(
                    OMX_IN  OmxComponent hComponent,);

    OMX_ERRORTYPE (*UseEGLImage)(
                    OMX_IN  OmxComponent hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN void* eglImage);

    OMX_ERRORTYPE (*ComponentRoleEnum)(
                    OMX_IN  OmxComponent hComponent,
             		OMX_OUT OMX_U8 *cRole,
             		OMX_IN OMX_U32 nIndex);

    /*pure virtual functions and must be overrided*/
    OMX_ERRORTYPE (*Start)(OMX_IN  OmxComponent hComponent);

    OMX_ERRORTYPE (*Stop)(OMX_IN  OmxComponent hComponent);

    OMX_ERRORTYPE (*Resume)(OMX_IN  OmxComponent hComponent);

    OMX_ERRORTYPE (*Pause)(OMX_IN  OmxComponent hComponent);

    OMX_ERRORTYPE (*Prepare)(OMX_IN  OmxComponent hComponent);

    OMX_ERRORTYPE (*Preroll)(OMX_IN  OmxComponent hComponent);

    OMX_ERRORTYPE (*AllResourcesReady)(OMX_IN  OmxComponent hComponent);

    OMX_ERRORTYPE (*FreeResources)(OMX_IN  OmxComponent hComponent);

EndOfVirtuals;

ClassMembers(OmxComponent, Base, \
    OMX_COMPONENTTYPE *(*Create)(OmxComponent pBase); \
    void               (*setHandle)(OmxComponent pBase, OMX_COMPONENTTYPE *handle); \
    OMX_ERRORTYPE      (*postEvent)(OmxComponent pBase, OMX_EVENTTYPE event, OMX_U32 param1, OMX_U32 param2, OMX_PTR pEventData); \
    OMX_ERRORTYPE      (*postFillBufferDone)(OmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer);  \
    OMX_ERRORTYPE      (*postEmptyBufferDone)(OmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer); \
    OMX_ERRORTYPE      (*setState)(OmxComponent pBase, OMX_STATETYPE targetState); \
)
    OMX_COMPONENTTYPE      *mComponentHandle;  /*OMX spec defined Component handle*/
    OMX_CALLBACKTYPE       *mCallbacks;        /*the registered callbacks from OMX IL client*/
    OMX_PTR                mAppData;           /*the OMX IL client specific DATA for component callbacks using */

    MagMsgChannelHandle    mCmdEvtChannel;     /*command message dispatching logic*/

    OMX_STATETYPE          mState;             /*current component state*/

EndOfClassMembers;

#endif
