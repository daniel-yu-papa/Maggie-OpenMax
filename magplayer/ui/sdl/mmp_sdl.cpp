#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

extern "C" {
#include "libavformat/avformat.h"
}

#include "SDL.h"
#include "SDL_ttf.h"

#include "framework/MagFramework.h"
#include "mmp.h"
#include "MagInvokeDef.h"
#include "OMX_Core.h"

#define SEEK_ONCE_TIME (10 * 1000) /*in ms*/
const int black_Y = 0;
const int black_U = 128;
const int black_V = 128;

//Screen dimension constants
const int DEFAULT_SCREEN_WIDTH  = 640;
const int DEFAULT_SCREEN_HEIGHT = 480;

static VideoMetaData_t *gpVideoMetaData = NULL;
static AudioMetaData_t *gpAudioMetaData = NULL;

static bool gStopped = true;

static int gSet_win_width = 0;
static int gSet_win_height = 0;

MagEventHandle         gPrepareCompleteEvent;
MagEventGroupHandle    gPrepareEventGroup;

MagEventHandle         gFlushDoneEvent;
MagEventGroupHandle    gFlushEventGroup;

MagEventHandle         gSeekCompleteEvent;
MagEventGroupHandle    gSeekEventGroup;

/*static void *monitorThreadProc(void *arg){
    BufferStatistic_t bufStat;

    MagMediaPlayer *player = (MagMediaPlayer *)arg;

    while(monitorFlag){
        player->invoke(INVOKE_ID_GET_BUFFER_STATUS, NULL, &bufStat);
        printf("audio buffer: %d ms, video buffer: %d ms, loadingSpeed %d Bytes/Sec/\n", 
                bufStat.audio_buffer_time, bufStat.video_buffer_time, bufStat.loadingSpeed);

        sleep(1);
    }

    printf("exit monitorThreadProc!\n");
    return NULL;
}*/

static void eventCallback(mmp_event_t evt, void *handler, unsigned int param1, unsigned int param2){
    MagMediaPlayer *pMplayer = static_cast<MagMediaPlayer *>(handler);

    switch (evt){
        case MMP_PLAYER_EVT_PLAYBACK_ERROR:

            break;

        case MMP_PLAYER_EVT_PLAYBACK_COMPLETE:
            gStopped = true;
            break;

        case MMP_PLAYER_EVT_BUFFER_STATUS:

            break;

        case MMP_PLAYER_EVT_SEEK_COMPLETE:
            Mag_SetEvent(gSeekCompleteEvent);
            break;

        case MMP_PLAYER_EVT_FLUSH_DONE:
            Mag_SetEvent(gFlushDoneEvent);
            break;

        case MMP_PLAYER_EVT_PREPARE_COMPLETE:
        {
            void *p;
            printf("get event: PREPARE_COMPLETE\n");
            if (gpVideoMetaData == NULL){
                gpVideoMetaData = (VideoMetaData_t *)mag_mallocz(sizeof(VideoMetaData_t));
            }

            if (gpAudioMetaData == NULL){
                gpAudioMetaData = (AudioMetaData_t *)mag_mallocz(sizeof(AudioMetaData_t));
            }

            p = static_cast<void *>(gpVideoMetaData);
            pMplayer->invoke(INVOKE_ID_GET_VIDEO_META_DATA, NULL, &p);

            p = static_cast<void *>(gpAudioMetaData);
            pMplayer->invoke(INVOKE_ID_GET_AUDIO_META_DATA, NULL, &p);

            AGILE_LOGD("video: w[%d], h[%d], fps[%f], bps[%d], codec[%s]",
                        gpVideoMetaData->width,
                        gpVideoMetaData->height,
                        gpVideoMetaData->fps,
                        gpVideoMetaData->bps,
                        gpVideoMetaData->codec);

            AGILE_LOGD("audio: hz[%d], bps[%d], codec[%s]",
                        gpAudioMetaData->hz,
                        gpAudioMetaData->bps,
                        gpAudioMetaData->codec);

            gSet_win_width  = gpVideoMetaData->width;
            gSet_win_height = gpVideoMetaData->height;

            Mag_SetEvent(gPrepareCompleteEvent);
        }

            break;

        default:
            break;

    }
}

