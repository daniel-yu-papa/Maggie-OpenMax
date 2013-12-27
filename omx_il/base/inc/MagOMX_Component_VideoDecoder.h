#ifndef __MAGOMX_COMPONENT_VIDEODECODER_H__
#define __MAGOMX_COMPONENT_VIDEODECODER_H__

DeclareClass(MagOmxComponent_VideoDecoder, MagOmxComponent);

Virtuals(MagOmxComponent_VideoDecoder, MagOmxComponent) 
    OMX_ERRORTYPE (*GetCodecParameter)(
                    OMX_IN  MagOmxComponent_VideoDecoder hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*SetCodecParameter)(
                    OMX_IN  MagOmxComponent_VideoDecoder hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*GetCodecConfig)(
                    OMX_IN  MagOmxComponent_VideoDecoder hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_INOUT OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*SetCodecConfig)(
                    OMX_IN  MagOmxComponent_VideoDecoder hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_IN  OMX_PTR pComponentConfigStructure);
EndOfVirtuals;


ClassMembers(MagOmxComponent_VideoDecoder, MagOmxComponent \
)
    MagOmx_VDec_Proxy *mVdecObj;
EndOfClassMembers;

#endif