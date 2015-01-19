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

#define SEEK_ONCE_TIME (20 * 1000) /*in ms*/

//Screen dimension constants
const int DEFAULT_SCREEN_WIDTH  = 544;
const int DEFAULT_SCREEN_HEIGHT = 576;

static VideoMetaData_t *gpVideoMetaData = NULL;
static AudioMetaData_t *gpAudioMetaData = NULL;

static bool gStopped = true;
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
        }

            break;

        default:
            break;

    }
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
    float volume = 0.1;
    int win_width;
    int win_height;

    bool done = false;

    SDL_Event event;
    SDL_Surface *pWindow = NULL;
    SDL_Overlay *pVideoOverlay = NULL;
    TTF_Font *pFont = NULL;
    void *pVideoFrame = NULL;

    int flags = SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_HWACCEL;
    int is_full_screen = 0;
    SDL_Rect VideoDestRect;

	if (argc < 2){
		printf("usage: mmp_sdl --url "" --seek n\n");
        printf("               --url: the playback link\n");
        printf("               --seek: the milliseconds value that once seek action moves forward or backward\n");
        printf("        --full-screen: full screen (defalut: resizable)\n");
		return -1;
	}

    for (i = 1; i < argc;){
        if (!strcasecmp(argv[i], "--url")){
            url = (char *)malloc(strlen(argv[i + 1]) + 1);
            strcpy(url, argv[i + 1]);
        }else if (!strcasecmp(argv[i], "--seek")){
            once_seek_time = atoi(argv[i + 1]) * 1000;
        }else if (!strcasecmp(argv[i], "--full-screen")){
            is_full_screen = 1;
        }
        i = i + 2;
    }

    if (is_full_screen) flags |= SDL_FULLSCREEN;
    else                flags |= SDL_RESIZABLE;

    printf("mmp_sdl plays url: %s, seek time: %d, full screen: %s\n\n\n", url, once_seek_time, is_full_screen ? "Y" : "N");
    
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
    }else{
    	ret = player->prepareAsync();
    	if (ret != MMP_OK){
    		AGILE_LOGE("failed to do prepareAsync()\n");
    		return -1;
    	}
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
    
    if (gpVideoMetaData){
        win_width  = gpVideoMetaData->width;
        win_height = gpVideoMetaData->height;
    }else{
        win_width  = DEFAULT_SCREEN_WIDTH;
        win_height = DEFAULT_SCREEN_HEIGHT;
    }

    AGILE_LOGD("create the window: w:%d -- h:%d", win_width, win_height);

    //Create window
    pWindow = SDL_SetVideoMode(win_width, win_height, 0, flags);
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
                        /*seek forward*/
                        seekTime += once_seek_time;
                        player->seekTo(seekTime);
                        break;
                    }else if (event.key.keysym.sym == SDLK_LEFT){
                        printf("key down: left arrow\n");
                        /*seek backward*/
                        seekTime -= once_seek_time;
                        if (seekTime < 0)
                            seekTime = 0;
                        player->seekTo(seekTime);
                        break;
                    }else if (event.key.keysym.sym == SDLK_UP){
                        printf("key down: up arrow\n");
                        /*volume up*/
                        volume += 0.1;
                        if (volume > 1.0)
                            volume = 1.0;
                        player->setVolume(volume, volume);
                        break;
                    }else if (event.key.keysym.sym == SDLK_DOWN){
                        printf("key down: down arrow\n");
                        /*volume down*/
                        volume -= 0.1;
                        if (volume < 0.0)
                            volume = 0.0;
                        player->setVolume(volume, volume);
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
                            player->start();
                            gStopped = !gStopped;
                        }
                        break;
                    }else if (event.key.keysym.sym == SDLK_f){
                        printf("key down: f\n");
                        /*flush*/
                        /*if the initial state is playing, do flush() and then resume(), the playing continues
                        * if the initial state is pause, do flush() and then resume(), keep pause state
                        */
                        player->flush();
                        break;
                    }else if (event.key.keysym.sym == SDLK_r){
                        printf("key down: r\n");
                        /*reset and play*/
                        player->reset();

                        player->setDataSource(url);
                        player->prepareAsync();
                        player->start();
                        break;
                    }else if (event.key.keysym.sym != SDLK_ESCAPE) {
                        break;
                    }

                case SDL_QUIT:
                    printf("kill the window\n");
                    gStopped = !gStopped;
                    /*quit the playback*/
                    free(url);
                    printf("kill the window 1111\n");
                    delete player;
                    printf("kill the window 2222\n");
                    done = true;
                    printf("kill the window 3333\n");
                    break;
            }
        }

        if (!gStopped){
            if (player->invoke(INVOKE_ID_GET_DECODED_VIDEO_FRAME, NULL, &pVideoFrame) == MAG_NO_ERROR){
                OMX_BUFFERHEADERTYPE *buf;
                AVFrame *frame;

                buf = static_cast<OMX_BUFFERHEADERTYPE *>(pVideoFrame);
                frame = (AVFrame *)buf->pBuffer;

                if (pVideoOverlay == NULL){
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

                UpdateVideoOverlay(pVideoOverlay, frame);
                player->invoke(INVOKE_ID_PUT_USED_VIDEO_FRAME, pVideoFrame, NULL);
                Mag_TimeTakenStatistic(MAG_TRUE, __FUNCTION__, NULL);
                SDL_DisplayYUVOverlay(pVideoOverlay, &VideoDestRect);
                Mag_TimeTakenStatistic(MAG_FALSE, __FUNCTION__, "SDL_DisplayYUVOverlay");
            }
        }
    }

error:
    TTF_CloseFont( pFont );

    //Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();

    printf("Exit the mmp_sdl!!\n");
}