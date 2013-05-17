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

inline OMXComponentSetHeader(OMX_PTR header, OMX_U32 size){
  OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
  *((OMX_U32*)header) = size;

  ver->s.nVersionMajor = OMXVERSIONMAJOR;
  ver->s.nVersionMinor = OMXVERSIONMINOR;
  ver->s.nRevision     = OMXVERSIONREVISION;
  ver->s.nStep         = OMXVERSIONSTEP;
}


#endif