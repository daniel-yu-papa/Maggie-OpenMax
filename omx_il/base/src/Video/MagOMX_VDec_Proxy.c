#include "MagOMX_VDec_Proxy.h"

OMX_ERRORTYPE virtual_MagOmx_VDec_Proxy_Start(OMX_IN  MagOmx_VDec_Base hVdec){
    OMX_ERRORTYPE ret;
    MagOmx_VDec_Proxy self = ooc_cast(hVdec, MagOmx_VDec_Proxy);

    if (NULL == self->mVDecObj) {
        self->loadVdec(self);
        if (NULL == self->mVDecObj){
            AGILE_LOGE("failed to load the real vdec object!");
            return OMX_ErrorInsufficientResources;
        }
    }
    ret = self->mVDecObj->Start(self->mVDecObj);
    return ret;
}

OMX_ERRORTYPE virtual_MagOmx_VDec_Proxy_Stop(OMX_IN  MagOmx_VDec_Base hVdec){
    OMX_ERRORTYPE ret;
    MagOmx_VDec_Proxy self = ooc_cast(hVdec, MagOmx_VDec_Proxy);

    if (NULL == self->mVDecObj) {
        self->loadVdec(self);
        if (NULL == self->mVDecObj){
            AGILE_LOGE("failed to load the real vdec object!");
            return OMX_ErrorInsufficientResources;
        }
    }
    ret = self->mVDecObj->Stop(self->mVDecObj);
    return ret;
}

OMX_ERRORTYPE virtual_MagOmx_VDec_Proxy_Resume(OMX_IN  MagOmx_VDec_Base hVdec){
    OMX_ERRORTYPE ret;
    MagOmx_VDec_Proxy self = ooc_cast(hVdec, MagOmx_VDec_Proxy);

    if (NULL == self->mVDecObj) {
        self->loadVdec(self);
        if (NULL == self->mVDecObj){
            AGILE_LOGE("failed to load the real vdec object!");
            return OMX_ErrorInsufficientResources;
        }
    }
    ret = self->mVDecObj->Resume(self->mVDecObj);
    return ret;
}

OMX_ERRORTYPE virtual_MagOmx_VDec_Proxy_Pause(OMX_IN  MagOmx_VDec_Base hVdec){
    OMX_ERRORTYPE ret;
    MagOmx_VDec_Proxy self = ooc_cast(hVdec, MagOmx_VDec_Proxy);

    if (NULL == self->mVDecObj) {
        self->loadVdec(self);
        if (NULL == self->mVDecObj){
            AGILE_LOGE("failed to load the real vdec object!");
            return OMX_ErrorInsufficientResources;
        }
    }
    ret = self->mVDecObj->Pause(self->mVDecObj);
    return ret;
}

OMX_ERRORTYPE virtual_MagOmx_VDec_Proxy_Prepare(OMX_IN  MagOmx_VDec_Base hVdec){
    OMX_ERRORTYPE ret;
    MagOmx_VDec_Proxy self = ooc_cast(hVdec, MagOmx_VDec_Proxy);

    if (NULL == self->mVDecObj) {
        self->loadVdec(self);
        if (NULL == self->mVDecObj){
            AGILE_LOGE("failed to load the real vdec object!");
            return OMX_ErrorInsufficientResources;
        }
    }
    ret = self->mVDecObj->Prepare(self->mVDecObj);
    return ret;
}

OMX_ERRORTYPE virtual_MagOmx_VDec_Proxy_Preroll(OMX_IN  MagOmx_VDec_Base hVdec){
    OMX_ERRORTYPE ret;
    MagOmx_VDec_Proxy self = ooc_cast(hVdec, MagOmx_VDec_Proxy);

    if (NULL == self->mVDecObj) {
        self->loadVdec(self);
        if (NULL == self->mVDecObj){
            AGILE_LOGE("failed to load the real vdec object!");
            return OMX_ErrorInsufficientResources;
        }
    }
    ret = self->mVDecObj->Preroll(self->mVDecObj);
    return ret;
}


void MagOmx_VDec_Proxy_LoadVdec(MagOmx_VDec_Proxy thiz){
    if (NULL == thiz->mVDecObj){

    }
}

/*Class Constructor/Destructor*/
static void MagOmx_VDec_Proxy_initialize(Class this){
    MagOmx_VDec_ProxyVtableInstance.MagOmx_VDec_Base.Start    = virtual_MagOmx_VDec_Proxy_Start;
    MagOmx_VDec_ProxyVtableInstance.MagOmx_VDec_Base.Stop     = virtual_MagOmx_VDec_Proxy_Stop;  
    MagOmx_VDec_ProxyVtableInstance.MagOmx_VDec_Base.Resume   = virtual_MagOmx_VDec_Proxy_Resume;
    MagOmx_VDec_ProxyVtableInstance.MagOmx_VDec_Base.Pause    = virtual_MagOmx_VDec_Proxy_Pause;
    MagOmx_VDec_ProxyVtableInstance.MagOmx_VDec_Base.Prepare  = virtual_MagOmx_VDec_Proxy_Prepare;
    MagOmx_VDec_ProxyVtableInstance.MagOmx_VDec_Base.Preroll  = virtual_MagOmx_VDec_Proxy_Preroll;
    
    /*pure virtual functions and must be overrided*/
    MagOmx_VDec_ProxyVtableInstance.GetCodecParameter = NULL;
    MagOmx_VDec_ProxyVtableInstance.SetCodecParameter = NULL;
    MagOmx_VDec_ProxyVtableInstance.GetCodecConfig    = NULL;
    MagOmx_VDec_ProxyVtableInstance.SetCodecConfig    = NULL;
}

static void MagOmx_VDec_Proxy_constructor(MagOmx_VDec_Proxy thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(MagOmx_VDec_Proxy));
    chain_constructor(MagOmx_VDec_Proxy, thiz, params);

    thiz->mVDecObj = NULL;
    
    thiz->loadVdec = MagOmx_VDec_Proxy_LoadVdec;
}

static void MagOmx_VDec_Proxy_destructor(MagOmx_VDec_Proxy thiz, MagOmx_VDec_ProxyVtableInstance vtab){
    AGILE_LOGV("Enter!");
}


MagOmx_VDec_Proxy MagOMX_VDec_Create(){
    return (MagOmx_VDec_Proxy) ooc_new( MagOmx_VDec_Proxy, NULL);
}


