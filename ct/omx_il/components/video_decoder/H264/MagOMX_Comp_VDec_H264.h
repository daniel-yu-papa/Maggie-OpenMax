#ifdef  __MAGOMX_COMP_VDEC_H264__
#define __MAGOMX_COMP_VDEC_H264__

DeclareClass(MagOmxComponent_VDec_H264, MagOmxComponent_VideoDecoder);

Virtuals(MagOmxComponent_VDec_H264, MagOmxComponent_VideoDecoder) 
    OMX_ERRORTYPE (*GetCodecParameter)(
                    OMX_IN  MagOmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*SetCodecParameter)(
                    OMX_IN  MagOmxComponent hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);

    OMX_ERRORTYPE (*GetCodecConfig)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_INOUT OMX_PTR pComponentConfigStructure);

    OMX_ERRORTYPE (*SetCodecConfig)(
                    OMX_IN  MagOmxComponent hComponent,
                    OMX_IN  OMX_INDEXTYPE nIndex, 
                    OMX_IN  OMX_PTR pComponentConfigStructure);
EndOfVirtuals;

ClassMembers(MagOmxComponent_VDec_H264, MagOmxComponent_VideoDecoder
)

EndOfClassMembers;

#endif
