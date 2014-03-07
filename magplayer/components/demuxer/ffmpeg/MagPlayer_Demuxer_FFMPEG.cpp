#include "MagPlayer_Demuxer_FFMPEG.h"

#define DUMMY_FILE "dummy_file"

static const ui32 kAVIOBufferSize = (32 * 1024);

static const ui32 kVideoBufPoolSize = (8 * 1024 * 1024);
static const ui32 kAudioBufPoolSize = (512 * 1024);
static const ui32 kSubtitleBufPoolSize = (512 * 1024);


MagPlayer_Demuxer_FFMPEG::MagPlayer_Demuxer_FFMPEG():
                             mInitialized(false),
                             mTotalTrackNum(0),
                             mStreamIDRoot(NULL){

}

MagPlayer_Demuxer_FFMPEG::~MagPlayer_Demuxer_FFMPEG(){

}

i32 MagPlayer_Demuxer_FFMPEG::AVIO_Read (ui8 *buf, int buf_size){
    int size = buf_size;
    
    if (NULL != mDataSource){
        if (MPCP_OK == mDataSource->Read(buf, &size)){
            return size;
        }
    }
    return 0;
}

i32 MagPlayer_Demuxer_FFMPEG::AVIO_Write(ui8* buf, int buf_size){
    AGILE_LOGE("The operation is not valid!");
    return 0;
}

i64 MagPlayer_Demuxer_FFMPEG::AVIO_Seek (i64 offset, i32 whence){
    MPCP_ORIGINTYPE flag;
    i64 size;
    i64 targetPosition;
    i64 currentPos;
    
    switch(whence) {
        case SEEK_SET:
            flag = MPCP_OriginBegin;
            targetPosition = offset;
            break;
            
        case SEEK_CUR:
            flag = MPCP_OriginCur;
            if (MPCP_OK == mDataSource->GetCurrentPosition(&currentPos)){
                targetPosition = currentPos + offset;
            }else{
                return -1;
            }
            break;
            
        case SEEK_END:
            flag = MPCP_OriginEnd;
            if (MPCP_OK == mDataSource->GetSize(&size)){
                targetPosition = size + offset;
            }else{
                return -1;
            }
            break;
            
        case AVSEEK_SIZE:
            if (MPCP_OK == mDataSource->GetSize(&size)){
                return size;
            }else{
                AGILE_LOGE("Failed to get the size of the data source!");
                return -1;
            }
            
        default:
            AGILE_LOGE("Invalid seek whence (%d) in ByteIOAdapter::Seek", whence);
    }

    if (MPCP_OK == mDataSource->SetPosition(offset, flag)){
        return targetPosition;
    }
    return -1;
}

i32 MagPlayer_Demuxer_FFMPEG::AVIO_Static_Read (void *opaque, ui8 *buf, int buf_size){
    if (NULL == opaque){
        AGILE_LOGE("input void *opaque is NULL!");
        return 0;
    }
    
    return static_cast<MagPlayer_Demuxer_FFMPEG *>(opaque)->AVIO_Read(buf, buf_size);
}

i32 MagPlayer_Demuxer_FFMPEG::AVIO_Static_Write(void *opaque, ui8 *buf, int buf_size){
    if (NULL == opaque){
        AGILE_LOGE("input void *opaque is NULL!");
        return 0;
    }
    
    return static_cast<MagPlayer_Demuxer_FFMPEG *>(opaque)->AVIO_Write(buf, buf_size);
}

i64 MagPlayer_Demuxer_FFMPEG::AVIO_Static_Seek (void *opaque, i64 offset, i32 whence){
    if (NULL == opaque){
        AGILE_LOGE("input void *opaque is NULL!");
        return 0;
    }
    
    return static_cast<MagPlayer_Demuxer_FFMPEG *>(opaque)->AVIO_Seek(buf, buf_size);
}

