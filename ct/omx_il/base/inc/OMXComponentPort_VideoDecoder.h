#ifndef __OMX_COMPONENT_PORT_VIDEO_DECODER_H__
#define __OMX_COMPONENT_PORT_VIDEO_DECODER_H__

#include "OMX_ClassMagic.h"
#include "OMXComponentPort_base.h"

#define DEFAULT_VIDEO_DECODER_BUF_SIZE   0

#define DEFAULT_VIDEO_MIME_STRING_LENGTH 128

DERIVEDCLASS(OMXComponentPort_VideoDecoder_t, OMXComponentPort_base_t)
#define OMXComponentPort_VideoDecoder_t_FIELDS \
    OMXComponentPort_base_t_FIELDS
    
ENDCLASS(OMXComponentPort_VideoDecoder_t)

#endif