#ifndef __MAG_OMX_COMPONENT_BASE_H__
#define __MAG_OMX_COMPONENT_BASE_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"


DeclareClass(MagOmxComponent, Base);

Virtuals(MagOmxComponent, Base) 
    OMX_ERRORTYPE (*GetComponentVersion)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_STRING pComponentName,
                    OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                    OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID);
                    
    OMX_ERRORTYPE (*SendCommand)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_COMMANDTYPE Cmd,
                    OMX_IN  OMX_U32 nParam1,
                    OMX_IN  OMX_PTR pCmdData); 
                    
    OMX_ERRORTYPE (*GetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);
                    
    OMX_ERRORTYPE (*SetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);
                    
    OMX_ERRORTYPE (*GetConfig)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_INOUT OMX_PTR pComponentConfigStructure);
                    
    OMX_ERRORTYPE (*SetConfig)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_IN  OMX_PTR pComponentConfigStructure);
                    
    OMX_ERRORTYPE (*GetExtensionIndex)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_STRING cParameterName,
                    OMX_OUT OMX_INDEXTYPE* pIndexType);
                    
    OMX_ERRORTYPE (*GetState)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_STATETYPE* pState);
                    
    OMX_ERRORTYPE (*ComponentTunnelRequest)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_U32 nPort,
                    OMX_IN  OMX_HANDLETYPE hTunneledComp,
                    OMX_IN  OMX_U32 nTunneledPort,
                    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup);
                    
    OMX_ERRORTYPE (*UseBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes,
                    OMX_IN OMX_U8* pBuffer);
                    
    OMX_ERRORTYPE (*AllocateBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN OMX_U32 nSizeBytes);
                    
    OMX_ERRORTYPE (*FreeBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_U32 nPortIndex,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
                    
    OMX_ERRORTYPE (*EmptyThisBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
                    
    OMX_ERRORTYPE (*FillThisBuffer)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
                    
    OMX_ERRORTYPE (*SetCallbacks)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                    OMX_IN  OMX_PTR pAppData);
                    
    OMX_ERRORTYPE (*ComponentDeInit)(
                    OMX_IN  OMX_HANDLETYPE hComponent,);
                    
    OMX_ERRORTYPE (*UseEGLImage)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN void* eglImage);
                    
    OMX_ERRORTYPE (*ComponentRoleEnum)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
             		OMX_OUT OMX_U8 *cRole,
             		OMX_IN OMX_U32 nIndex);
EndOfVirtuals;

ClassMembers(MagOmxComponent, Base, \               
    OMX_COMPONENTTYPE *(*Create)(
                    OMX_IN MagOmxComponent pBase, 
                    OMX_IN OMX_PTR pAppData);       \
)
    OMX_COMPONENTTYPE      mComponentObject;  /*OMX IL spec defined Component handle*/
EndOfClassMembers;

#endif
