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

#ifndef __MAG_PLAYER_DRIVER_IMPLE_H__
#define __MAG_PLAYER_DRIVER_IMPLE_H__

#include "MagPlayer.h"
#include "MagPlayerDriverImplBase.h"

class MagPlayerDriverImpl : public MagPlayerDriverImplBase{
public:
    MagPlayerDriverImpl(void *client, notify_client_callback_f cb);
    virtual ~MagPlayerDriverImpl();
    
    virtual _status_t        setDataSource(const char *url);
    virtual _status_t        setDataSource(ui32 fd, i64 offset, i64 length);
    virtual _status_t        prepare();
    virtual _status_t        prepareAsync();
    virtual _status_t        start();
    virtual _status_t        stop();
    virtual _status_t        pause();
    virtual bool             isPlaying();
    virtual _status_t        seekTo(int msec);
    virtual _status_t        flush();
    virtual _status_t        fast(int speed);
    virtual _status_t        getCurrentPosition(int* msec);
    virtual _status_t        getDuration(int* msec);
    virtual _status_t        reset();
    virtual _status_t        setVolume(float leftVolume, float rightVolume);
    virtual _status_t        setParameter(int key, void *request);
    virtual _status_t        getParameter(int key, void **reply);
    virtual _status_t        invoke(const ui32 methodID, void *request, void **reply);
    virtual ui32             getVersion();
    
    static void PrepareCompleteEvtListener(void *priv);
    static void SeekCompleteEvtListener(void *priv);
    static void FlushCompleteEvtListener(void *priv);
    static void ErrorEvtListener(void *priv, i32 what, i32 extra);
    static void InfoEvtListener(void *priv, i32 what, i32 extra);
    
    enum State_t{
        MPD_ST_ERROR              = 0,
        MPD_ST_IDLE               = 1 << 0,
        MPD_ST_INITIALIZED        = 1 << 1,
    };

    State_t mState;
    
private:
    void *mpClient;
    notify_client_callback_f mClientNotifyFn;

    MagPlayer *mpPlayer;
    
    void sendEvent(i32 msg, i32 ext1=0, i32 ext2=0);

    i32 mSetTrackIndex;
};


#endif