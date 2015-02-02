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

#ifndef __MAGPLAYER_CLOCK_IMPLBASE_H__
#define __MAGPLAYER_CLOCK_IMPLBASE_H__

#include "framework/MagFramework.h"


class MagClockImplBase{
public:
    MagClockImplBase(){};
    virtual ~MagClockImplBase(){};

    virtual _status_t connectVideoPipeline(void *pVpl) = 0;
    virtual _status_t connectAudioPipeline(void *pApl) = 0;
    virtual _status_t disconnectVideoPipeline(void *pVpl) = 0;
    virtual _status_t disconnectAudioPipeline(void *pApl) = 0;
    virtual _status_t init() = 0;
    virtual _status_t setup() = 0;
    virtual _status_t start() = 0;
    virtual _status_t stop() = 0;
    virtual _status_t pause() = 0;
    virtual _status_t resume() = 0;
    virtual _status_t flush() = 0;
    virtual _status_t reset() = 0;
    virtual i64       getPlayingTime() = 0;
    virtual i64       getMediaTime() = 0;
};

#endif