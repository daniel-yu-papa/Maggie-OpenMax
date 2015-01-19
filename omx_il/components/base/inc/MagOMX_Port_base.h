#ifndef __MAG_OMX_PORT_BASE_H__
#define __MAG_OMX_PORT_BASE_H__
#include <stdio.h>
#include <stdarg.h>

#include "framework/MagFramework.h"
#include "MagOMX_IL.h"

/*specifies the minimum number of buffers that the port requires*/
#define kPortBuffersMinNum      (0)
#define kPortBufferSize         (0)
#define kInvalidPortIndex       (0x7FFFFFFF)

typedef enum{
  kSharedBuffer,
  kNoneSharedBuffer
}MagOmxPort_BufferPolicy_t;

typedef struct{
    OMX_U32  portIndex;
    OMX_BOOL isInput;
    OMX_BUFFERSUPPLIERTYPE bufSupplier;
    OMX_U32  formatStruct;
    OMX_U8   name[32];
}MagOmxPort_Constructor_Param_t;

typedef enum{
  kTunneledPortStatusEvt,
  kBufferPopulatedEvt
}MagOmxPort_Event_t;

typedef enum{
  kPort_State_Stopped,
  kPort_State_Paused,
  kPort_State_Running,
  kPort_State_Flushing,
  kPort_State_Flushed
}MagOmxPort_State_t;

typedef struct{
    List_t node;
    OMX_BUFFERHEADERTYPE* pBuffer;
    MagMessageHandle msg;
    OMX_PTR priv;
}MagOMX_Pending_Buffer_Node_t;

DeclareClass(MagOmxPort, Base);

Virtuals(MagOmxPort, Base) 
    OMX_ERRORTYPE (*Enable)(OMX_HANDLETYPE hPort);
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
                  OMX_BUFFERHEADERTYPE **ppBuffer);

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
                  MagMessageHandle pBufferHandler);

    void (*SendEvent)(
                  OMX_HANDLETYPE hPort,
                  MagOmxPort_Event_t evtType);

    /*Get Shared buffer message*/
    MagMessageHandle (*GetSharedBufferMsg)(OMX_HANDLETYPE hPort);

    /*Get the output buffer that holds the generated data from the Component*/
    OMX_BUFFERHEADERTYPE* (*GetOutputBuffer)(OMX_HANDLETYPE hPort);

    /*Put the output buffer*/
    void (*PutOutputBuffer)(OMX_HANDLETYPE hPort, OMX_BUFFERHEADERTYPE* pBuf);

    /*Get the port domain tpye*/
    OMX_PORTDOMAINTYPE (*GetDomainType)(OMX_HANDLETYPE hPort);

    /*Set the specific port definition*/
    OMX_ERRORTYPE (*SetPortSpecificDef)(OMX_HANDLETYPE hPort, void *pFormat);

    /*Get the specific port definition*/
    OMX_ERRORTYPE (*GetPortSpecificDef)(OMX_HANDLETYPE hPort, void *pFormat);

    /*Set port parameters*/
    OMX_ERRORTYPE (*SetParameter)(OMX_HANDLETYPE hPort, 
                                  OMX_INDEXTYPE nIndex, 
                                  OMX_PTR pPortParam);

    /*Get port parameters*/
    OMX_ERRORTYPE (*GetParameter)(OMX_HANDLETYPE hPort, 
                                  OMX_INDEXTYPE nIndex, 
                                  OMX_PTR pPortParam);

    /*put the buffer into the return list and wait for really returning at certain time*/
    OMX_ERRORTYPE (*putReturnBuffer)(OMX_HANDLETYPE hPort, 
                                     OMX_BUFFERHEADERTYPE* pBuffer, 
                                     MagMessageHandle msg, 
                                     OMX_PTR priv);

    /*send back the buffer to the source port*/
    OMX_ERRORTYPE (*sendReturnBuffer)(OMX_HANDLETYPE hPort, 
                                      OMX_BUFFERHEADERTYPE* pBuffer);

    /*send out the buffer to the destinate port*/
    OMX_ERRORTYPE (*sendOutputBuffer)(OMX_HANDLETYPE hPort, 
                                      OMX_BUFFERHEADERTYPE* pBuffer);

    /*directly send out the output buffer to APP*/
    void (*sendOutputBufferToAPP)(OMX_HANDLETYPE hPort, 
                                  OMX_BUFFERHEADERTYPE* pBufHeader);

EndOfVirtuals;


