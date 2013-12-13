#ifndef __MAG_OMX_COMPONENT_BASE_H__
#define __MAG_OMX_COMPONENT_BASE_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

typedef struct{
    OMX_COMMANDTYPE Cmd;
    OMX_U32         nParam;
    OMX_PTR         pCmdData;
}MagOmxCmdMsg_t;


DeclareClass(MagOmxComponent, Base);

Virtuals(MagOmxComponent, Base) 
    OMX_ERRORTYPE (*GetComponentVersion)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_OUT OMX_STRING pComponentName,
                    OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                    OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID);

    OMX_ERRORTYPE (*SendCommand)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_COMMANDTYPE Cmd,
                    OMX_IN  OMX_U32 nParam1,
                    OMX_IN  OMX_PTR pCmdData);
    
    /*GetParameter(), SetParameter(), GetConfig(), SetConfig() are pure virtual functions and must be overrided by sub-class*/
    OMX_ERRORTYPE (*GetParameter)(
                    OMX_IN  MagOmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*SetParameter)(
                    OMX_IN  MagOmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*GetConfig)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_INOUT OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*SetConfig)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_IN  OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*GetExtensionIndex)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_STRING cParameterName,
                    OMX_OUT OMX_INDEXTYPE* pIndexType);

    
    OMX_ERRORTYPE (*GetState)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_OUT OMX_STATETYPE* pState);

    OMX_ERRORTYPE (*ComponentTunnelRequest)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_U32 nPort,
                    OMX_IN  OMX_HANDLETYPE hTunneledComp,
                    OMX_IN  OMX_U32 nTunneledPort,
                    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup); 

    OMX_ERRORTYPE (*UseBuffer)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes,
                    OMX_IN OMX_U8* pBuffer);

    OMX_ERRORTYPE (*AllocateBuffer)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes);

    OMX_ERRORTYPE (*FreeBuffer)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_U32 nPortIndex,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*EmptyThisBuffer)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*FillThisBuffer)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*SetCallbacks)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                    OMX_IN  OMX_PTR pAppData);

    OMX_ERRORTYPE (*ComponentDeInit)(
                    OMX_IN  MagOmxComponent hComponent,);

    OMX_ERRORTYPE (*UseEGLImage)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN void* eglImage);

    OMX_ERRORTYPE (*ComponentRoleEnum)(
                    OMX_IN  MagOmxComponent hComponent,
             		OMX_OUT OMX_U8 *cRole,
             		OMX_IN OMX_U32 nIndex);

    /*pure virtual functions and must be overrided*/
    OMX_ERRORTYPE (*Start)(OMX_IN  MagOmxComponent hComponent);

    OMX_ERRORTYPE (*Stop)(OMX_IN  MagOmxComponent hComponent);

    OMX_ERRORTYPE (*Resume)(OMX_IN  MagOmxComponent hComponent);

    OMX_ERRORTYPE (*Pause)(OMX_IN  MagOmxComponent hComponent);

    OMX_ERRORTYPE (*Prepare)(OMX_IN  MagOmxComponent hComponent);

    OMX_ERRORTYPE (*Preroll)(OMX_IN  MagOmxComponent hComponent);

    OMX_ERRORTYPE (*AllResourcesReady)(OMX_IN  MagOmxComponent hComponent);

    OMX_ERRORTYPE (*FreeResources)(OMX_IN  MagOmxComponent hComponent);

EndOfVirtuals;

ClassMembers(MagOmxComponent, Base, \
    OMX_COMPONENTTYPE *(*Create)(MagOmxComponent pBase); \
    void               (*setHandle)(MagOmxComponent pBase, OMX_COMPONENTTYPE *handle); \
    OMX_ERRORTYPE      (*postEvent)(MagOmxComponent pBase, OMX_EVENTTYPE event, OMX_U32 param1, OMX_U32 param2, OMX_PTR pEventData); \
    OMX_ERRORTYPE      (*postFillBufferDone)(MagOmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer);  \
    OMX_ERRORTYPE      (*postEmptyBufferDone)(MagOmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer); \
    OMX_ERRORTYPE      (*setState)(MagOmxComponent pBase, OMX_STATETYPE targetState); \
    OMX_ERRORTYPE      (*addPort)(MagOmxComponent pBase, OMX_U32 nPortIndex, MagOmxPort pPort); \
    MagOmxPort         (*getPort)(MagOmxComponent pBase, OMX_U32 nPortIndex); \ 
)
    OMX_COMPONENTTYPE      *mComponentHandle;  /*OMX spec defined Component handle*/
    OMX_CALLBACKTYPE       *mCallbacks;        /*the registered callbacks from OMX IL client*/
    OMX_PTR                mAppData;           /*the OMX IL client specific DATA for component callbacks using */

    MagMsgChannelHandle    mCmdEvtChannel;     /*command message dispatching logic*/

    OMX_STATETYPE          mState;             /*current component state*/

    RBTreeNodeHandle       mPortTreeRoot;      /*the root node of the red-black tree for component ports*/

EndOfClassMembers;

#endif
