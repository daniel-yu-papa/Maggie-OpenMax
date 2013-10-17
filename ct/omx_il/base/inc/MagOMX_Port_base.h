#ifndef __MAG_OMX_COMPONENT_PORT_H__
#define __MAG_OMX_COMPONENT_PORT_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

/*specifies the minimum number of buffers that the port requires*/
#define OMX_PORT_MIN_BUFFER_NUM  2

#define OMX_PORT_BUFFER_SIZE     (8 * 1024)

typedef struct {
    List_t node;
    OMX_BUFFERHEADERTYPE *pHeader;
    OMX_BOOL isAllocator;
}OMXComponentPort_Buffer_t;

typedef struct{
    OMX_U32 msg_idx;
    OMX_BUFFERHEADERTYPE *pBufHeader;
}BufferMsg_t;

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

    /*the pure virtual function: must be overrided by subclass*/
    OMX_ERRORTYPE (*ProcessBuffer)(
                   MagOmxPort hPort,
                   OMX_BUFFERHEADERTYPE *pBufferHdr);
EndOfVirtuals;


ClassMembers(MagOmxPort, Base, \
    void (*getPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef); \
    void (*setPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef); \
    OMX_ERRORTYPE (*sendBuffer)(MagOmxPort hPort, OMX_BUFFERHEADERTYPE* pBuffer); \
)
    OMX_PARAM_PORTDEFINITIONTYPE mPortDef;
    MagMutexHandle               mhMutex;

    List_t                       mBufListHeader;
    OMX_U32                      mNumAssignedBuffers;

    magMempoolHandle             mBufferPool;
    MagMsgChannelHandle          mhBufMsgChannel;
    OMX_U32                      mNumBufReceived;
    
EndOfClassMembers;


#endif
