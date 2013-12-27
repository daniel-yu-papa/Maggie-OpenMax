#include "OMXComponent_Demux.h"

#define COMPONENT_NAME "OMX.Mag.ffmpeg.demux"
#define ROLE_NAME "container_demuxer.all"

static OMX_ERRORTYPE ComponentInit(OMX_HANDLETYPE * hComponent, const OMX_STRING name){

}

Maggie_OMX_CompRegistration_t *OMX_ComponentRegistration(void){
    static char * roles[] = {ROLE_NAME, NULL};
    static Maggie_OMX_CompRegistration_t comp_reg = {
        COMPONENT_NAME, roles, ComponentInit
    };
    static Maggie_OMX_CompRegistration_t * regs[] = {&comp_reg};

    return regs;
}

