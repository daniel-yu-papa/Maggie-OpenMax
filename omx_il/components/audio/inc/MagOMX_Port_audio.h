#ifndef __MAGOMX_PORT_AUDIO_H__
#define __MAGOMX_PORT_AUDIO_H__

#include "MagOMX_Port_base.h"
#include "MagOMX_Port_baseImpl.h"

typedef struct{
    List_t node;

    OMX_AUDIO_CODINGTYPE eEncoding;
}MagOMX_Audio_PortFormat_t;

DeclareClass(MagOmxPortAudio, MagOmxPortImpl);

Virtuals(MagOmxPortAudio, MagOmxPortImpl) 
   
EndOfVirtuals;

ClassMembers(MagOmxPortAudio, MagOmxPortImpl, \
	void (*addFormat)(MagOmxPortAudio hPort, MagOMX_Audio_PortFormat_t *pEncoding); \
)
    MagMutexHandle         mhMutex;
    MagMiniDBHandle        mParametersDB;
    /*
     *mPortFormatList.next links to the default port format(highest preference format)
     *the later nodes are lower preference formats
     */
    List_t                 mPortFormatList;

    OMX_AUDIO_PORTDEFINITIONTYPE *mpPortDefinition;

EndOfClassMembers;

#endif