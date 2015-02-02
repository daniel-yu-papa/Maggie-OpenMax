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

#ifndef __MAG_OMX_COMPONENT_BASE_H__
#define __MAG_OMX_COMPONENT_BASE_H__

#include "framework/MagFramework.h"
#include "MagOMX_IL.h"

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
                    OMX_IN  OMX_HANDLETYPE hComponent);

    OMX_ERRORTYPE (*ComponentRoleEnum)(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex);
    
    OMX_ERRORTYPE (*UseEGLImage)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_IN OMX_U32 nPortIndex,
                    OMX_IN OMX_PTR pAppPrivate,
                    OMX_IN void* eglImage);

    OMX_COMPONENTTYPE *(*Create)(
                    OMX_IN MagOmxComponent pBase, 
                    OMX_IN OMX_PTR pAppData);
EndOfVirtuals;

ClassMembers(MagOmxComponent, Base, \
    OMX_COMPONENTTYPE *(*getComponentObj)(MagOmxComponent self); \
    void    (*setName)(MagOmxComponent self, OMX_U8 *pName);     \
    OMX_U8 *(*getName)(MagOmxComponent self);                    \
)
    OMX_COMPONENTTYPE *mpComponentObject;  /*OMX IL spec defined Component handle*/
    OMX_U8            mName[64];     
EndOfClassMembers;

static inline void CompLogPriv(MagOmxComponent hComp, 
                               int lvl, 
                               const char *tag,
                               const char *func,
                               const int  line, 
                               const char *pMsg, ...){ 
    char head[512] = "";                                                                           
    char message[1024] = "";                                         
    va_list args;                                               
                                                                
    va_start(args, pMsg); 
    if (hComp) {                                     
        snprintf(head, 512, "[comp: %s][%p] %s", 
                 hComp->getName(hComp),             
                 (void *)hComp,
                 pMsg); 
    }else{
        snprintf(head, 512, "[comp: NULL] %s",
                 pMsg);  
    }

    vsnprintf(message, 1024, head, args);   
    Mag_agilelogPrint(lvl, tag, func, line, message);                                  
    va_end(args);     
}

#define COMP_LOGV(hComp, ...)  CompLogPriv(hComp, AGILE_LOG_VERBOSE, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define COMP_LOGD(hComp, ...)  CompLogPriv(hComp, AGILE_LOG_DEBUG, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define COMP_LOGI(hComp, ...)  CompLogPriv(hComp, AGILE_LOG_INFO, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define COMP_LOGW(hComp, ...)  CompLogPriv(hComp, AGILE_LOG_WARN, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define COMP_LOGE(hComp, ...)  CompLogPriv(hComp, AGILE_LOG_ERROR, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#endif
