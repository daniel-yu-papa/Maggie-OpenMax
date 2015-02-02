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

#ifndef __MAGPLAYER_DRIVER_IMPL_BASE_H__
#define __MAGPLAYER_DRIVER_IMPL_BASE_H__

#include "framework/MagFramework.h"

typedef void (*notify_client_callback_f)(void* cookie, int msg, int ext1, int ext2);

class MagPlayerDriverImplBase{
public:
    MagPlayerDriverImplBase(){};
    virtual ~MagPlayerDriverImplBase(){};
    
    virtual _status_t        setDataSource(const char *url) = 0;
    virtual _status_t        setDataSource(ui32 fd, i64 offset, i64 length) = 0;
    virtual _status_t        prepare() = 0;
    virtual _status_t        prepareAsync() = 0;
    virtual _status_t        start() = 0;
    virtual _status_t        stop() = 0;
    virtual _status_t        pause() = 0;
    virtual bool             isPlaying() = 0;
    virtual _status_t        seekTo(int msec) = 0;
    virtual _status_t        flush() = 0;
    virtual _status_t        fast(int speed) = 0;
    virtual _status_t        getCurrentPosition(int* msec) = 0;
    virtual _status_t        getDuration(int* msec) = 0;
    virtual _status_t        reset() = 0;
    virtual _status_t        setVolume(float leftVolume, float rightVolume) = 0;
    virtual _status_t        setParameter(int key, void *request) = 0;
    virtual _status_t        getParameter(int key, void **reply) = 0;
    virtual _status_t        invoke(const ui32 methodID, void *request, void **reply) = 0;
    virtual ui32             getVersion() = 0;
};

#endif