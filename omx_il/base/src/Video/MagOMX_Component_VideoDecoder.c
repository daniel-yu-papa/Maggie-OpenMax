#include "MagOMX_Component_VideoDecoder.h"

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_GetParameter(
                    OMX_IN  MagOmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_SetParameter(
                    OMX_IN  MagOmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_GetConfig(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_INOUT OMX_PTR pComponentConfigStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_SetConfig(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_IN  OMX_PTR pComponentConfigStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_Start(OMX_IN  MagOmxComponent hComponent){
    OMX_ERRORTYPE ret;
    MagOmxComponent_VideoDecoder self = ooc_cast(hComponent, MagOmxComponent_VideoDecoder);

    if (self->mVdecObj)
        ret = self->mVdecObj->Start(self->mVdecObj);
    else
        ret = OMX_ErrorNotImplemented;

    return ret;
}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_Stop(OMX_IN  MagOmxComponent hComponent){
    OMX_ERRORTYPE ret;
    MagOmxComponent_VideoDecoder self = ooc_cast(hComponent, MagOmxComponent_VideoDecoder);

    if (self->mVdecObj)
        ret = self->mVdecObj->Stop(self->mVdecObj);
    else
        ret = OMX_ErrorNotImplemented;

    return ret;
}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_Resume(OMX_IN  MagOmxComponent hComponent){
    OMX_ERRORTYPE ret;
    MagOmxComponent_VideoDecoder self = ooc_cast(hComponent, MagOmxComponent_VideoDecoder);

    if (self->mVdecObj)
        ret = self->mVdecObj->Resume(self->mVdecObj);
    else
        ret = OMX_ErrorNotImplemented;

    return ret;
}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_Pause(OMX_IN  MagOmxComponent hComponent){
    OMX_ERRORTYPE ret;
    MagOmxComponent_VideoDecoder self = ooc_cast(hComponent, MagOmxComponent_VideoDecoder);

    if (self->mVdecObj)
        ret = self->mVdecObj->Pause(self->mVdecObj);
    else
        ret = OMX_ErrorNotImplemented;

    return ret;
}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_Prepare(OMX_IN  MagOmxComponent hComponent){
    OMX_ERRORTYPE ret;
    MagOmxComponent_VideoDecoder self = ooc_cast(hComponent, MagOmxComponent_VideoDecoder);

    if (self->mVdecObj)
        ret = self->mVdecObj->Prepare(self->mVdecObj);
    else
        ret = OMX_ErrorNotImplemented;

    return ret;
}

OMX_ERRORTYPE virtual_MagOmxComponent_VideoDecoder_Preroll(OMX_IN  MagOmxComponent hComponent){
    OMX_ERRORTYPE ret;
    MagOmxComponent_VideoDecoder self = ooc_cast(hComponent, MagOmxComponent_VideoDecoder);

    if (self->mVdecObj)
        ret = self->mVdecObj->Preroll(self->mVdecObj);
    else
        ret = OMX_ErrorNotImplemented;

    return ret;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_VideoDecoder_initialize(Class this){
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.GetParameter    = virtual_MagOmxComponent_VideoDecoder_GetParameter;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.SetParameter    = virtual_MagOmxComponent_VideoDecoder_SetParameter;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.GetConfig       = virtual_MagOmxComponent_VideoDecoder_GetConfig;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.SetConfig       = virtual_MagOmxComponent_VideoDecoder_SetConfig;

    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.Start           = virtual_MagOmxComponent_VideoDecoder_Start;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.Stop            = virtual_MagOmxComponent_VideoDecoder_Stop;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.Resume          = virtual_MagOmxComponent_VideoDecoder_Resume;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.Pause           = virtual_MagOmxComponent_VideoDecoder_Pause;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.Prepare         = virtual_MagOmxComponent_VideoDecoder_Prepare;
    MagOmxComponent_VideoDecoderVtableInstance.MagOmxComponent.Preroll         = virtual_MagOmxComponent_VideoDecoder_Preroll;
    
    /*pure virtual functions and must be overrided*/
    MagOmxComponent_VideoDecoderVtableInstance.GetCodecParameter = NULL;
    MagOmxComponent_VideoDecoderVtableInstance.SetCodecParameter = NULL;
    MagOmxComponent_VideoDecoderVtableInstance.GetCodecConfig    = NULL;
    MagOmxComponent_VideoDecoderVtableInstance.SetCodecConfig    = NULL;
}

static void MagOmxComponent_VideoDecoder_constructor(MagOmxComponent_VideoDecoder thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(MagOmxComponent_VideoDecoder));
    chain_constructor(MagOmxComponent_VideoDecoder, thiz, params);

    thiz->mVdecObj = MagOMX_VDec_Create((char *)params);
}

static void MagOmxComponent_destructor(MagOmxComponent_VideoDecoder thiz, MagOmxComponent_VideoDecoderVtable vtab){
    AGILE_LOGV("Enter!");
}


