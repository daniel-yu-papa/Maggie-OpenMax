#ifndef __MAG_OMX_COMPONENT_PORT_H__
#define __MAG_OMX_COMPONENT_PORT_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

typedef struct {
    List_t node;
    OMX_BUFFERHEADERTYPE *pHeader;
    OMX_BOOL isAllocator;
}OMXComponentPort_Buffer_t;

/*specifies the minimum number of buffers that the port requires*/
#define OMX_PORT_MIN_BUFFER_NUM  2

#define OMX_PORT_BUFFER_SIZE     (8 * 1024)

DeclareClass(MagOmxPort, Base);

Virtuals(MagOmxPort, Base) 
    OMX_ERRORTYPE (*enablePort)(MagOmxPort hPort);
    
    OMX_ERRORTYPE (*disablePort)(MagOmxPort hPort);

    OMX_ERRORTYPE (*flushPort)(MagOmxPort hPort);

    OMX_ERRORTYPE (*markBuffer)(MagOmxPort hPort, OMX_MARKTYPE * mark);

    OMX_ERRORTYPE (*AllocateBuffer)(
                  MagOmxPort hPort,
                  OMX_BUFFERHEADERTYPE** ppBufferHdr,
                  OMX_PTR pAppPrivate,
                  OMX_U32 nSizeBytes);
    
    OMX_ERRORTYPE (*FreeBuffer)(
                  MagOmxPort hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE (*UseBuffer)(
                   MagOmxPort hPort,
                   OMX_BUFFERHEADERTYPE **ppBufferHdr,
                   OMX_PTR pAppPrivate,
                   OMX_U32 nSizeBytes,
                   OMX_U8 *pBuffer);
EndOfVirtuals;


ClassMembers(MagOmxPort, Base, \
    void (*getPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef); \
    void (*setPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef); \
)
    OMX_PARAM_PORTDEFINITIONTYPE mPortDef;
    MagMutexHandle mhMutex;

    List_t         mBufListHeader;
    OMX_U32        mNumAssignedBuffers;

    magMempoolHandle mBufferPool;
EndOfClassMembers;


#endif
