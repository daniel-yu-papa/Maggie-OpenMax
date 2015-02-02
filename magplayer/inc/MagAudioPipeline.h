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

#ifndef __MAGPLAYER_AUDIO_PIPELINE_H__
#define __MAGPLAYER_AUDIO_PIPELINE_H__

#include "framework/MagFramework.h"
#include "MagPipelineFactory.h"
#include "MagAudioPipelineImplBase.h"
 
class MagAudioPipeline{
public:
    MagAudioPipeline(Pipeline_Type_t type);
    virtual ~MagAudioPipeline();

    virtual void setMagPlayerNotifier(MagMessageHandle notifyMsg);
    virtual MagMessageHandle getMagPlayerNotifier();
    virtual _status_t init(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t setup();
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush();
    virtual _status_t reset();
    virtual _status_t getClkConnectedComp(i32 *port, void **ppComp);
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume);
    virtual _status_t getDecodedFrame(void **ppAudioFrame);
    virtual _status_t putUsedFrame(void *pAudioFrame);
    
protected:
    MagAudioPipelineImplBase *getPipelineImpl();

private:
    MagAudioPipelineImplBase *mPipeline;
    Pipeline_Type_t mType;
};

#endif