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

#ifndef __MAG_MEDIA_PLAYER_H__
#define __MAG_MEDIA_PLAYER_H__

/*
*Event related definitions
*/
typedef enum mmp_event_t {
    MMP_EVENT_ERROR = 0,            //any errors
    MMP_EVENT_PLAYBACK_COMPLETE,    //playback is complete
    MMP_EVENT_SEEK_COMPLETE,        //seek completes
    MMP_EVENT_PREPARE_COMPLETE,     //prepare completes
    MMP_EVENT_BUFFER_STATUS,        //buffer status report. param1: buffering percentage, param2: buffering time in ms
    MMP_EVENT_FLUSH_COMPLETE,       //To flush the player is complete
    MMP_EVENT_NEW_STREAM_REPORT,    //To report out the media stream. param1: stream id for retrieve the detail info
    MMP_EVENT_MAX
}mmp_event_t;

typedef void (*mmp_event_callback_t)(mmp_event_t evt, void *handler, unsigned int param1, unsigned int param2);

/*
* Invoke related definitions
*/
typedef enum {
    MMP_STREAM_TYPE_VIDEO,
    MMP_STREAM_TYPE_AUDIO,
    MMP_STREAM_TYPE_SUBTITLE
}mmp_stream_type_t;

typedef struct{
    unsigned int width;
    unsigned int height;
    unsigned int fps;
    unsigned int bps;     /*in kb/s*/
    char codec[128];
}VideoMetaData_t;

typedef struct{
    unsigned int hz;
    unsigned int bps;
    char codec[128];
}AudioMetaData_t;

typedef struct{
    mmp_stream_type_t type;
    union{
        VideoMetaData_t vMetaData;
        AudioMetaData_t aMetaData;
    }metadata;
    int stream_id;
}mmp_meta_data_t;

typedef enum mmp_invoke_t{
    MMP_INVOKE_GET_NETWORK_BANDWIDTH,         /*int kb/s*/
    MMP_INVOKE_READ_STREAM_META_DATA,         /*mmp_meta_data_t*/
    MMP_INVOKE_SET_AUDIO_TRACK,               /*int audio_track_id*/ 
    MMP_INVOKE_SET_SUBTITLE_TRACK,            /*int subtitle_track_id*/ 
    MMP_INVOKE_GET_DECODED_VIDEO_FRAME,
    MMP_INVOKE_PUT_USED_VIDEO_FRAME
}mmp_invoke_t;

/*
*Parameter related definitions
*/
typedef struct mmp_buffer_setting_t {
    int buffer_type;        /*1: frame buffer,  0: byte buffer, -1: no buffer*/
    int buffer_size;        /*in bytes. -1: no limited*/
    int buffer_time;        /*in ms.    -1: not used for byte buffer*/
    int buffer_high_level;  /*in percentage*/
    int buffer_low_level;   /*in percentage*/
    int buffer_play_level;  /*in percentage*/
}mmp_buffer_setting_t;

typedef enum mmp_parameter_t{
    MMP_PARAMETER_BUFFER_SETTING,             /*mmp_buffer_setting_t*/
    MMP_PARAMETER_AVSYNC_ENABLE,              /*int. 1: enable avsync, 0: disable avsync*/
    MMP_PARAMETER_RETURN_VIDEO_FRAME,         /*int. 1: return back the decoded video frame. 0: directly display*/
    MMP_PARAMETER_SUBTITLE_FILE_PATH,         /*char* */
    MMP_PARAMETER_LOOPING_PLAYBACK            /*int. 1: enable, 0: disable */
}mmp_parameter_t;

/*
*Base class definition
*/
class MagMediaPlayer{
public:
    MagMediaPlayer(){};
    virtual ~MagMediaPlayer(){};

    virtual int        setDataSource(const char *url) = 0;
    virtual int        setDataSource(unsigned int fd, signed long long offset, signed long long length) = 0;
    virtual int        prepare() = 0;
    virtual int        prepareAsync() = 0;
    virtual int        start() = 0;
    virtual int        stop() = 0;
    virtual int        pause() = 0;
    virtual bool       isPlaying() = 0;
    virtual int        seekTo(int msec) = 0;
    virtual int        flush() = 0;
    virtual int        fast(int speed) = 0;
    virtual int        getCurrentPosition(int* msec) = 0;
    virtual int        getDuration(int* msec) = 0;
    virtual int        reset() = 0;
    virtual int        setVolume(float leftVolume, float rightVolume) = 0;
    virtual int        setParameter(int key, void *request) = 0;
    virtual int        getParameter(int key, void **reply) = 0;
    virtual int        invoke(const unsigned int methodID, void *request, void **reply) = 0;
    virtual int        registerEventCallback(mmp_event_callback_t cb, void *handler) = 0;
};

MagMediaPlayer* GetMediaPlayer();

#endif