ClassMembers(MagOmxPort, Base, \
    OMX_U8 *(*getPortName)(MagOmxPort hPort); \
    OMX_ERRORTYPE (*getPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef); \
    OMX_ERRORTYPE (*setPortDefinition)(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef); \
    
    void          (*setParameter)(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 value);       \
    OMX_ERRORTYPE (*getParameter)(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 *pValue); \
    
    OMX_BOOL      (*isInputPort)(MagOmxPort hPort);  \
    OMX_BOOL      (*isBufferSupplier)(MagOmxPort hPort); \
    OMX_BOOL      (*isTunneled)(MagOmxPort hPort); \
    OMX_U32       (*getPortIndex)(MagOmxPort hPort); \

    OMX_U32       (*getDef_BufferCountActual)(MagOmxPort root); \
    OMX_U32       (*getDef_BufferSize)(MagOmxPort root); \
    OMX_BOOL      (*getDef_Populated)(MagOmxPort root); \
    OMX_BOOL      (*getDef_Enabled)(MagOmxPort root); \

    MagOmxPort_BufferPolicy_t (*getBufferPolicy)(MagOmxPort root); \
    MagOmxPort_State_t (*getState)(MagOmxPort root); \

    void          (*setTunneledFlag)(MagOmxPort hPort, OMX_BOOL setting); \
    void          (*setDef_BufferCountActual)(MagOmxPort root, OMX_U32 cnt); \
    void          (*setDef_BufferSize)(MagOmxPort root, OMX_U32 bufSize); \
    void          (*setDef_Populated)(MagOmxPort root, OMX_BOOL flag); \
    void          (*setDef_Enabled)(MagOmxPort root, OMX_BOOL flag); \
    void          (*setBufferPolicy)(MagOmxPort root, MagOmxPort_BufferPolicy_t policy); \
    void          (*setState)(MagOmxPort root, MagOmxPort_State_t st); \

    void          (*resetBufferSupplier)(MagOmxPort root); \

    void           (*setAttachedComponent)(MagOmxPort root, OMX_HANDLETYPE comp); \
    OMX_HANDLETYPE (*getAttachedComponent)(MagOmxPort root); \

    void           (*copyPortDef)(MagOmxPort root, OMX_PARAM_PORTDEFINITIONTYPE *pDest, OMX_PARAM_PORTDEFINITIONTYPE *pSrc); \
)
    OMX_PARAM_PORTDEFINITIONTYPE *mpPortDefinition;
    MagMiniDBHandle              mParametersDB;
    OMX_BUFFERSUPPLIERTYPE       mBufferSupplier;
    OMX_BUFFERSUPPLIERTYPE       mInitialBufferSupplier;

    MagMutexHandle               mhMutex;
    MagMutexHandle               mhParamMutex;

    OMX_BOOL                     mIsTunneled;
    MagOmxPort_BufferPolicy_t    mBufferPolicy;
    MagOmxPort_State_t           mState;

    OMX_HANDLETYPE               mAttachedComponent;

    OMX_U8                       mPortName[32];

EndOfClassMembers;

typedef struct{
    List_t node;
    MagOmxPort hPort; 
}MagOmx_Port_Node_t;

typedef struct{
    List_t node;        /*add into the List: mBufferList*/
    List_t runNode;     /*add into the List: mRunningBufferList*/

    /*bufferHeaderOwner: the port owns the buffer header allocation and free
     *                   NULL: means owned by the OMX IL Client
     */
    MagOmxPort bufferHeaderOwner;
    /*bufferOwner: the port owns the buffer allocation and free
     *                   NULL: means owned by the OMX IL Client
     */
    MagOmxPort bufferOwner;
    /*
     *returnBufPortListH: the head of the list linking all port nodes[MagOmx_Port_Node_t] 
     *                    where the buffer header need to be returned back.
     */
    List_t returnBufPortListH; 
    /*
     *outputBufPortListH: the head of the list linking all port nodes[MagOmx_Port_Node_t] 
     *                    where the buffer header need to be transmitted to.
     */
    List_t outputBufPortListH;
    /*
     *freeBufPortListH: the head of the list linking all free port nodes[MagOmx_Port_Node_t] 
     *                  The purpose is to reduce the malloc and free overhead.
     */
    List_t freeBufPortListH;

    OMX_BUFFERHEADERTYPE  *pOmxBufferHeader;

    /*
     * for debugging using only
     */
    OMX_U32 sequence;
}MagOMX_Port_Buffer_t;

static inline void PortLogPriv(MagOmxPort hPort, 
                               int lvl, 
                               const char *tag,
                               const char *func,
                               const int  line, 
                               const char *pMsg, ...){ 
    char head[512] = "";                                                                           
    char message[1024] = "";                                         
    va_list args;                                               
                                                                
    va_start(args, pMsg); 
    if (hPort) {                                     
      snprintf(head, 512, "[port: %s][%s][%s][%p] %s", 
                hPort->getPortName(hPort),             
                hPort->isTunneled(hPort) ? "T" : "N-T",            
                hPort->isBufferSupplier(hPort) ? "B-S" : "N-B-S",
                (void *)hPort,
                pMsg); 
    }else{
       snprintf(head, 512, "[port: NULL] %s",
                pMsg);  
    }

    vsnprintf(message, 1024, head, args);   
    Mag_agilelogPrint(lvl, tag, func, line, message);                                  
    va_end(args);     
}

#define PORT_LOGV(hPort, ...)  PortLogPriv(hPort, AGILE_LOG_VERBOSE, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define PORT_LOGD(hPort, ...)  PortLogPriv(hPort, AGILE_LOG_DEBUG, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define PORT_LOGI(hPort, ...)  PortLogPriv(hPort, AGILE_LOG_INFO, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define PORT_LOGW(hPort, ...)  PortLogPriv(hPort, AGILE_LOG_WARN, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#define PORT_LOGE(hPort, ...)  PortLogPriv(hPort, AGILE_LOG_ERROR, MODULE_TAG, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)

#endif
