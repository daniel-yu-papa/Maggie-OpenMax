#ifndef __OMX_COMPONENT_PORT_H__
#define __OMX_COMPONENT_PORT_H__

#include "OMX_ClassMagic.h"

/*the number of buffers that are required on this port before it is populated*/
#define DEFAULT_BUFFER_NUM_PER_PORT  2

/*specifies the minimum number of buffers that the port requires*/
#define DEFAULT_MIN_BUFFER_NUM_PER_PORT 2

#define DEFAULT_BASE_PORT_BUF_SIZE   (4 * 1024)

typedef struct {
    List_t node;
    OMX_BUFFERHEADERTYPE *pHeader;
    OMX_U32 nBufSize;
}OMXComponentPort_Buffer_t;

CLASS(OMXComponentPort_base_t)
#define OMXComponentPort_base_t_FIELDS \
    List_t node;                             /* the list node that locates at the "portListHead" in the OMX Component Container */ \
    OMX_COMPONENTTYPE *pCompContainer;       /* the OMX component reference that contains the port */ \
    OMX_PARAM_PORTDEFINITIONTYPE sPortParam;  /* the specific port parameters */ \
    \
    List_t portBufListHead; /* list the allocated buffers, the maximum number of the buffers should be sPortParam.nBufferCountActual*/\
    OMX_U8 numPortBufAlloc; /* the number of the allocated buffers */ \
    MagMsgChannelHandle bufferMgrHandle; /*the message channel for sending port buffer management msg*/ \
    \
    OMX_ERRORTYPE (*PortDestructor)(OMXComponentPort_base_t *pPort); /**< The destructor of the port*/ \
    OMX_ERRORTYPE (*Port_AllocateBuffer)(OMXComponentPort_base_t *pPort, \
                                         OMX_BUFFERHEADERTYPE** ppBuffer, \ 
                                         OMX_U32 nPortIndex, \
                                         OMX_PTR pAppPrivate, \
                                         OMX_U32 nSizeBytes);/**< Replaces the AllocateBuffer call for the base port. */   \
    OMX_ERRORTYPE (*Port_UseBuffer)(OMXComponentPort_base_t *pPort, \
                                    OMX_BUFFERHEADERTYPE** ppBufferHdr, \
                                    OMX_U32 nPortIndex, \
                                    OMX_PTR pAppPrivate, \
                                    OMX_U32 nSizeBytes, \
                                    OMX_U8* pBuffer);/**< The standard use buffer function applied to the port class */ \
    OMX_ERRORTYPE (*Port_FreeBuffer)(OMXComponentPort_base_t *pPort, \
                                     OMX_U32 nPortIndex, \
                                     OMX_BUFFERHEADERTYPE* pBuffer); /**< The standard free buffer function applied to the port class */   
ENDCLASS(OMXComponentPort_base_t)





#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

DeclareClass(OmxPort, Base);

Virtuals(OmxPort, Base) 
    OMX_ERRORTYPE (*enablePort)(OmxPort hPort);
    
    OMX_ERRORTYPE (*disablePort)(OmxPort hPort);

    OMX_ERRORTYPE (*flushPort)(OmxPort hPort);

    OMX_ERRORTYPE (*markBuffer)(OmxPort hPort, OMX_MARKTYPE * mark);

    OMX_ERRORTYPE (*AllocateBuffer)(
                  OmxPort hPort,
                  OMX_BUFFERHEADERTYPE** ppBuffer,
                  OMX_PTR pAppPrivate,
                  OMX_U32 nSizeBytes);

    OMX_ERRORTYPE (*FreeBuffer)(
                  OmxPort hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);
 
EndOfVirtuals;


ClassMembers(OmxPort, Base, \
    OMX_PARAM_PORTDEFINITIONTYPE *(*getPortDefinition)(OmxPort hPort); \
    OMX_ERRORTYPE (*setPortDefinition)(OmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef); \
)
    OMX_PARAM_PORTDEFINITIONTYPE mPortDef;
EndOfClassMembers;


#endif
