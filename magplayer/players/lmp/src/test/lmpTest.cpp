#include "lmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "MagInvokeDef.h"

static bool monitorFlag = false;

void *monitorThreadProc(void *arg){
    BufferStatistic_t bufStat;

    LinuxMediaPlayer *player = (LinuxMediaPlayer *)arg;

    while(monitorFlag){
        player->invoke(INVOKE_ID_GET_BUFFER_STATUS, NULL, &bufStat);
        printf("audio buffer: %d ms, video buffer: %d ms, loadingSpeed %d Bytes/Sec/\n", 
                bufStat.audio_buffer_time, bufStat.video_buffer_time, bufStat.loadingSpeed);

        sleep(4);
    }

    printf("exit monitorThreadProc!\n");
}

void lmpTestCb(lmp_event_t evt, void *handler, unsigned int param1, unsigned int param2){
    printf("get event: %d, param1: %d, param2: %d\n", evt, param1, param2);
}

int main(int argc, char *argv[]){
	char *url = NULL;
	LinuxMediaPlayer *player = NULL;
    int ret;
    bool first_running = true;
    bool isPlaying = false;
    char c;
    float volume = 0.1;
    int seekTime = 0;
    bool isReset = false;
    pthread_t  monitorThread;

	if (argc < 2){
		printf("usage: lmpTest url\n");
		return -1;
	}
    url = argv[1];
    printf("lmpTest playes url: %s\n\n\n", url);

    player = GetMediaPlayer();
    if (player == NULL){
    	printf("failed to create the player!\n");
    	return -1;
    }

    monitorFlag = true;
    pthread_create(&monitorThread, NULL, monitorThreadProc, (void *)player);

    player->registerEventCallback(lmpTestCb, NULL);
    
    ret = player->setDataSource(url);
    if (ret != LMP_OK){
    	printf("failed to do setDataSource(%s)\n", url);
    	return -1;
    }else{
    	ret = player->prepareAsync();
    	if (ret != LMP_OK){
    		printf("failed to do prepareAsync()\n");
    		return -1;
    	}
    }
    
    while (1){
        printf("p: play, s: stop, t: reset, a: pause, r: resume, v: volume up, b: volume down, f: flush, e: seek 1s, d: seek -1s, q: exit the program.\n");
        scanf("%c",&c);
        printf("\ninput is '%c'\n", c);

        if (c == 'p'){
            if (isReset){
                player->setDataSource(url);
                player->prepareAsync();
                player->start();
                isPlaying = true;
                isReset   = false;
            }else{
            	// if(!isPlaying){
    	            printf("start playing ...\n");
                    if (!first_running)
    	               player->prepareAsync();
    	            player->start();
    	            isPlaying = true;
                    first_running = false;
    	        // }else{
    	        // 	printf("In playing, ignore the play command!\n");
    	        // }
            }
        }else if (c == 's'){
            if(isPlaying){
	            printf("stop playing ...\n");
	            player->stop();
	            isPlaying = false;
	        }else{
	        	printf("In stopping, ignore the stop command!\n");
	        }
        }else if (c == 't'){
            printf("reset player ...\n");
            player->reset();
            isReset = true;
        }else if (c == 'q'){
        	printf("To destroy the player. Quit!\n");
            // if (isPlaying){
            // 	player->stop();
            // }
            monitorFlag = false;
            pthread_join(monitorThread, NULL);
            delete player;
            printf("after deleting player\n");
            break;
        }else if (c == 'a'){
            printf("pause!\n");
            player->pause();
        }else if (c == 'r'){
            printf("resume!\n");
            player->start();
        }else if (c == 'v'){
            volume = volume + 0.1;
            if (volume > 1.0)
                volume = 1.0;
            printf("volume up to %f\n", volume);
            player->setVolume(volume, volume);
        }else if (c == 'b'){
            volume = volume - 0.1;
            if (volume < 0.0)
                volume = 0.0;
            printf("volume down to %f\n", volume);
            player->setVolume(volume, volume);
        }else if (c == 'f'){
            /*if the initial state is playing, do flush() and then resume(), the playing continues
            * if the initial state is pause, do flush() and then resume(), keep pause state
            */
            printf("flush the player\n");
            player->flush();
        }else if (c == 'e'){
            seekTime += 180000;
            printf("seek to %d ms\n", seekTime);
            player->seekTo(seekTime);
        }else if (c == 'd'){
            seekTime -= 180000;
            printf("seek to %d ms\n", seekTime);
            player->seekTo(seekTime);
        }
	}
    printf("exit lmpTest ...\n\n");
}