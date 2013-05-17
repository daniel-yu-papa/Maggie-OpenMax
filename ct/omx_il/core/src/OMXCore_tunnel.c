#include "OMX_Core.h"

OMX_ERRORTYPE OMX_SetupTunnel (
    OMX_HANDLETYPE hOutput,
    OMX_U32 nPortOutput,
    OMX_HANDLETYPE hInput,
    OMX_U32 nPortInput){

    OMX_TUNNELSETUPTYPE tunnel_setup;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE * in;
    OMX_COMPONENTTYPE * out;
    
    if ((NULL == hOutput) || (NULL == hInput)){
        return OMX_ErrorBadParameter;
    }

    in  = (OMX_COMPONENTTYPE *)hInput;
    out = (OMX_COMPONENTTYPE *)hOutput;

    tunnel_setup.nTunnelFlags = 0;
    tunnel_setup.eSupplier    = OMX_BufferSupplyUnspecified;

    ret = out->ComponentTunnelRequest(out, nPortOutput, in, nPortInput, &tunnel_setup);

    if (OMX_ErrorNone == ret){
        ret = in->ComponentTunnelRequest(in, nPortInput, out, nPortOutput, &tunnel_setup);

        if (OMX_ErrorNone != ret){
            AGILE_LOGE("unable to setup tunnel on Input port[h-0x%x : p-%d]",
                        in, nPortInput);
            ret = out->ComponentTunnelRequest(out, nPortOutput, NULL, 0, NULL);
        }
    }else{
        AGILE_LOGE("unable to setup tunnel between Input port[h-0x%x : p-%d] to Output port[h-0x%x : p-%d]",
                    in, nPortInput, out, nPortOutput);
    }

    return ret;
}

