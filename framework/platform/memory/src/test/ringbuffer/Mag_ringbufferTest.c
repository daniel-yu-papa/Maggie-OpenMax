#include "Mag_ringbuffer.h"
#include <time.h>
#include <unistd.h>

ui8 gSource[256];
i64 gTotalWrite = 0;
pthread_mutex_t  gMutex;

#define WRITING_LOOPS 200000
#define READING_LOOPS 10000

static void *fillBufferEntry(void *arg){
#define WRITE_BYTES 32
    MagRingBufferHandle rb = (MagRingBufferHandle)arg;
    i32 i;
    ui8 *p1, *p2;
    i32 total_write = 0;
    i32 ret_write;
    i32 write_bytes = WRITE_BYTES;
    i32 first_part = 0;
    i32 second_part = 0;

    p1 = gSource;
    p2 = NULL;
    first_part = write_bytes;
    for (i = 0; i < WRITING_LOOPS; i++){

        pthread_mutex_lock(&gMutex);
        if (p1){
            ret_write = rb->write(rb, first_part, p1);
            total_write += ret_write;
            gTotalWrite += ret_write;
            if(ret_write == first_part){
                if (p2){
                    ret_write = rb->write(rb, second_part, p2);
                    total_write += ret_write;
                    gTotalWrite += ret_write;
                }
            }
        }
        pthread_mutex_unlock(&gMutex);

        write_bytes = (write_bytes + 10) % 256;
        if (write_bytes == 0)
            write_bytes = WRITE_BYTES;

        total_write = total_write % 256;

        if (total_write > 0){
            first_part = 256 - total_write;
            second_part = write_bytes - first_part;

            p1 = &gSource[total_write];
            if (second_part > 0){
                p2 = gSource;
            }else{
                p2 = NULL;
            }
            /*AGILE_LOGD("first part: %d, second_part: %d, p1: %p, p2: %p [gSource: %p]", 
                        first_part, second_part, p1, p2, gSource);*/
        }else{
            p1 = gSource;
            p2 = NULL;
            first_part = write_bytes;
            second_part = 0;
            /*AGILE_LOGD("first part: %d, second_part: %d, p1: %p, p2%p [gSource: %p]", 
                        first_part, second_part, p1, p2, gSource);*/
        }

        usleep(10);
    }
}

static void verify_read_buffer(ui8 *buf, i32 len, ui8 start_value){
    i32 i;
    ui8 expected_value = start_value;
    boolean matched = MAG_TRUE;

    for (i = 0; i < len; i++){
        if (buf[i] != expected_value){
            AGILE_LOGE("the %dth data: 0x%x is not the expected value: 0x%x", 
                        i, buf[i], expected_value); 
            matched = MAG_FALSE;
        }
        expected_value++;
    }

    if (matched){
        AGILE_LOGD("matched!!!");
    }
}

int main(){
    i32 i;
    i32 j;
    ui8 init_value = 0x00;
    ui8 expected_value;
    ui8 seek_expected_value;
    ui8 *p = NULL;
    MagRingBufferHandle rb;
    pthread_t fillBufThread;
    ui8 read_buf[256];
    i32 read_bytes = 18;
    i32 ret_bytes;
    i64 source_start;
    i64 source_end;
    i64 seek_ret;
    i32 residue;

    Mag_agilelogCreate();

    for (i = 0; i < 256; i++){
        gSource[i] = init_value++;
    }

    AGILE_LOGD("enter!");
    
    rb = Mag_createRingBuffer(256, 0);

    pthread_mutex_init(&gMutex, NULL /* default attributes */);
    pthread_create(&fillBufThread, NULL, fillBufferEntry, rb);

    expected_value = init_value;
    for (i = 0; i < READING_LOOPS; i++){
        for (j = 0; j < 20; j++){
            ret_bytes = rb->read(rb, read_bytes, read_buf);
            if (ret_bytes > 0){
                if (ret_bytes < read_bytes){
                    AGILE_LOGD("read bytes: %d vs expected bytes: %d", ret_bytes, read_bytes);
                }

                AGILE_LOGD("*******Loop %d: check read buffer(%d)*******", i, ret_bytes);
                verify_read_buffer(read_buf, ret_bytes, expected_value);
                AGILE_LOGD("***********Loop %d: check end***********", i);

                read_bytes = (read_bytes + 10) % 256;
                if (read_bytes == 0){
                    read_bytes = 18;
                }
                expected_value = ++read_buf[ret_bytes - 1] % 256;
            }else{
                AGILE_LOGD("read bytes: 0");
            }

            usleep(10);
        }

        pthread_mutex_lock(&gMutex);

        rb->getSourceRange(rb, &source_start, &source_end);
        AGILE_LOGD("source_start: %lld, source_end: %lld, total write: %lld", 
                    source_start, source_end, gTotalWrite);

#define SOURCE_OFFSET 20
        AGILE_LOGD("*******Seek to offset 20*******");
        seek_ret = rb->seek(rb, source_start + SOURCE_OFFSET);

        if (seek_ret == 0){
            residue = (i32)(source_start % 256);
            seek_expected_value = (residue + SOURCE_OFFSET) % 256;
            ret_bytes = rb->read(rb, 32, read_buf);
            AGILE_LOGD("ret_bytes = %d, seek_expected_value: 0x%x", ret_bytes, seek_expected_value);
            verify_read_buffer(read_buf, ret_bytes, seek_expected_value);
            expected_value = ++read_buf[ret_bytes - 1] % 256;
            AGILE_LOGD("*******Seek to offset end******");
        }else{
            AGILE_LOGD("*******offset short: %d******", seek_ret);
        }
        pthread_mutex_unlock(&gMutex);
        
    }
    
    pthread_join(fillBufThread, NULL);

    pthread_mutex_destroy(&gMutex);
    Mag_destroyRingBuffer(&rb);

    Mag_agilelogDestroy();

    return 0;
}