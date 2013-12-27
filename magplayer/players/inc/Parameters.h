#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

typedef enum{
    MagParamTypeInt32,
    MagParamTypeInt64,
    MagParamTypeUInt32,
    MagParamTypeFloat,
    MagParamTypeDouble,
    MagParamTypePointer,
    MagParamTypeString,
}MagParamType_t;

typedef struct{
    ui32 x;
    ui32 y;
    ui32 w;
    ui32 h;
}MagVideoDisplay_t;

typedef enum{
    MediaTypeTS,
    MediaTypeES,
    MediaTypeMP4, 
    MediaTypeFLV,
}MagMediaType_t;

/*for TS type media stream*/
typedef struct{
    ui32 pidTable[32];
    ui32 codec[32];
    ui32 num;
}MagTsPIDList_t;

#define kMediaType             "media.type"            /*Int32: MagMediaType_t*/

#define kVideo_Codec           "video.codec"           /*Int32: OMX_VIDEO_CODINGTYPE*/
#define kVideo_TS_pidlist      "video.ts.pidlist"      /*Pointer: MagTsPIDList_t */

#define kVideo_Display         "video.display"         /*Pointer: MagVideoDisplay_t*/

#define kAudio_Codec           "audio.codec"           /*Int32: OMX_AUDIO_CODINGTYPE*/
#define kAudio_TS_pidlist      "audio.ts.pidlist"      /*Pointer: MagTsPIDList_t */


#endif