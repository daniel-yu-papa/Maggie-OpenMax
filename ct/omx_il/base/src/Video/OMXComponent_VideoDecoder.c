#include "OMXComponent_base_VideoDecoder.h"


static OMX_ERRORTYPE VideoDecoder_getParameter(OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nParamIndex,
                        OMX_INOUT OMX_PTR pComponentParameterStructure){

}

static OMX_ERRORTYPE VideoDecoder_setParameter(OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_IN OMX_PTR pComponentParameterStructure){

}

static OMX_ERRORTYPE VideoDecoder_setConfig(OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_INOUT OMX_PTR pComponentConfigStructure){

}

static OMX_ERRORTYPE VideoDecoder_getConfig(OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_INOUT OMX_PTR pComponentConfigStructure){

}

static OMX_ERRORTYPE OMXComponent_Base_VideoDecoder_Constructor(
                     OMX_HANDLETYPE *hComp,
                     OMX_BOOL       isSyncMode){
    OMXComponent_VideoDecoder_PrivateBase_t *priv = NULL;
    OMXSubComponentCallbacks_t *cb;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    if(NULL == *hComp){
        *hComp = (OMX_COMPONENTTYPE *)calloc(1, sizeof(OMX_COMPONENTTYPE));
        if (NULL == *hComp){
            ret = OMX_ErrorInsufficientResources;
            AGILE_LOG_FATAL("failed to allocate memory for OMX Component");
            goto failure;
        }
    }

    if (NULL == (*hComp)->pComponentPrivate){
        priv = (OMXComponent_VideoDecoder_PrivateBase_t *)calloc(1, sizeof(OMXComponent_VideoDecoder_PrivateBase_t));
        if(NULL == priv){
            ret = OMX_ErrorInsufficientResources;
            AGILE_LOG_FATAL("failed to allocate memory for OMX VideoDecoder Component Private Data");
            goto failure;
        }
        (*hComp)->pComponentPrivate = priv;
    }else{
        priv = (OMXComponent_VideoDecoder_PrivateBase_t *)(*hComp)->pComponentPrivate;
    }
    
    ret = OMXComponent_Base_Constructor(hComp, isSyncMode);
    if (OMX_ErrorNone != ret){
        goto failure;
    }
    
    cb = (OMXSubComponentCallbacks_t *)malloc(sizeof(OMXSubComponentCallbacks_t));
    if (NULL == cb){
        AGILE_LOGE("failed to allocate memory for OMXSubComponentCallbacks_t");
    }

    cb->getParameter = VideoDecoder_getParameter;
    cb->setParameter = VideoDecoder_setParameter;
    cb->getConfig    = VideoDecoder_getConfig;
    cb->setConfig    = VideoDecoder_setConfig;
    
    OMXComponent_Base_SetCallbacks(*hComp, cb);

    return ret;
    
failure:
    if (priv)
        free(priv);

    if (*hComp)
        free(*hComp);
    
    return ;
}

static OMX_ERRORTYPE OMXComponent_Base_VideoEncoder_Destructor(
                     OMX_HANDLETYPE *hComp,
                     OMX_BOOL       dynamic){
}

OMX_ERRORTYPE OMXComponent_VideoDecoder_SetCallbacks(OMX_HANDLETYPE hComp, OMXSubComponentCallbacks_t *cb){
    OMX_COMPONENTTYPE *pComp;
    OMXComponent_VideoDecoder_PrivateBase_t *priv;
    
    if(NULL == hComp)
        return OMX_ErrorBadParameter;
    
    pComp = (OMX_COMPONENTTYPE *)hComp;

    priv = (OMXComponent_VideoDecoder_PrivateBase_t *)pComp->pComponentPrivate;
    priv->videoDecoderComp_callbacks = *cb;
    
    return OMX_ErrorNone;
}

