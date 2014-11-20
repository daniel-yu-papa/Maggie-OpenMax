#ifndef __MAG_PLAYER_COMMON_H__
#define __MAG_PLAYER_COMMON_H__

#include "OMX_Audio.h"
#include "OMX_AudioExt.h"
#include "OMX_Video.h"
#include "OMX_VideoExt.h"

#define STRINGIFY(x) case x: return #x

static inline const char *OmxCodec2String(ui32 omxCodec) {
    switch (omxCodec) {
        STRINGIFY(OMX_VIDEO_CodingAVC);
        STRINGIFY(OMX_VIDEO_CodingMPEG4);
        STRINGIFY(OMX_VIDEO_CodingMPEG2);
        STRINGIFY(OMX_AUDIO_CodingMP3);
        STRINGIFY(OMX_AUDIO_CodingMP2);
        STRINGIFY(OMX_AUDIO_CodingAAC);
        STRINGIFY(OMX_AUDIO_CodingAC3);
        STRINGIFY(OMX_AUDIO_CodingDDPlus);
        default: return "codec - unknown";
    }
}

#define ALIGNTO(value, alignment) (((value / alignment) + 1) * alignment)

enum {
    MAG_NO_MORE_DATA = (MAG_STATUS_EXTENSION + 1),
    MAG_PREPARE_FAILURE,
    MAG_READ_ABORT,
    MAG_DEMUXER_ABORT
};

#endif