AVIOContext *MagPlayer_Demuxer_FFMPEG::ffmpeg_utiles_CreateAVIO(){
    ui32 size;
    ui8  *buffer;
    AVIOContext *context;
    
    size = kAVIOBufferSize + FF_INPUT_BUFFER_PADDING_SIZE;
    buffer = static_cast<ui8 *>(mag_malloc(size));

    if (NULL == buffer){
        AGILE_LOGE("Failed to create the AVIO buffer for ffmpeg!");
        return NULL;
    }
    
    context = avio_alloc_context
                    (buffer,
                     size ,
                     0,
                     static_cast<void*>(this),
                     AVIO_Static_Read,
                     AVIO_Static_Write,
                     AVIO_Static_Seek);
        
    if (NULL == context){
        AGILE_LOGE("Failed to create the AVIO context!");
        mag_free(buffer);
        buffer = NULL;
        return NULL;
    }
    return context;
}

AVFormatContext *MagPlayer_Demuxer_FFMPEG::ffmpeg_utiles_CreateAVFormat(AVInputFormat *inputFormat, 
                                                                                 AVIOContext *context, 
                                                                                 bool seekable, 
                                                                                 bool probe){
    AVFormatContext *format;

    format = avformat_alloc_context();
    if (NULL == format){
        AGILE_LOGE("Failed to create the AVFormatContext!");
        return NULL;
    }
    
    format->pb = context;
    context->pb->seekable = seekable;

    if (probe){
        i32 res;
        
        res = avformat_open_input(
            &context,
            DUMMY_FILE,  /* need to pass a filename*/
            inputFormat,   /* probe the container format*/
            NULL);         /* no special parameters*/
        
        if (res < 0) {
            AGILE_LOGE("Failed to open the input stream.");
            av_free(context);
            return NULL;
        }

        res = avformat_find_stream_info(context, NULL);
        if (res < 0) {
            AGILE_LOGE("Failed to find stream information.");
            avformat_close_input(&context);
            return NULL;
        }
    }

    return format;
}

void MagPlayer_Demuxer_FFMPEG::ffmpeg_utiles_SetOption(const char *opt, const char *arg){
    const AVOption *o;
    const AVClass *fc = avformat_get_class();

    if ((o = av_opt_find(&fc, opt, NULL, 0,
                          AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ))) {
        av_dict_set(&mpFormatOpts, opt, arg, (o->type == AV_OPT_TYPE_FLAGS) ? AV_DICT_APPEND : 0);
        AGILE_LOGD("[SetOption] - %s:%s", opt, arg);
    }
}

enum CodecID MagPlayer_Demuxer_FFMPEG::convertCodec_OMXToFFMPEG(ui32 omxCodec){
    enum CodecID codec;
    
    switch(static_cast<OMX_VIDEO_CODINGTYPE>(omxCodec)){
        case OMX_VIDEO_CodingAVC:
            codec = AV_CODEC_ID_H264;
            break;

        case OMX_VIDEO_CodingMPEG4:
            codec = AV_CODEC_ID_MPEG4;
            break;

        case OMX_VIDEO_CodingMPEG2:
            codec = AV_CODEC_ID_MPEG2VIDEO;
            break;
            
        case OMX_AUDIO_CodingMP3:
        case OMX_AUDIO_CodingMP2:
            codec = AV_CODEC_ID_MP3;
            break;
            
        case OMX_AUDIO_CodingAAC:
            codec = AV_CODEC_ID_AAC;
            break;
            
        case OMX_AUDIO_CodingAC3:
            codec = AV_CODEC_ID_AC3;
            break;

        case OMX_AUDIO_CodingDDPlus:
            codec = AV_CODEC_ID_EAC3;
            break;
            
        default:
            AGILE_LOGE("the omxCodec type[%d] is not supported!", omxCodec);
            codec = AV_CODEC_ID_NONE;
            break;
    };
    return codec;  
}

_status_t  MagPlayer_Demuxer_FFMPEG::create_track(TrackInfo_t *track){
    ui32 buf_pool_size = 0;
    Stream_Track *strack = NULL;
    MediaBuffer_t *mb;
    
    if (track->type == TRACK_VIDEO){
        buf_pool_size = kVideoBufPoolSize;
    }else if (track->type == TRACK_AUDIO){
        buf_pool_size = kAudioBufPoolSize;
    }else{
        buf_pool_size = kSubtitleBufPoolSize;
    }

    strack = new Stream_Track(track, buf_pool_size);
    if (NULL != strack){
        mStreamIDRoot = rbtree_insert(mStreamIDRoot, track->streamID, static_cast<void *>(strack));
    }else{
        return MAG_NO_MEMORY;
    }
    return MAG_NO_ERROR;
}

