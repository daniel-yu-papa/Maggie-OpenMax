#ifndef __MAG_OMX_PORT_BASE_H__
#define __MAG_OMX_PORT_BASE_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

/*specifies the minimum number of buffers that the port requires*/
#define kPortBuffersMinNum       2
#define kPortBufferSize         (1024 * 1024)
#define kInvalidPortIndex       (0x7fffffff);

typedef struct{
    OMX_U32  portIndex;
    OMX_BOOL isInput;
}MagOmxPort_Constructor_Param_t;

DeclareClass(MagOmxPort, Base);

Virtuals(MagOmxPort, Base) 
    OMX_ERRORTYPE (*enablePort)(OMX_HANDLETYPE hPort, OMX_PTR AppData);
    
    OMX_ERRORTYPE (*disablePort)(OMX_HANDLETYPE hPort);

    OMX_ERRORTYPE (*flushPort)(OMX_HANDLETYPE hPort);

    OMX_ERRORTYPE (*markBuffer)(OMX_HANDLETYPE hPort, OMX_MARKTYPE * mark);

    /*To request the component to use a buffer allocated by the IL client or a buffer supplied by a tunneled component.*/
    OMX_ERRORTYPE (*UseBuffer)(
                   OMX_HANDLETYPE hPort,
                   OMX_BUFFERHEADERTYPE **ppBufferHdr,
                   OMX_PTR pAppPrivate,
                   OMX_U32 nSizeBytes,
                   OMX_U8 *pBuffer);

    /*To request that the component allocate a new buffer and buffer header*/
    OMX_ERRORTYPE (*AllocateBuffer)(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE** ppBufferHdr,
                  OMX_PTR pAppPrivate,
                  OMX_U32 nSizeBytes);

    /*To release a buffer and buffer header from the component*/
    OMX_ERRORTYPE (*FreeBuffer)(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);

    /*To send a filled buffer to an input port of a component.*/
    OMX_ERRORTYPE (*EmptyThisBuffer)(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);

    /*To send an empty buffer to an output port of a component.*/
    OMX_ERRORTYPE (*FillThisBuffer)(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);
EndOfVirtuals;


ClassMembers(MagOmxPort, Base, \
    void (*getPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef); \
    void (*setPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef); \
    
    void (*setParameter)(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 value);       \
    OMX_ERRORTYPE (*getParameter)(MagOmxPort hPort, OMX_INDEXTYPE nIndex); \
    
    OMX_BOOL (*isInputPort)(MagOmxPort hPort); \
    OMX_U32  (*getPortIndex)(MagOmxPort hPort); \

    OMX_U32 (*getDef_BufferCountActual)(MagOmxPort root); \
    OMX_BOOL (*getDef_Populated)(MagOmxPort root); \
    OMX_BOOL (*getDef_Enabled)(MagOmxPort root); \
    
    void (*setDef_Populated)(MagOmxPort root, OMX_BOOL flag); \
    void (*setDef_Enabled)(MagOmxPort root, OMX_BOOL flag); \
)
    OMX_PARAM_PORTDEFINITIONTYPE mPortDefinition;
    MagMiniDBHandle              mParametersDB;

    MagMutexHandle               mhMutex;
    MagMutexHandle               mhParamMutex;
    
EndOfClassMembers;


#endif
