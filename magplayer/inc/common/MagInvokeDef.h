#ifndef __MAG_INVOKE_DEF_H__
#define __MAG_INVOKE_DEF_H__

typedef struct{
	int audio_buffer_time;
	int video_buffer_time;
	int loadingSpeed;
}BufferStatistic_t;

typedef struct{
	unsigned int top_rgb;
	unsigned int bottom_rgb;
}PictureRGB_t;

typedef enum invoke_id_t{
    INVOKE_ID_GET_BUFFER_STATUS = 0x1000,
    INVOKE_ID_GET_PICTURE_RGB
}INVOKE_ID_t;

#endif