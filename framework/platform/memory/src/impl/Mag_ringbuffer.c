#include "Mag_ringbuffer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_RingBuffer"

/*unblock read*/
static i32 MagRingBufferRead(struct MagRingBuffer *self, i32 bytes, ui8 *pBuf){
    i32 first_part = 0;
    i32 second_part = 0;
    i32 read = 0;
    i32 remaining = bytes;

    i32 local_rb_reading_pos;
    i32 local_rb_writing_pos;
    i32 local_rb_unread_data_size;
    i32 orig_rb_unread_data_size;

    local_rb_reading_pos = self->rb_reading_pos;
    local_rb_writing_pos = self->rb_writing_pos;
    local_rb_unread_data_size = self->rb_unread_data_size;
    orig_rb_unread_data_size  = self->rb_unread_data_size;

    if (local_rb_unread_data_size > 0){
        if (local_rb_unread_data_size < bytes){
            remaining = local_rb_unread_data_size;
        }

        if (local_rb_writing_pos > local_rb_reading_pos){
            AGILE_LOGV("rb_reading_pos: %d, read: %d", local_rb_reading_pos, remaining);
            memcpy(pBuf, self->pRingBuffer + local_rb_reading_pos, remaining);
            local_rb_reading_pos = (local_rb_reading_pos + remaining) % self->rb_size;
        }else{
            first_part = self->rb_size - local_rb_reading_pos;
            if (remaining > first_part)
                second_part = remaining - first_part;
            else
                first_part = remaining;

            AGILE_LOGV("rb_reading_pos: %d, first part read: %d", local_rb_reading_pos, first_part);
            memcpy(pBuf, self->pRingBuffer + local_rb_reading_pos, first_part);
            if (second_part > 0){
                AGILE_LOGV("rb_reading_pos: %d, second part read: %d", local_rb_reading_pos, second_part);
                memcpy(pBuf + first_part, self->pRingBuffer, second_part);
            }

            local_rb_reading_pos = (local_rb_reading_pos + first_part + second_part) % self->rb_size;
        }

        read = remaining;
        local_rb_unread_data_size -= remaining;

        if (local_rb_unread_data_size == 0){
            AGILE_LOGD("Ring buffer[%p] becomes empty!, rb_writing_pos: %d, rb_reading_pos: %d", 
                        self, local_rb_writing_pos, local_rb_reading_pos);
        }
    }

    pthread_mutex_lock(&self->mutex);

    if (orig_rb_unread_data_size == self->rb_unread_data_size){
        self->rb_unread_data_size = local_rb_unread_data_size;
    }else{
        if (self->rb_unread_data_size > orig_rb_unread_data_size){
            self->rb_unread_data_size = local_rb_unread_data_size + (self->rb_unread_data_size - orig_rb_unread_data_size);
            
        }else{
            AGILE_LOGE("[%p]: fatal error: rb_unread_data_size = %d < orig_rb_unread_data_size = %d!!!!", 
                        self->rb_unread_data_size, orig_rb_unread_data_size);
        }
    }

    self->rb_reading_pos = local_rb_reading_pos;

    pthread_mutex_unlock(&self->mutex);

    return read;
}

