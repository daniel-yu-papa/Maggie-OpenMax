#ifndef __OMX_COMPONENT_PORT_CONTAINER_DEMUXER_H__
#define __OMX_COMPONENT_PORT_CONTAINER_DEMUXER_H__

#include "OMX_ClassMagic.h"
#include "OMXComponentPort_base.h"

typedef enum{
    DEMUX_OUTPUT_PORT_AUDIO,
    DEMUX_OUTPUT_PORT_VIDEO,
    DEMUX_OUTPUT_PORT_SUBTITLE
}demuxerOutputPortType_t;

typedef struct{
    List_t node;
}demuxerAudioStreamInfo_t;

typedef struct{
    List_t node;
}demuxerVideoStreamInfo_t;

typedef struct{
    List_t node;
}demuxerSubtitleStreamInfo_t;


typedef struct{
    List_t streamListHead;
    OMX_U8 streamNumber;
    OMX_HANDLETYPE playingStreamHandle;
}demuxerStreamInfo_t;

DERIVEDCLASS(OMXComponentPort_ContainerDemuxer_t, OMXComponentPort_base_t)
#define OMXComponentPort_ContainerDemuxer_t_FIELDS \
    OMXComponentPort_base_t_FIELDS \
    List_t filledBufListHead; /*organized as FIFO*/ \
    demuxerOutputPortType_t type; \
    demuxerStreamInfo_t info;
ENDCLASS(OMXComponentPort_ContainerDemuxer_t)

#endif