bool MagPlayer_Demuxer_FFMPEG::ts_noprobe_decide_adding(enum CodecID codec, ui32 pid){
    i32 i;
    
    for (i = 0; i < mpAVFormat->nb_streams; i++){
        if (NULL != mpAVFormat->streams[i]){
            AGILE_LOGD("Stream #%d - pid=%d, codec_id=%d", 
                        i,
                        mpAVFormat->streams[i]->id, 
                        mpAVFormat->streams[i]->codec->codec_id);
            
            if (!(mpAVFormat->streams[i]->id & TS_NOPROBE_REMOVE_PID_MASK) && (mpAVFormat->streams[i]->id == pid)){
                if (mpAVFormat->streams[i]->codec->codec_id == codec){
                    return false;
                }else{
                    mpAVFormat->streams[i]->id |= TS_NOPROBE_REMOVE_PID_MASK;
                    AGILE_LOGD("[FFMPEG_Demux::addAVStreams]: To remove old stream and to add new pid=%d, codec_id=%d", 
                               pid, codec);
                    return true;
                }
            } 
        }
    }
    return true;
}

_status_t  MagPlayer_Demuxer_FFMPEG::ts_noprobe_add_streams(){
    void *value;
    char keyName[64];
    i32 number;
    i32 i;
    boolean ret;
    TrackInfo_t *track;
    enum CodecID codec;

    AVCodec *avcodec;
    AVStream *avstream;
    AVCodecContext *avcontext;
    
    if (NULL == mParamDB){
        AGILE_LOGE("the Demuxer is not initialized. Quit!");
        return MAG_NO_INIT;
    }

    ret = mParamDB->findInt32(mParamDB, kDemuxer_Video_Track_Number, &number);
    if (ret == MAG_TRUE){
        mTotalTrackNum = mTotalTrackNum + number;
    }

    ret = mParamDB->findInt32(mParamDB, kDemuxer_Audio_Track_Number, &number);
    if (ret == MAG_TRUE){
        mTotalTrackNum = mTotalTrackNum + number;
    }

    ret = mParamDB->findInt32(mParamDB, kDemuxer_Subtitle_Track_Number, &number);
    if (ret == MAG_TRUE){
        mTotalTrackNum = mTotalTrackNum + number;
    }
    AGILE_LOGI("total track number is %d", mTotalTrackNum);
    
    
    for (i = 0; i < mTotalTrackNum; i++){
        sprintf(keyName, kDemuxer_Track_Info, i);
        ret = mParamDB->findPointer(mParamDB, keyName, &value);
        if (ret == MAG_TRUE){
            track = static_cast<TrackInfo_t *>(value);
            codec = convertCodec_OMXToFFMPEG(track->codec);
            if (ts_noprobe_decide_adding(codec, track->pid)){
                avcodec = avcodec_find_decoder(codec);
                if (!avcodec) {        
                    AGILE_LOGE("Failed to create video codec[type %d]", codec);        
                    continue;  
                }

                
                avstream = avformat_new_stream(mpAVFormat, avcodec);
                if (!avstream) {
                    AGILE_LOGE("Failed to create video stream[type %d]", codec);        
                    continue;  
                }

                if (track->pid > 0){
                    avstream->id    = track->pid;
                    track->streamID = avstream->index;
                    avstream->id |= TS_NOPROBE_NEW_PID_MASK;
                    AGILE_LOGD("add stream pid = 0x%x", avstream->id);
                    create_track(track);
                }else{
                    avstream->id = 0;
                }
                avcontext = avstream->codec;
                avcontext->codec_id = codec;

                if (track->type == TRACK_VIDEO){
                    avcontext->codec_type = AVMEDIA_TYPE_VIDEO;
                    /*TODO: need to be configured with the parameters setting*/
                    avcontext->time_base.den = 50;
                    avcontext->time_base.num = 1;
                }else if (track->type == TRACK_AUDIO){
                    avcontext->codec_type = AVMEDIA_TYPE_AUDIO;
                }else if (track->type == TRACK_SUBTITLE){
                    avcontext->codec_type = AVMEDIA_TYPE_SUBTITLE;
                }
            }
        }
    }

    return MAG_NO_ERROR;
}

