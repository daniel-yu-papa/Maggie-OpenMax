#include "OMXComponentPort_VideoDecoder.h"

OMX_ERRORTYPE OMXComponentPort_VideoDecoder_Constructor(OMX_IN OMX_COMPONENTTYPE *pCompContainer,  
                                                                         OMX_IN OMX_U32 nPortIndex,
                                                                         OMX_IN OMX_BOOL isInput,
                                                                         OMX_OUT OMXComponentPort_VideoDecoder_t **ppCompPort){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    if(NULL == *ppCompPort){
        *ppCompPort = (OMXComponentPort_VideoDecoder_t *)calloc(1, sizeof(OMXComponentPort_VideoDecoder_t));
        if(NULL == *ppCompPort)
            return OMX_ErrorInsufficientResources;
    }
    
    ret = OMXComponentPort_base_Constructor(pCompContainer, nPortIndex, isInput, ppCompPort);
    if(OMX_ErrorNone != ret){
        goto failure;
    }
    
    (*ppCompPort)->sPortParam.eDomain = OMX_PortDomainVideo;

    /* set default value*/
    (*ppCompPort)->sPortParam.format.video.cMIMEType = malloc(DEFAULT_VIDEO_MIME_STRING_LENGTH);
    strcpy((*ppCompPort)->sPortParam.format.video.cMIMEType, "raw/video");
    (*ppCompPort)->sPortParam.format.video.pNativeRender = NULL;
    (*ppCompPort)->sPortParam.format.video.nFrameWidth = 0;
    (*ppCompPort)->sPortParam.format.video.nFrameHeight = 0;
    (*ppCompPort)->sPortParam.format.video.nStride = 0;
    (*ppCompPort)->sPortParam.format.video.nSliceHeight = 0;
    (*ppCompPort)->sPortParam.format.video.nBitrate = 0;
    (*ppCompPort)->sPortParam.format.video.xFramerate = 0;
    (*ppCompPort)->sPortParam.format.video.bFlagErrorConcealment = OMX_FALSE;
    (*ppCompPort)->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    (*ppCompPort)->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    (*ppCompPort)->sPortParam.format.video.pNativeWindow = NULL;
    
failure:
    if (*ppCompPort){
        free(*ppCompPort);
        *ppCompPort = NULL;
    }
    return ret;
    
}