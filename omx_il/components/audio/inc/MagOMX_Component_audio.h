#ifndef __MAGOMX_COMPONENT_AUDIO_H__
#define __MAGOMX_COMPONENT_AUDIO_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

DeclareClass(MagOmxComponentAudio, MagOmxComponentImpl);

Virtuals(MagOmxComponentAudio, MagOmxComponentImpl) 
	OMX_ERRORTYPE (*MagOmx_Audio_GetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);
                    
    OMX_ERRORTYPE (*MagOmx_Audio_SetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);
EndOfVirtuals;

ClassMembers(MagOmxComponentAudio, MagOmxComponentImpl, \
	void (*self)(void); \
)
    MagMutexHandle         mhMutex;

EndOfClassMembers;

#endif