_status_t  MagPlayer_Demuxer_FFMPEG::ts_noprobe_start(){ 
    i32 rc;
    _status_t ret = MAG_NO_ERROR;
    
    AVDictionary *avdict_tmp = NULL;
    AVIOContext *context;
    AVInputFormat *fmt;

    if (!mInitialized){
        context = ffmpeg_utiles_CreateAVIO();
        if (NULL == context){
            return MAG_NO_MEMORY;
        }
        
        ffmpeg_utiles_SetOption("probesize", "1024"); //128000

        mpAVFormat = ffmpeg_utiles_CreateAVFormat(NULL, context, false, false);
        if (NULL == mpAVFormat){
            return MAG_NO_MEMORY;
        }
        
        av_dict_copy(&avdict_tmp, mpFormatOpts, 0);

        if ((rc = av_opt_set_dict(mpAVFormat, &avdict_tmp)) < 0)
        {
            AGILE_LOGE("Failed to do av_opt_set_dict()");
            ret = MAG_UNKNOWN_ERROR;
            goto opt_fail;
        }
        
        fmt = av_find_input_format("mpegts");
        if (!fmt)
        {
            AGILE_LOGE("Failed to do av_find_input_format(mpegts)");
            ret = MAG_NAME_NOT_FOUND;
            goto failure;
        }
        fmt->flags |= AVFMT_NOFILE;
        mpAVFormat->iformat = fmt;

        /* allocate private data */
        if (mpAVFormat->iformat->priv_data_size > 0) {
            if (!(mpAVFormat->priv_data = av_mallocz(mpAVFormat->iformat->priv_data_size))) {
                AGILE_LOGE("no memory for mAVFormat->priv_data allocation(size = %d)", 
                           mpAVFormat->iformat->priv_data_size);
                ret = MAG_NO_MEMORY;
                goto failure;
            }
            
            if (mpAVFormat->iformat->priv_class) {
                *(const AVClass**)mpAVFormat->priv_data = mpAVFormat->iformat->priv_class;
                av_opt_set_defaults(mpAVFormat->priv_data);
                if (av_opt_set_dict(mpAVFormat->priv_data, &avdict_tmp) < 0){
                    ret = MAG_UNKNOWN_ERROR;
                    goto failure;
                }
            }
        }
    }
    
    ts_noprobe_add_streams();

    if (!mInitialized){
        av_demuxer_open(mAVFormat);
    }else{
        avformat_mpegts_add_pid(mpAVFormat);
    }
    mInitialized = true;
    
    av_dump_format(mAVFormat, 0, DUMMY_FILE, 0);
    
opt_fail:
    if (&mpFormatOpts) {
        av_dict_free(&mpFormatOpts);
        mpFormatOpts = avdict_tmp;
    }
    
    return ret;
    
failure:
    if (mpAVFormat)
        avformat_close_input(&mpAVFormat);
    mpAVFormat = NULL;
    return ret;
}

_status_t  MagPlayer_Demuxer_FFMPEG::ts_noprobe_stop(){
    boolean ret;
    i32 number = 0;
    i32 i;
    char keyName[64];

    if (NULL == mParamDB){
        AGILE_LOGE("the Demuxer is not initialized. Quit!");
        return MAG_NO_INIT;
    }
    
    for (i = 0; i < mTotalTrackNum; i++){
        sprintf(keyName, kDemuxer_Track_Info, i);
        mParamDB->deleteItem(mParamDB, keyName);
    }
    mTotalTrackNum = 0;
    
    return MAG_NO_ERROR;
}

_status_t  MagPlayer_Demuxer_FFMPEG::probe_start(){

}

