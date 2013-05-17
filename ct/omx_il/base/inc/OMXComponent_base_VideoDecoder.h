#ifndef __OMX_COMPONENT_BASE_VIDEODECODER_H__
#define __OMX_COMPONENT_BASE_VIDEODECODER_H__

DERIVEDCLASS(OMXComponent_VideoDecoder_PrivateBase_t, OMXComponentPrivateBase_t)
#define OMXComponent_VideoDecoder_PrivateBase_t_FIELDS \
    OMXComponentPrivateBase_t_FIELDS \
    OMXSubComponentCallbacks_t *videoDecoderComp_callbacks; /*the registered callbacks by the child component*/
    
ENDCLASS(OMXComponent_VideoDecoder_PrivateBase_t)

OMX_ERRORTYPE OMXComponent_VideoDecoder_SetCallbacks(OMX_HANDLETYPE hComp, OMXSubComponentCallbacks_t *cb);
#endif