static void SetVideoOverlayBlack(SDL_Overlay *pVideoOverlay, int width, int height)
{
    int i;

    /* get a pointer on the bitmap */
    SDL_LockYUVOverlay (pVideoOverlay);

    for (i = 0; i < width * height; i++){
        *(pVideoOverlay->pixels[0] + i) = black_Y;
    }

    for (i = 0; i < width * height / 4; i++){
        *(pVideoOverlay->pixels[2] + i) = black_U;
        *(pVideoOverlay->pixels[1] + i) = black_V;
    }

    pVideoOverlay->pitches[0] = width;
    pVideoOverlay->pitches[2] = width / 2;
    pVideoOverlay->pitches[1] = width / 2;

    /* update the bitmap content */
    SDL_UnlockYUVOverlay(pVideoOverlay);

}

static void UpdateVideoOverlay(SDL_Overlay *pVideoOverlay, AVFrame *frame)
{
    AVPicture pict = { { 0 } };

    /* get a pointer on the bitmap */
    SDL_LockYUVOverlay (pVideoOverlay);

    pict.data[0] = pVideoOverlay->pixels[0];
    pict.data[1] = pVideoOverlay->pixels[2];
    pict.data[2] = pVideoOverlay->pixels[1];

    pict.linesize[0] = pVideoOverlay->pitches[0];
    pict.linesize[1] = pVideoOverlay->pitches[2];
    pict.linesize[2] = pVideoOverlay->pitches[1];

    // FIXME use direct rendering
    av_picture_copy(&pict, (AVPicture *)frame,
                    (enum AVPixelFormat)frame->format, frame->width, frame->height);

    /* update the bitmap content */
    SDL_UnlockYUVOverlay(pVideoOverlay);

}