/*unblock write*/
static i32 MagRingBufferWrite(struct MagRingBuffer *self, i32 bytes, ui8 *pBuf){
    i32 free_space = 0;
    i32 old_data_size = 0;
    i32 write = 0;
    i32 remaining = bytes;
    i32 first_part = 0;
    i32 second_part = 0;

    i64 local_source_offset = 0;
    i32 local_rb_reading_pos;
    i32 local_rb_writing_pos;
    i32 local_rb_unread_data_size;
    i32 local_rb_total_data_size = 0;
    i32 orig_rb_unread_data_size;

    local_rb_reading_pos = self->rb_reading_pos;
    local_rb_writing_pos = self->rb_writing_pos;
    local_rb_unread_data_size = self->rb_unread_data_size;
    orig_rb_unread_data_size = self->rb_unread_data_size;
    local_rb_total_data_size = self->rb_total_data_size;

    if (local_rb_unread_data_size < self->rb_size){
        free_space = self->rb_size - local_rb_total_data_size;

        if (free_space < bytes){
            if (free_space > 0){
                memcpy(self->pRingBuffer + local_rb_writing_pos, pBuf, free_space);
                local_rb_total_data_size += free_space;
                local_rb_writing_pos = 0;
                local_rb_unread_data_size += free_space;
                write += free_space;
                remaining = bytes - write;
            }

            /*the buffer is full of the data(read and unread), need to over-write the old data*/
            if (local_rb_reading_pos > local_rb_writing_pos){
                old_data_size = local_rb_reading_pos - local_rb_writing_pos;
                if (old_data_size < remaining){
                    remaining = old_data_size;
                }
            }else if (local_rb_reading_pos <= local_rb_writing_pos){
                /*If read_pos == write_pos, it means the ring buffer is empty*/
                first_part = self->rb_size - local_rb_writing_pos;
                second_part = local_rb_reading_pos;

                if (first_part < remaining){
                    memcpy(self->pRingBuffer + local_rb_writing_pos, pBuf + write, first_part);
                    remaining -= first_part;
                    local_rb_writing_pos = 0;
                    local_rb_unread_data_size += first_part;
                    local_source_offset += (i64)first_part;
                    /*self->source_offset += (i64)first_part;*/
                    write += first_part;

                    if (second_part < remaining){
                        remaining = second_part;
                    }
                }
            }

            local_source_offset += (i64)remaining;
            /*self->source_offset += (i64)remaining;*/
        }else{
            local_rb_total_data_size += bytes;
        }

        if (remaining > 0){
            AGILE_LOGV("rb_writing_pos: %d, write: %d, remaining: %d", 
                        local_rb_writing_pos, write, remaining);
            memcpy(self->pRingBuffer + local_rb_writing_pos, pBuf + write, remaining);

            local_rb_writing_pos      = (local_rb_writing_pos + remaining) % self->rb_size;
            local_rb_unread_data_size += remaining;
            write                     += remaining;

            if (local_rb_unread_data_size == self->rb_size){
                AGILE_LOGD("Ring buffer[%p] becomes full! rb_writing_pos: %d, rb_reading_pos: %d", 
                            self, local_rb_writing_pos, local_rb_reading_pos);
            }
        }
    }

    pthread_mutex_lock(&self->mutex);

    if (orig_rb_unread_data_size == self->rb_unread_data_size){
        self->rb_unread_data_size = local_rb_unread_data_size;
    }else{
        if (self->rb_unread_data_size < orig_rb_unread_data_size){
            self->rb_unread_data_size = local_rb_unread_data_size - (orig_rb_unread_data_size - self->rb_unread_data_size);  
        }else{
            AGILE_LOGE("[%p]: fatal error: rb_unread_data_size = %d > orig_rb_unread_data_size = %d!!!!", 
                        self->rb_unread_data_size, orig_rb_unread_data_size);
        }
    }

    self->rb_writing_pos = local_rb_writing_pos;
    self->source_offset +=  local_source_offset;
    self->rb_total_data_size = local_rb_total_data_size;

    pthread_mutex_unlock(&self->mutex);

    return write;
}

