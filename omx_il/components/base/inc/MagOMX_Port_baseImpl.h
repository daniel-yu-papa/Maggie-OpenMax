#ifndef __MAG_OMX_PORT_BASE_IMPL_H__
#define __MAG_OMX_PORT_BASE_IMPL_H__

#include "MagOMX_Port_base.h"

#define OMX_BUFFERFLAG_EXT_INFREELIST  0x01000000
#define OMX_BUFFERFLAG_EXT_INBUSYLIST  0x02000000

typedef struct{
    List_t node;
    OMX_BUFFERHEADERTYPE *omx_buffer;
}MagOMX_Port_Buffer_t;

DeclareClass(MagOmxPortBase, MagOmxPort);

Virtuals(MagOmxPortBase, MagOmxPort) 
    /*pure virtual functions and must be overrided by sub-component*/
    OMX_ERRORTYPE (*MagOMX_AllocateBuffer)(OMX_U8 **ppBuffer, OMX_U32 nSizeBytes);
    OMX_ERRORTYPE (*MagOMX_FreeBuffer)(OMX_U8 *pBuffer);
EndOfVirtuals;


ClassMembers(MagOmxPortBase, MagOmxPort, \
    
)
    MagMutexHandle               mhMutex;

    OMX_U32                      mBuffersTotal;
    List_t                       mFreeBufListHeader;
    List_t                       mBusyBufListHeader;
    
EndOfClassMembers;


#endif
