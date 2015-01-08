#ifndef __MAG_INVOKE_DEF_H__
#define __MAG_INVOKE_DEF_H__

typedef struct{
	int audio_buffer_time;
	int video_buffer_time;
	int loadingSpeed;
}BufferStatistic_t;

typedef struct{
	unsigned int width;
	unsigned int height;
	double       fps;
	unsigned int bps;     /*in kb/s*/
    char codec[128];
}VideoMetaData_t;

typedef struct{
    unsigned int hz;
    unsigned int bps;
    char codec[32];
}AudioMetaData_t;

typedef enum invoke_id_t{
    INVOKE_ID_GET_BUFFER_STATUS = 0x1000,
    INVOKE_ID_GET_VIDEO_META_DATA,
    INVOKE_ID_GET_AUDIO_META_DATA,
    INVOKE_ID_GET_DECODED_VIDEO_FRAME,
    INVOKE_ID_PUT_USED_VIDEO_FRAME,
    INVOKE_ID_GET_DECODED_AUDIO_FRAME,
    INVOKE_ID_PUT_USED_AUDIO_FRAME,
}INVOKE_ID_t;

#endif