_status_t  MagPlayer_Demuxer_FFMPEG::probe_stop(){

}

_status_t  MagPlayer_Demuxer_FFMPEG::start(MagPlayer_Component_CP *contentPipe, ui32 flags):
                                     mDataSource(contentPipe){
    char value[32];
    boolean ret;
    
    if (NULL == mParamDB){
        AGILE_LOGE("the Demuxer is not initialized. Quit!");
        return MAG_NO_INIT;
    }

    ret = mParamDB->findString(mParamDB, kDemuxer_Container_Type, &value);

    if (ret == MAG_TRUE){
        if (!strcmp(value, "ts")){
            ret = mParamDB->findString(mParamDB, kDemuxer_Probe_Stream, &value);

            if (ret == MAG_TRUE){
                if (!strcmp(value, "no")){
                    return ts_noprobe_start();
                }
            }
        }
    }

    return probe_start();
}

_status_t  MagPlayer_Demuxer_FFMPEG::stop(){
    char value[32];
    boolean ret;
    
    if (NULL == mParamDB){
        AGILE_LOGE("the Demuxer is not initialized. Quit!");
        return MAG_NO_INIT;
    }

    ret = mParamDB->findString(mParamDB, kDemuxer_Container_Type, &value);

    if (ret == MAG_TRUE){
        if (!strcmp(value, "ts")){
            ret = mParamDB->findString(mParamDB, kDemuxer_Probe_Stream, &value);

            if (ret == MAG_TRUE){
                if (!strcmp(value, "no")){
                    return ts_noprobe_stop();
                }
            }
        }
    }

    return probe_stop();
}

void MagPlayer_Demuxer_FFMPEG::setMediaBufferFields(MediaBuffer_t *mb, AVPacket *packet){
    if ((NULL == mb) || (NULL = packet))
        return;

    mb->buffer      = packet->data;
    mb->buffer_size = packet->size;
    mb->pts         = packet->pts;
    mb->dts         = packet->dts;
    if (AV_PKT_FLAG_KEY == packet->flags)
        mb->flag = STREAM_FRAME_FLAG_KEY_FRAME;
}

_status_t MagPlayer_Demuxer_FFMPEG::readMore(Stream_Track *track, ui32 StreamID){
    AVPacket packet;
    MediaBuffer_t *mb;
    Stream_Track *other_track;
    
    while (true) {
        i32 res = av_read_frame(mpAVFormat, &packet);
        if (res >= 0) {
            av_dup_packet(&packet);
            if (StreamID == packet.stream_index){
                mb = track->getMediaBuffer();
                setMediaBufferFields(mb, &packet);
                track->enqueueFrame(mb);
                av_free_packet(&packet);
                break;
            }else{
                other_track = static_cast<Stream_Track *>(rbtree_get(mStreamIDRoot, packet.stream_index));
                if (NULL != other_track){
                    TrackInfo_t* info;
                    info = other_track->getInfo();
                    if (info->status == TRACK_PLAY){
                        mb = other_track->getMediaBuffer();
                        setMediaBufferFields(mb, &packet);
                        other_track->enqueueFrame(mb);
                    }
                }
                av_free_packet(&packet);
            }  
        } else {
            AGILE_LOGD("No more packets from ffmpeg, End of stream!");
            return MAG_NOT_ENOUGH_DATA;
        }
    }
    return MAG_NO_ERROR;
}

_status_t   MagPlayer_Demuxer_FFMPEG::readFrameInternal(ui32 StreamID, MediaBuffer_t **buffer){
    void *value;
    Stream_Track *track;
    MediaBuffer_t* mb = NULL;
    _status_t ret = MAG_NO_ERROR;
    
    value = rbtree_get(mStreamIDRoot, StreamID);
    if (NULL != value){
        track =  static_cast<Stream_Track *>(value);
        mb = track->dequeueFrame();

        if (NULL == mb){
            ret = readMore(StreamID);
            if (MAG_NO_ERROR == ret){
                mb = track->dequeueFrame();
            }
        }
    }
    *buffer = mb;
    return ret;
}



