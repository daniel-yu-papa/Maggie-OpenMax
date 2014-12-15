#ifndef __MAGOMX_PORT_VIDEO_H__
#define __MAGOMX_PORT_VIDEO_H__

#include "MagOMX_Port_base.h"
#include "MagOMX_Port_baseImpl.h"

typedef struct{
	List_t node;

	OMX_U32              xFramerate;
	OMX_VIDEO_CODINGTYPE eCompressionFormat;
    OMX_COLOR_FORMATTYPE eColorFormat;
}MagOMX_Video_PortFormat_t;

DeclareClass(MagOmxPortVideo, MagOmxPortImpl);

Virtuals(MagOmxPortVideo, MagOmxPortImpl) 
   
EndOfVirtuals;

ClassMembers(MagOmxPortVideo, MagOmxPortImpl, \
    /*add the supported video format from the vendor port */
	void (*addFormat)(MagOmxPortVideo hPort, MagOMX_Video_PortFormat_t *pFormat); \
)
    MagMutexHandle         mhMutex;
    /*
     *mPortFormatList.next links to the default port format(highest preference format)
     *the later nodes are lower preference formats
     */
    List_t                 mPortFormatList;

    OMX_VIDEO_PORTDEFINITIONTYPE *mpPortDefinition;

EndOfClassMembers;

#endif