int main(int argc, char *argv[]){
	char *url = NULL;
	MagMediaPlayer *player = NULL;
    int ret;
    bool paused = false;
    int i;
    int once_seek_time = SEEK_ONCE_TIME;
    int seekTime = 0;
    
    int display_win_width;
    int display_win_height;
    bool done = false;

    SDL_Event event;
    SDL_Surface *pWindow = NULL;
    SDL_Overlay *pVideoOverlay = NULL;
    TTF_Font *pFont = NULL;
    void *pVideoFrame = NULL;

    int flags = SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_HWACCEL | SDL_RESIZABLE;
    SDL_Rect VideoDestRect;

	if (argc < 2){
		printf("usage: mmp_sdl --url "" --seek n\n");
        printf("               --url: the playback link\n");
        printf("               --seek: the milliseconds value that once seek action moves forward or backward\n");
		return -1;
	}

    for (i = 1; i < argc;){
        if (!strcasecmp(argv[i], "--url")){
            url = (char *)malloc(strlen(argv[i + 1]) + 1);
            strcpy(url, argv[i + 1]);
        }else if (!strcasecmp(argv[i], "--seek")){
            once_seek_time = atoi(argv[i + 1]) * 1000;
        }
        i = i + 2;
    }

    Mag_CreateEventGroup(&gPrepareEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&gPrepareCompleteEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(gPrepareEventGroup, gPrepareCompleteEvent);
    }
    
    Mag_CreateEventGroup(&gFlushEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&gFlushDoneEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(gFlushEventGroup, gFlushDoneEvent);
    }

    Mag_CreateEventGroup(&gSeekEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&gSeekCompleteEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(gSeekEventGroup, gSeekCompleteEvent);
    }

    printf("mmp_sdl plays url: %s, seek time: %d s\n\n\n", url, once_seek_time/1000);
    
    player = GetMediaPlayer();
    if (player == NULL){
    	AGILE_LOGE("failed to create the player!\n");
    	return -1;
    }

    player->registerEventCallback(eventCallback, player);
    
    ret = player->setDataSource(url);
    if (ret != MMP_OK){
    	AGILE_LOGE("failed to do setDataSource(%s)\n", url);
    	return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        AGILE_LOGE("Couldn't initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    if( TTF_Init() == -1 ){
        AGILE_LOGE( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());                  
        return -1;                
    }

    //Create the font
    pFont = TTF_OpenFont( "FreeSerif.ttf", 28 );

    if( pFont == NULL )
    {
        AGILE_LOGE( "Font could not be created! TTF Error: %s\n", TTF_GetError() );
        return -1;
    }
    
    gSet_win_width      = DEFAULT_SCREEN_WIDTH;
    gSet_win_height     = DEFAULT_SCREEN_HEIGHT;
    display_win_width   = gSet_win_width;
    display_win_height  = gSet_win_height;

    //Create default window
    pWindow = SDL_SetVideoMode(display_win_width, display_win_height, 0, flags);
    if( pWindow == NULL ){
        AGILE_LOGE( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
        return -1;
    }
    
    /* Set the window manager title bar */
    SDL_WM_SetCaption("Mag Media Player", url);

    while (!done){
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_VIDEORESIZE:
                    gSet_win_width  = event.resize.w;
                    gSet_win_height = event.resize.h;
                    printf("resize the window(w: %d, h:%d)\n", gSet_win_width, gSet_win_height);
                    break;
                
                case SDL_KEYDOWN:
                    printf("key down: %d\n", event.key.keysym.sym);
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        printf("key down: space\n");
                        if (paused){
                            player->start();
                        }else{
                            player->pause();
                        }
                        paused = !paused;
                        break;
                    }else if (event.key.keysym.sym == SDLK_RIGHT){
                        printf("key down: right arrow\n");
                        Mag_ClearEvent(gSeekCompleteEvent);
                        /*seek forward*/
                        seekTime = once_seek_time;
                        player->seekTo(seekTime);

                        Mag_WaitForEventGroup(gSeekEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                        break;
                    }else if (event.key.keysym.sym == SDLK_LEFT){
                        printf("key down: left arrow\n");
                        Mag_ClearEvent(gSeekCompleteEvent);
                        /*seek backward*/
                        seekTime =(0 - once_seek_time);
                        player->seekTo(seekTime);

                        Mag_WaitForEventGroup(gSeekEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                        break;
                    }else if (event.key.keysym.sym == SDLK_s){
                        printf("key down: s\n");
                        /*stop*/
                        if (!gStopped){
                            player->stop();
                            gStopped = !gStopped;
                        }
                        break;
                    }else if (event.key.keysym.sym == SDLK_p){
                        printf("key down: p\n");
                        /*play*/
                        if (gStopped){
                            ret = player->prepareAsync();
                            if (ret != MMP_OK){
                                AGILE_LOGE("failed to do prepareAsync()\n");
                                return -1;
                            }
                            player->start();
                            gStopped = !gStopped;
                        }
                        break;
                    }else if (event.key.keysym.sym == SDLK_f){
                        printf("key down: f\n");
                        Mag_ClearEvent(gFlushDoneEvent);
                        /*flush*/
                        /*if the initial state is playing, do flush() and then resume(), the playing continues
                        * if the initial state is pause, do flush() and then resume(), keep pause state
                        */
                        player->flush();

                        Mag_WaitForEventGroup(gFlushEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                        break;
                    }else if (event.key.keysym.sym == SDLK_r){
                        printf("key down: r\n");

                        Mag_ClearEvent(gPrepareCompleteEvent);
                        /*reset and play*/
                        player->reset();

                        player->setDataSource(url);
                        player->prepareAsync();
                        player->start();

                        Mag_WaitForEventGroup(gPrepareEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                        break;
                    }else if (event.key.keysym.sym != SDLK_ESCAPE) {
                        break;
                    }

                case SDL_QUIT:
                    printf("kill the window\n");
                    gStopped = !gStopped;
                    /*quit the playback*/
                    free(url);
                    printf("before killing the player\n");
                    delete player;
                    done = true;
                    printf("after killing the player\n");
                    goto error;
            }
        }

        if (!gStopped && !paused){
            if (player->invoke(INVOKE_ID_GET_DECODED_VIDEO_FRAME, NULL, &pVideoFrame) == MAG_NO_ERROR){
                OMX_BUFFERHEADERTYPE *buf;
                AVFrame *frame;

                buf = static_cast<OMX_BUFFERHEADERTYPE *>(pVideoFrame);
                frame = (AVFrame *)buf->pBuffer;

                if (gSet_win_width != display_win_width || gSet_win_height != display_win_height){
                    pWindow = SDL_SetVideoMode(gSet_win_width, gSet_win_width, 0, flags);
                    if( pWindow == NULL ){
                        AGILE_LOGE( "New Window could not be created! SDL Error: %s\n", SDL_GetError() );
                    }                    
                }

                if (pVideoOverlay == NULL ||
                    gSet_win_width != display_win_width || 
                    gSet_win_height != display_win_height){

                    if (pVideoOverlay){
                        SDL_FreeYUVOverlay(pVideoOverlay);
                    }

                    /* Create the overlay */
                    pVideoOverlay = SDL_CreateYUVOverlay(gpVideoMetaData->width, gpVideoMetaData->height, SDL_YV12_OVERLAY, pWindow);
                    if ( pVideoOverlay == NULL ) {
                        AGILE_LOGE("Couldn't create overlay: %s\n", SDL_GetError());
                        return -1;
                    }else{
                        AGILE_LOGD("Succeed to create the overlay: %dx%dx%d %s %s overlay\n",
                                    pVideoOverlay->w,pVideoOverlay->h,pVideoOverlay->planes,
                                    pVideoOverlay->hw_overlay?"hardware":"software",
                                    pVideoOverlay->format==SDL_YV12_OVERLAY?"YV12":
                                    pVideoOverlay->format==SDL_IYUV_OVERLAY?"IYUV":
                                    pVideoOverlay->format==SDL_YUY2_OVERLAY?"YUY2":
                                    pVideoOverlay->format==SDL_UYVY_OVERLAY?"UYVY":
                                    pVideoOverlay->format==SDL_YVYU_OVERLAY?"YVYU":
                                    "Unknown");
                        for(i = 0; i < pVideoOverlay->planes; i++){
                            AGILE_LOGD("  plane %d: pitch=%d\n", i, pVideoOverlay->pitches[i]);
                        }
                    }

                    VideoDestRect.w = pVideoOverlay->w;
                    VideoDestRect.h = pVideoOverlay->h;
                    VideoDestRect.x = 0;
                    VideoDestRect.y = 0;
                }

                display_win_width = gSet_win_width;
                display_win_height = gSet_win_height;

                UpdateVideoOverlay(pVideoOverlay, frame);
                player->invoke(INVOKE_ID_PUT_USED_VIDEO_FRAME, pVideoFrame, NULL);
                Mag_TimeTakenStatistic(MAG_TRUE, __FUNCTION__, NULL);
                SDL_DisplayYUVOverlay(pVideoOverlay, &VideoDestRect);
                Mag_TimeTakenStatistic(MAG_FALSE, __FUNCTION__, "SDL_DisplayYUVOverlay");
            }
        }else if (gStopped){
            if (pVideoOverlay){
                SetVideoOverlayBlack(pVideoOverlay, display_win_width, display_win_height);
                SDL_DisplayYUVOverlay(pVideoOverlay, &VideoDestRect);
            }
        }
    }

error:
    Mag_DestroyEvent(&gPrepareCompleteEvent);
    Mag_DestroyEventGroup(&gPrepareEventGroup);

    Mag_DestroyEvent(&gFlushDoneEvent);
    Mag_DestroyEventGroup(&gFlushEventGroup);

    Mag_DestroyEvent(&gSeekCompleteEvent);
    Mag_DestroyEventGroup(&gSeekEventGroup);

    TTF_CloseFont( pFont );
    //Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();

    printf("Exit the mmp_sdl!!\n");
}