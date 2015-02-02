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

#ifndef __MAG_PLAYER_DRIVER_H__
#define __MAG_PLAYER_DRIVER_H__

#include "MagPlayerDriverImplBase.h"

class MagPlayerDriver{
public:
    MagPlayerDriver(void *client, notify_client_callback_f cb);
    ~MagPlayerDriver();
    
    _status_t        setDataSource(const char *url);
    _status_t        setDataSource(unsigned int fd, long long offset, long long length);
    _status_t        prepare();
    _status_t        prepareAsync();
    _status_t        start();
    _status_t        stop();
    _status_t        pause();
    bool             isPlaying();
    _status_t        seekTo(int msec);
    _status_t        flush();
    _status_t        fast(int speed);
    _status_t        getCurrentPosition(int* msec);
    _status_t        getDuration(int* msec);
    _status_t        reset();
    _status_t        setVolume(float leftVolume, float rightVolume);
    _status_t        setParameter(int key, void *request);
    _status_t        getParameter(int key, void **reply);
    _status_t        invoke(const unsigned int methodID, void *request, void **reply);
    ui32             getVersion();

protected:
    MagPlayerDriverImplBase *getPlayerDriver();

private:
    void *mPrivData;
    notify_client_callback_f mEventCB;
    MagPlayerDriverImplBase *mPlayerDriver;

};


#endif