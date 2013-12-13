#ifndef __MAG_OMX_PORT_VIDEO_DECODER_H__
#define __MAG_OMX_PORT_VIDEO_DECODER_H__

#include "MagOMX_Port_base.h"

DeclareClass(MagOmxPort_VideoDecoder, MagOmxPort);

Virtuals(MagOmxPort_VideoDecoder, MagOmxPort)

EndOfVirtuals;

ClassMembers(MagOmxPort_VideoDecoder, MagOmxPort, \
    void (*getPortDefinition)(MagOmxPort_VideoDecoder hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef); \
    void (*setPortDefinition)(MagOmxPort_VideoDecoder hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef); \
    OMX_ERRORTYPE (*sendBuffer)(MagOmxPort_VideoDecoder hPort, OMX_BUFFERHEADERTYPE* pBuffer); \
)

EndOfClassMembers;

#endif