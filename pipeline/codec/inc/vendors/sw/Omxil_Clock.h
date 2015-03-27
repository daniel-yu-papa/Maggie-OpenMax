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

#ifndef __OMXIL_CLOCK_H__
#define __OMXIL_CLOCK_H__

#include "MagClockImpl.h"
#include "framework/MagFramework.h"

class OmxilClock : public MagClockImpl{
public:
	OmxilClock();
    virtual ~OmxilClock();

    virtual _status_t connectVideoPipeline(void *pVpl);
    virtual _status_t connectAudioPipeline(void *pApl);
    virtual _status_t disconnectVideoPipeline(void *pVpl);
    virtual _status_t disconnectAudioPipeline(void *pApl);
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
    
private:
	OMX_HANDLETYPE   mhClock;

    OMX_CALLBACKTYPE mClockCallbacks;

    OMX_U32          mPortIdxToARen;
    OMX_U32          mPortIdxToVSch;

    OMX_TICKS        mStartTime;

    MagEventHandle         mClkStIdleEvent;
    MagEventGroupHandle    mStIdleEventGroup;

    MagEventHandle         mClkStLoadedEvent;
    MagEventGroupHandle    mStLoadedEventGroup;

    MagEventHandle         mClkStExecutingEvent;
    MagEventGroupHandle    mStExecutingEventGroup;

    MagEventHandle         mClkStPauseEvent;
    MagEventGroupHandle    mStPauseEventGroup;

    static OMX_ERRORTYPE ClockEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);
};

#endif