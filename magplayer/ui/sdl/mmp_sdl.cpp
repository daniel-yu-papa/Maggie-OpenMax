#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "SDL.h"
#include "SDL_ttf.h"

#include "mmp.h"
#include "MagInvokeDef.h"
#include "framework/MagFramework.h"
#include "libavutil/frame.h"
#include "OMX_Core.h"

#define SEEK_ONCE_TIME (20 * 1000) /*in ms*/

//Screen dimension constants
const int DEFAULT_SCREEN_WIDTH  = 640;
const int DEFAULT_SCREEN_HEIGHT = 480;

static VideoMetaData_t *gpVideoMetaData = NULL;
static AudioMetaData_t *gpAudioMetaData = NULL;

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

            break;

        case MMP_PLAYER_EVT_BUFFER_STATUS:

            break;

        case MMP_PLAYER_EVT_SEEK_COMPLETE:

            break;

        case MMP_PLAYER_EVT_PREPARE_COMPLETE:
        {
            printf("get event: PREPARE_COMPLETE\n");
            if (gpVideoMetaData == NULL){
                gpVideoMetaData = (VideoMetaData_t *)malloc(sizeof(VideoMetaData_t));
            }

            if (gpAudioMetaData == NULL){
                gpAudioMetaData = (AudioMetaData_t *)malloc(sizeof(AudioMetaData_t));
            }

            pMplayer->invoke(INVOKE_ID_GET_VIDEO_META_DATA, NULL, gpVideoMetaData);
            pMplayer->invoke(INVOKE_ID_GET_AUDIO_META_DATA, NULL, gpAudioMetaData);
        }

            break;

        default:
            break;

    }
}

static  SDL_Texture* getTextTexture( TTF_Font *font, 
                                     SDL_Renderer *renderer,
                                     char *pText, 
                                     SDL_Color textColor, 
                                     int *width, 
                                     int *height ){
    SDL_Texture* textTexture = NULL;
    //Render text surface
    SDL_Surface* textSurface;

    textSurface = TTF_RenderText_Solid( font, pText, textColor );
    if( textSurface != NULL )
    {
        //Create texture from surface pixels
        textTexture = SDL_CreateTextureFromSurface( renderer, textSurface );
        if( textTexture == NULL )
        {
            AGILE_LOGE( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            *width  = textSurface->w;
            *height = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    else
    {
        AGILE_LOGE( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }

    return textTexture;
}

static void renderTexture( SDL_Renderer *renderer, 
                           SDL_Texture *texture,
                           int x, int y, int w, int h ){
    //Set rendering space and render to screen
    SDL_Rect renderTarget = { x, y, w, h };
    SDL_Rect  *pTarget = NULL;

    if (w > 0 && h > 0){
        pTarget = &renderTarget;
    }
    SDL_RenderCopyEx(renderer, texture, NULL, pTarget, 0.0, NULL, SDL_FLIP_NONE);
}

static void UpdateVideoTexture(SDL_Texture *texture, AVFrame *frame)
{
    SDL_UpdateYUVTexture(texture, NULL, 
                         frame->data[0], frame->linesize[0], 
                         frame->data[1], frame->linesize[1],
                         frame->data[2], frame->linesize[2]);
#if 0
    SDL_Color *color;
    Uint8 *src;
    Uint32 *dst;
    int i;
    void *pixels;
    int pitch;
    int total_copied = 0;

    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0) {
        AGILE_LOGE("Couldn't lock texture: %s\n", SDL_GetError());
        return;
    }
    
    for (i = 0; i < frame->height; i++){
        memcpy((Uint8*)pixels + i * frame->width, 
               frame->data[0] + i * frame->linesize[0]),
               frame->width);
    }
    total_copied = (i - 1) * frame->width;

    for (i = 0; i < frame->height / 2; i++){
        memcpy((Uint8*)pixels + total_copied + i * (frame->width / 2), 
               frame->data[1] + i * frame->linesize[1]),
               frame->width / 2);
    }
    total_copied +=  (i - 1) * frame->width / 2;

    for (i = 0; i < frame->height / 2; i++){
        memcpy((Uint8*)pixels + total_copied + i * (frame->width / 2), 
               frame->data[2] + i * frame->linesize[2]),
               frame->width / 2);
    }

    SDL_UnlockTexture(texture);
#endif
}