/*the offset against the start of the source
* return:
* 0: hit on the buffer
* others: the offset short to the source range
*/
static i64 MagRingBufferSeek(struct MagRingBuffer *self, i64 offset){
    i64 seek_short = 0;
    i32 seek_offset = 0;
    i64 source_range_end = 0;

    if (offset < 0){
        return kInvalidSeekOffset;
    }

    pthread_mutex_lock(&self->mutex);

    /*{
        i32 out;
        ui8 *p = self->pRingBuffer;

        for(out = 0; out < 16; out++){
            AGILE_LOGD("%dth: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                        out, p[out*16], p[out*16 + 1], p[out*16 + 2], p[out*16 + 3],
                        p[out*16 + 4], p[out*16 + 5], p[out*16 + 6], p[out*16 + 7],
                        p[out*16 + 8], p[out*16 + 9], p[out*16 + 10], p[out*16 + 11],
                        p[out*16 + 12], p[out*16 + 13], p[out*16 + 14], p[out*16 + 15]);
        }
    }*/

    source_range_end = self->source_offset + self->rb_total_data_size - 1;

    if (offset >= self->source_offset && offset <= source_range_end){
        /*hit on*/
        seek_offset = (i32)(offset - self->source_offset);
        if (self->rb_total_data_size < self->rb_size){
            self->rb_reading_pos = seek_offset;
        }else{
            AGILE_LOGD("seek offset: %d, rb_reading_pos: %d, rb_writing_pos: %d",
                        seek_offset, self->rb_reading_pos, self->rb_writing_pos);
            self->rb_reading_pos = (self->rb_writing_pos + seek_offset) % self->rb_size;
            self->rb_unread_data_size = self->rb_size - seek_offset;
        }
    }else{
        /*missed and get the offset short out of the data range*/
        if (offset > source_range_end)
            seek_short = offset - source_range_end;
        else
            seek_short = offset - self->source_offset;
    }

    pthread_mutex_unlock(&self->mutex);

    return seek_short;
}

static void MagRingBufferFlush(struct MagRingBuffer *self){
    pthread_mutex_lock(&self->mutex);

    self->rb_reading_pos = 0;
    self->rb_writing_pos = 0;
    self->rb_total_data_size = 0;
    self->rb_unread_data_size = 0;

    pthread_mutex_unlock(&self->mutex);
}

static void MagRingBufferSetSourcePos(struct MagRingBuffer *self, i64 source_offset){
    pthread_mutex_lock(&self->mutex);
    self->source_offset = source_offset;
    pthread_mutex_unlock(&self->mutex);
}

static void MagGetSourceRange(struct MagRingBuffer *self, i64 *start, i64 *end){
    pthread_mutex_lock(&self->mutex);

    *start = self->source_offset;
    *end   = self->source_offset + self->rb_total_data_size;

    pthread_mutex_unlock(&self->mutex);
}

MagRingBufferHandle Mag_createRingBuffer(i32 bytes, ui32 flags){
    MagRingBufferHandle hRB = NULL;
    int rc = 0;

    hRB = (MagRingBufferHandle)mag_mallocz(sizeof(MagRingBuffer_t));
    if (NULL == hRB){
        AGILE_LOGE("failed to create MagRingBuffer_t!");
        return NULL;
    }

    hRB->pRingBuffer = (ui8 *)mag_malloc(bytes);
    if (NULL == hRB->pRingBuffer){
        mag_free(hRB);
        AGILE_LOGE("failed to create ring buffer(%d bytes)!", bytes);
        return NULL;
    }

    hRB->rb_size = bytes;

    rc = pthread_mutex_init(&hRB->mutex, NULL /* default attributes */);
    if (rc != 0){
        mag_free(hRB->pRingBuffer);
        mag_free(hRB);
        AGILE_LOGE("failed to initialize the mutex, rc=%d!", rc);
        return NULL;
    }

    hRB->read           = MagRingBufferRead;
    hRB->write          = MagRingBufferWrite;
    hRB->seek           = MagRingBufferSeek;
    hRB->flush          = MagRingBufferFlush;
    hRB->setSourcePos   = MagRingBufferSetSourcePos;
    hRB->getSourceRange = MagGetSourceRange;

    return hRB;
}

void Mag_destroyRingBuffer(MagRingBufferHandle *phRingBuffer){
    MagRingBufferHandle h;

    h = *phRingBuffer;

    mag_free(h->pRingBuffer);
    pthread_mutex_destroy(&h->mutex);
    mag_freep((void **)phRingBuffer);
}