#ifndef __OMXCOMPONENT_BASE_VIDEOENCODER_H__
#define __OMXCOMPONENT_BASE_VIDEOENCODER_H__

typedef struct omx_component_priv_base_obj{
    const OMXComponentPrivateBase_t parent;

    OMXSubComponentCallbacks_t *subComp_callbacks;
}OMXComponent_VideoEncoder_PrivateBase_t;

OMX_ERRORTYPE OMXComponent_VideoEncoder_SetCallbacks(OMX_HANDLETYPE hComp, OMXSubComponentCallbacks_t *cb);
#endif