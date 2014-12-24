#ifndef __MAG_OMX_BUFFER_H__
#define __MAG_OMX_BUFFER_H__

#include "framework/MagFramework.h"

typedef   enum {
  STREAM_FRAME_FLAG_NONE        = 0,
  STREAM_FRAME_FLAG_KEY_FRAME   = (1 << 0),
  STREAM_FRAME_FLAG_EOS         = (1 << 1),
} StreamFrameFlag_t;

typedef struct{
  void *data;
  i32  size;
  ui32 type;
}MediaSideData_t;

typedef struct mag_omx_media_buffer_t{
  List_t node;

  ui32 buffer_size;
  void *buffer;
  i64 pts; /*in us*/
  i64 dts; /*in us*/  

  i32 stream_index; /*used for ffmpeg stream index*/
  i32 duration;     /*Duration of this packet in AVStream->time_base units*/
  i64 pos;          /*byte position in stream*/
  StreamFrameFlag_t flag;

  MediaSideData_t *side_data;
  i32 side_data_elems;
  
  void *track_obj;
  void *esFormatter;
  
  ui8  eosFrame;
  
  _status_t (*release)(struct mag_omx_media_buffer_t *);
}MagOmxMediaBuffer_t;

#endif
