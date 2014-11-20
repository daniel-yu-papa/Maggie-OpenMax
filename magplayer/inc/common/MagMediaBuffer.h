#ifndef __MAG_MEDIA_BUFFER_H__
#define __MAG_MEDIA_BUFFER_H__

#include "framework/MagFramework.h"

typedef   enum {
  STREAM_FRAME_FLAG_NONE        = 0,
  STREAM_FRAME_FLAG_KEY_FRAME   = (1 << 0),
  STREAM_FRAME_FLAG_EOS         = (1 << 1),
} StreamFrameFlag_t;

typedef struct media_buffer_t{
    list_t node;

    ui32 buffer_size;
    void *buffer;
    i64 pts; /*in us*/
    i64 dts; /*in us*/  
    StreamFrameFlag_t flag;

    void *track_obj;
    void *esFormatter;
    
    ui8  eosFrame;
    
    _status_t (*release)(struct media_buffer_t *);
}MediaBuffer_t;

#endif
