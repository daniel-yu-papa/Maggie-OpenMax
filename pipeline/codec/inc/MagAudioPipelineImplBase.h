/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __MAGPLAYER_AUDIO_PIPELINE_IMPLBASE_H__
#define __MAGPLAYER_AUDIO_PIPELINE_IMPLBASE_H__

#include "framework/MagFramework.h"
#include "MagStreamTrackManager.h"

class MagAudioPipelineImplBase{
public:
    enum{
        MagAudioPipeline_EmptyThisBuffer,
        MagAudioPipeline_Destroy,
    };

    enum{
        ST_INIT,
        ST_PLAY,
        ST_PAUSE,
        ST_STOP
    };

    MagAudioPipelineImplBase(){};
    virtual ~MagAudioPipelineImplBase(){};

    virtual void setMagPlayerNotifier(MagMessageHandle notifyMsg) = 0;
    virtual MagMessageHandle getMagPlayerNotifier() = 0;
    virtual _status_t init(i32 trackID, TrackInfo_t *sInfo) = 0;
    virtual _status_t setup()  = 0;
    virtual _status_t start()  = 0;
    virtual _status_t stop()   = 0;
    virtual _status_t pause()  = 0;
    virtual _status_t resume() = 0;
    virtual _status_t flush()  = 0;
    virtual _status_t reset()  = 0;
    virtual _status_t getClkConnectedComp(i32 *port, void **ppComp) = 0;
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume) = 0;
    virtual _status_t getDecodedFrame(void **ppAudioFrame) = 0;
    virtual _status_t putUsedFrame(void *pAudioFrame) = 0;
};

#endif