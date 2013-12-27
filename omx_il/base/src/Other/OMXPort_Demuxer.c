#include "OMXComponentPort_ContainerDemuxer.h"
#include "OMXComponent_msg.h"

static void portBufferManager(void *msg, void *priv){
    portBufferOperationMsg_t *m = (portBufferOperationMsg_t *)msg;
    OMXComponentPort_ContainerDemuxer_t *port = (OMXComponentPort_ContainerDemuxer_t *)priv;
    OMX_COMPONENTTYPE *comp;
    List_t *tmpNode;
    
    if (NULL == port){
        AGILE_LOGE("invalid priv parameter!");
        return;
    }
    
    comp = port->pCompContainer;
    
    if (m->action == DO_FILL_BUFFER){
        tmpNode = port->filledBufListHead.next;
        if (tmpNode == &port->filledBufListHead){
            comp->demuxerComp_callbacks->demux(comp);
        }
    }
}

OMX_ERRORTYPE OMXComponentPort_ContainerDemuxer_Constructor(OMX_IN OMX_COMPONENTTYPE *pCompContainer,  
                                                                         OMX_IN OMX_U32 nPortIndex,
                                                                         OMX_IN OMX_BOOL isInput,
                                                                         OMX_OUT OMXComponentPort_ContainerDemuxer_t **ppCompPort){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    if(NULL == *ppCompPort){
        *ppCompPort = (OMXComponentPort_ContainerDemuxer_t *)calloc(1, sizeof(OMXComponentPort_ContainerDemuxer_t));
        if(NULL == *ppCompPort)
            return OMX_ErrorInsufficientResources;
    }
    
    ret = OMXComponentPort_base_Constructor(pCompContainer, nPortIndex, isInput, ppCompPort);
    if(OMX_ErrorNone != ret){
        goto failure;
    }
    
    (*ppCompPort)->sPortParam.eDomain = OMX_PortDomainOther;

    /* set default value*/
    (*ppCompPort)->sPortParam.format.other.eFormat = OMX_OTHER_FormatBinary;

    Mag_MsgChannelReceiverAttach((*ppCompPort)->bufferMgrHandle, portBufferManager, (void *)(*ppCompPort));

    return ret;
    
failure:
    if (*ppCompPort){
        free(*ppCompPort);
        *ppCompPort = NULL;
    }
    return ret;
    
}




