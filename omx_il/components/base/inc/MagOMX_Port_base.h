#ifndef __MAG_OMX_PORT_BASE_H__
#define __MAG_OMX_PORT_BASE_H__

#include "ooc.h"
#include "OMX_Core.h"
#include "OMX_Types.h"

/*specifies the minimum number of buffers that the port requires*/
#define kPortBuffersMinNum       2
#define kPortBufferSize         (1024 * 1024)
#define kInvalidPortIndex       (0x7fffffff);

typedef enum{
  kSharedBuffer,
  kNoneSharedBuffer
}MagOmxPort_BufferPolicy_t;

typedef struct{
    OMX_U32  portIndex;
    OMX_BOOL isInput;
    OMX_BUFFERSUPPLIERTYPE bufSupplier;
}MagOmxPort_Constructor_Param_t;

typedef enum{
  kTunneledPortStatusEvt,
  kBufferPopulatedEvt
}MagOmxPort_Event_t;

DeclareClass(MagOmxPort, Base);

Virtuals(MagOmxPort, Base) 
    OMX_ERRORTYPE (*Enable)(OMX_HANDLETYPE hPort, OMX_PTR AppData);
    OMX_ERRORTYPE (*Disable)(OMX_HANDLETYPE hPort);
    OMX_ERRORTYPE (*Run)(OMX_HANDLETYPE hPort);
    OMX_ERRORTYPE (*Flush)(OMX_HANDLETYPE hPort);
    OMX_ERRORTYPE (*Pause)(OMX_HANDLETYPE hPort);
    OMX_ERRORTYPE (*Resume)(OMX_HANDLETYPE hPort);

    OMX_ERRORTYPE (*MarkBuffer)(OMX_HANDLETYPE hPort, OMX_MARKTYPE * mark);

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

    /*To request that the component allocate the buffer for tunneled ports*/
    OMX_ERRORTYPE (*AllocateTunnelBuffer)(
                  OMX_HANDLETYPE hPort);

    /*To release a buffer and buffer header from the port*/
    OMX_ERRORTYPE (*FreeBuffer)(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);

    /*To release all buffers and buffer headers from the port*/
    OMX_ERRORTYPE (*FreeAllBuffers)(
                  OMX_HANDLETYPE hPort);

    /*To free the buffer holding by tunneled ports*/
    OMX_ERRORTYPE (*FreeTunnelBuffer)(
                  OMX_HANDLETYPE hPort);

    /*To send a filled buffer to an input port of a component.*/
    OMX_ERRORTYPE (*EmptyThisBuffer)(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);

    /*To send an empty buffer to an output port of a component.*/
    OMX_ERRORTYPE (*FillThisBuffer)(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer);

    /*To setup the tunnel between the hPort and nTunneledPort.*/
    OMX_ERRORTYPE (*SetupTunnel)(
                  OMX_HANDLETYPE hPort,
                  OMX_HANDLETYPE hTunneledComp,
                  OMX_U32        nTunneledPortIndex,
                  OMX_TUNNELSETUPTYPE* pTunnelSetup);

    /*Register the buffer handling message. only works on Input port*/
    OMX_ERRORTYPE (*RegisterBufferHandler)(
                  OMX_HANDLETYPE hPort,
                  MagEventHandle pBufferHandler);

    void (*SendEvent)(
                  OMX_HANDLETYPE hPort,
                  MagOmxPort_Event_t evtType);

    /*Get Shared buffer message*/
    MagMessageHandle (*GetSharedBufferMsg)(OMX_HANDLETYPE hPort);
EndOfVirtuals;


ClassMembers(MagOmxPort, Base, \
    void          (*getPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef); \
    void          (*setPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef); \
    
    void          (*setParameter)(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 value);       \
    OMX_ERRORTYPE (*getParameter)(MagOmxPort hPort, OMX_INDEXTYPE nIndex); \
    
    OMX_BOOL      (*isInputPort)(MagOmxPort hPort);  \
    OMX_BOOL      (*isBufferSupplier)(MagOmxPort hPort); \
    OMX_BOOL      (*isTunneled)(MagOmxPort hPort); \
    OMX_U32       (*getPortIndex)(MagOmxPort hPort); \

    OMX_U32       (*getDef_BufferCountActual)(MagOmxPort root); \
    OMX_U32       (*getDef_BufferSize)(MagOmxPort root); \
    OMX_BOOL      (*getDef_Populated)(MagOmxPort root); \
    OMX_BOOL      (*getDef_Enabled)(MagOmxPort root); \
    MagOmxPort_BufferPolicy_t (*getBufferPolicy)(MagOmxPort root); \

    void          (*setTunneledFlag)(MagOmxPort hPort, OMX_BOOL setting); \
    void          (*setDef_BufferCountActual)(MagOmxPort root, OMX_U32 cnt); \
    void          (*setDef_BufferSize)(MagOmxPort root, OMX_U32 bufSize); \
    void          (*setDef_Populated)(MagOmxPort root, OMX_BOOL flag); \
    void          (*setDef_Enabled)(MagOmxPort root, OMX_BOOL flag); \
    void          (*setBufferPolicy)(MagOmxPort root, MagOmxPort_BufferPolicy_t policy); \
    void          (*resetBufferSupplier)(MagOmxPort root); \
)
    OMX_PARAM_PORTDEFINITIONTYPE *mpPortDefinition;
    MagMiniDBHandle              mParametersDB;
    OMX_BUFFERSUPPLIERTYPE       mBufferSupplier;
    OMX_BUFFERSUPPLIERTYPE       mInitialBufferSupplier;

    MagMutexHandle               mhMutex;
    MagMutexHandle               mhParamMutex;

    OMX_BOOL                     mIsTunneled;
    MagOmxPort_BufferPolicy_t    mBufferPolicy;

EndOfClassMembers;


#endif
