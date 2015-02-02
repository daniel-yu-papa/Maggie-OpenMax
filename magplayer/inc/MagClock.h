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

#ifndef __MAGPLAYER_CLOCK_H__
#define __MAGPLAYER_CLOCK_H__

#include "framework/MagFramework.h"
#include "MagPipelineFactory.h"
#include "MagClockImplBase.h"
 
class MagClock{
public:
    MagClock(Clock_Type_t type);
    virtual ~MagClock();

    virtual _status_t connectVideoPipeline(void *pVideoPipeline);
    virtual _status_t connectAudioPipeline(void *pAudioPipeline);
    virtual _status_t disconnectVideoPipeline(void *pVideoPipeline);
    virtual _status_t disconnectAudioPipeline(void *pAudioPipeline);
    virtual _status_t init();
    virtual _status_t setup();
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush();
    virtual _status_t reset();
    virtual i64       getPlayingTime();
    virtual i64       getMediaTime();
    
protected:
    MagClockImplBase *getClockImpl();

private:
    Clock_Type_t      mType;
    MagClockImplBase *mClock;
};

#endif