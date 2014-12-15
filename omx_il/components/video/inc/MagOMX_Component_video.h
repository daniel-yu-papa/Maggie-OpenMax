#ifndef __MAGOMX_COMPONENT_VIDEO_H__
#define __MAGOMX_COMPONENT_VIDEO_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

DeclareClass(MagOmxComponentVideo, MagOmxComponentImpl);

Virtuals(MagOmxComponentVideo, MagOmxComponentImpl) 
	OMX_ERRORTYPE (*MagOmx_Video_GetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);
                    
    OMX_ERRORTYPE (*MagOmx_Video_SetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);
EndOfVirtuals;

ClassMembers(MagOmxComponentVideo, MagOmxComponentImpl, \
	void (*self)(void); \
)
    MagMutexHandle         mhMutex;
    MagMiniDBHandle        mParametersDB;

EndOfClassMembers;

#endif