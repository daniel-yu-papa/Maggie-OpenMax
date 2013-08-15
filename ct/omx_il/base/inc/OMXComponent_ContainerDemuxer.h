#ifndef __OMX_COMPONENT_BASE_CONTAINER_DEMUXER_H__
#define __OMX_COMPONENT_BASE_CONTAINER_DEMUXER_H__

DERIVEDCLASS(OMXComponent_ContainerDemux_PrivateBase_t, OMXComponentPrivateBase_t)
#define OMXComponent_ContainerDemux_PrivateBase_t_FIELDS \
    OMXComponentPrivateBase_t_FIELDS \
    CPA_PIPETYPE *pipe; \
    CPA_HANDLE content; \
    OMXSubComponentCallbacks_t *demuxerComp_callbacks; /*the registered callbacks by the child component*/    
ENDCLASS(OMXComponent_ContainerDemux_PrivateBase_t)

typedef struct{
    /** demuxing function. */
    OMX_ERRORTYPE (*demux) (OMX_HANDLETYPE hComp);
    
}OMXComponentDemuxerCallbacks_t;

OMX_ERRORTYPE OMXComponent_ContainerDemux_SetCallbacks(OMX_HANDLETYPE hComp, OMXComponentDemuxerCallbacks_t *cb);

#endif