int main(int argc, char *argv[]){
	char *url = NULL;
	MagMediaPlayer *player = NULL;
    int ret;
    SDL_Event event;
    bool paused = false;
    bool stopped = true;
    int i;
    int once_seek_time = SEEK_ONCE_TIME;
    int seekTime = 0;
    float volume = 0.1;
    int win_width;
    int win_height;

    bool done = false;

    SDL_Window *pWindow = NULL;
    SDL_Renderer *pRenderer = NULL;
    TTF_Font *pFont = NULL;
    SDL_Color textColor = { 255, 255, 255, 0xFF }; /*white color text*/
    SDL_Texture *VideoTexture = NULL;
    void *pVideoFrame = NULL;


	if (argc < 2){
		printf("usage: mmp_sdl --url "" --seek n\n");
        printf("               --url: the playback link\n");
        printf("               --seek: the milliseconds value that once seek action moves forward or backward\n");
		return -1;
	}

    for (i = 1; i < argc;){
        if (!strcasecmp(argv[i], "--url")){
            url = (char *)malloc(strlen(argv[i + 1]));
            strcpy(url, argv[i + 1]);
        }else if (!strcasecmp(argv[i], "--seek")){
            once_seek_time = atoi(argv[i + 1]) * 1000;
        }
        i = i + 2;
    }

    printf("mmp_sdl plays url: %s, seek time: %d\n\n\n", url, once_seek_time);

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

    //Set texture filtering to linear
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
    {
        AGILE_LOGW( "Warning: Linear texture filtering not enabled!" );
    }

    if (gpVideoMetaData){
        win_width  = gpVideoMetaData->width;
        win_height = gpVideoMetaData->height;
    }else{
        win_width  = DEFAULT_SCREEN_WIDTH;
        win_height = DEFAULT_SCREEN_HEIGHT;
    }

    //Create window
    pWindow = SDL_CreateWindow( "Mag Media Player", 
                                SDL_WINDOWPOS_UNDEFINED, 
                                SDL_WINDOWPOS_UNDEFINED, 
                                win_width, 
                                win_height, 
                                SDL_WINDOW_SHOWN );
    if( pWindow == NULL )
    {
        AGILE_LOGE( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
        return -1;
    }
    
    //Create vsynced renderer for window
    pRenderer = SDL_CreateRenderer( pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( pRenderer == NULL )
    {
        AGILE_LOGE( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return -1;
    }

    //Initialize renderer color to Black
    SDL_SetRenderDrawColor( pRenderer, 0x00, 0x00, 0x00, 0xFF );

    while (!done){
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    SDL_RenderSetViewport(pRenderer, NULL);
                    
                }
                break;
            
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_SPACE) {
                    if (paused){
                        player->start();
                    }else{
                        player->pause();
                    }
                    paused = !paused;
                    break;
                }else if (event.key.keysym.sym == SDLK_RIGHT){
                    /*seek forward*/
                    seekTime += once_seek_time;
                    player->seekTo(seekTime);
                    break;
                }else if (event.key.keysym.sym == SDLK_LEFT){
                    /*seek backward*/
                    seekTime -= once_seek_time;
                    if (seekTime < 0)
                        seekTime = 0;
                    player->seekTo(seekTime);
                    break;
                }else if (event.key.keysym.sym == SDLK_UP){
                    /*volume up*/
                    volume += 0.1;
                    if (volume > 1.0)
                        volume = 1.0;
                    player->setVolume(volume, volume);
                    break;
                }else if (event.key.keysym.sym == SDLK_DOWN){
                    /*volume down*/
                    volume -= 0.1;
                    if (volume < 0.0)
                        volume = 0.0;
                    player->setVolume(volume, volume);
                    break;
                }else if (event.key.keysym.sym == SDLK_s){
                    /*stop*/
                    if (!stopped){
                        player->stop();
                        stopped = !stopped;
                    }
                    break;
                }else if (event.key.keysym.sym == SDLK_p){
                    /*play*/
                    if (stopped){
                        player->start();
                        stopped = !stopped;
                    }
                    break;
                }else if (event.key.keysym.sym == SDLK_f){
                    /*flush*/
                    /*if the initial state is playing, do flush() and then resume(), the playing continues
                    * if the initial state is pause, do flush() and then resume(), keep pause state
                    */
                    player->flush();
                    break;
                }else if (event.key.keysym.sym == SDLK_r){
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
                /*quit the playback*/
                free(url);
                delete player;
                done = true;
                break;
            }
        }

        if (!stopped){
            if (player->invoke(INVOKE_ID_GET_DECODED_VIDEO_FRAME, NULL, pVideoFrame) == MAG_NO_ERROR){
                OMX_BUFFERHEADERTYPE *buf;
                AVFrame *frame;

                buf   = static_cast<OMX_BUFFERHEADERTYPE *>(pVideoFrame);
                frame = (AVFrame *)buf->pBuffer;

                if (VideoTexture == NULL){
                    VideoTexture = SDL_CreateTexture(pRenderer, 
                                                     SDL_PIXELFORMAT_YV12, 
                                                     SDL_TEXTUREACCESS_STREAMING, 
                                                     gpVideoMetaData->width, 
                                                     gpVideoMetaData->height);
                    if (!VideoTexture) {
                        AGILE_LOGE("Couldn't set create video texture: %s\n", SDL_GetError());
                        goto error;
                    }
                }
                UpdateVideoTexture(VideoTexture, frame);
            }
            
            SDL_RenderClear(pRenderer);
            renderTexture(pRenderer, 
                          VideoTexture, 
                          0, 0, 
                          gpVideoMetaData->width, 
                          gpVideoMetaData->height); 
            SDL_RenderPresent(pRenderer);

            player->invoke(INVOKE_ID_PUT_USED_VIDEO_FRAME, pVideoFrame, NULL);
        }
    }

    SDL_DestroyTexture( VideoTexture );

error:
    TTF_CloseFont( pFont );

    //Destroy window    
    SDL_DestroyRenderer( pRenderer );
    SDL_DestroyWindow( pWindow );

    //Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();

    AGILE_LOGD("exit mmp_sdl ...\n\n");
}