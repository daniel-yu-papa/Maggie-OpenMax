#ifndef __OMX_COMPONENT_MSG_H__
#define __OMX_COMPONENT_MSG_H__

typedef enum{
    DO_FILL_BUFFER,
    DO_EMPTY_BUFFER,
    DO_FREE_BUFFER
}portBufferOperationType_t;

typedef struct{
    portBufferOperationType_t action;
    OMX_HANDLETYPE hComponent;
    OMX_BUFFERHEADERTYPE *pBuffer;
}portBufferOperationMsg_t;

#endif
