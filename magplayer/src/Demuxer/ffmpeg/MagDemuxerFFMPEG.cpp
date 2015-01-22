#include "MagDemuxerFFMPEG.h"

#include "OMX_Video.h"
#include "OMX_Audio.h"
#include "OMX_AudioExt.h"

#include "libavutil/log.h"

#include <unistd.h>
#include <stdint.h>
#include <limits>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Demuxer"


#define DUMMY_FILE "dummy_file"

static const ui32 kAVIOBufferSize = (32 * 1024);

// #define FFMPEG_DEMUXER_DEBUG

#ifdef FFMPEG_DEMUXER_DEBUG
#define VIDEO_FRAMES_DUMP_FILE "./ffmpeg_video.es"
#define AUDIO_FRAMES_DUMP_FILE "./ffmpeg_audio.es"
#endif

//#define FFMPEG_STREAMBUF_DEBUG

#ifdef FFMPEG_STREAMBUF_DEBUG
#define STREAMBUF_DUMP_FILE "./streambuf.data"
#endif

MagDemuxerFFMPEG::MagDemuxerFFMPEG():
                            mpAVFormat(NULL),
                            mpFormatOpts(NULL),
                            mTotalTrackNum(0)
                            {
    /*av_log_set_level(AV_LOG_DEBUG);*/
    av_log_set_callback(PrintLog_Callback);

    av_register_all();
    avformat_network_init();

    Mag_CreateMutex(&mSeekMutex);
#ifdef FFMPEG_DEMUXER_DEBUG
    mVideoDumpFile = NULL;
    mAudioDumpFile = NULL;
#endif

#ifdef FFMPEG_STREAMBUF_DEBUG
    mStreamBufFile = NULL;
#endif
}

MagDemuxerFFMPEG::~MagDemuxerFFMPEG(){
    AGILE_LOGV("enter!");
    av_log_set_callback(NULL);
    avformat_network_deinit();
    Mag_DestroyMutex(&mSeekMutex);

#ifdef FFMPEG_DEMUXER_DEBUG
    if (NULL != mAudioDumpFile){
        AGILE_LOGV("close audio file.");
        fclose(mAudioDumpFile);
    }
    if (NULL != mVideoDumpFile){
        AGILE_LOGV("close video file.");
        fclose(mVideoDumpFile);
    }
#endif

#ifdef FFMPEG_STREAMBUF_DEBUG
    if (mStreamBufFile != NULL){
        fclose(mStreamBufFile);
    }
#endif
}

void MagDemuxerFFMPEG::PrintLog_Callback(void* ptr, int level, const char* fmt, va_list vl){
    static int print_prefix = 1;
    char line[1024];
    
    /*if (level > AV_LOG_INFO)
        return;*/
    
    av_log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);

    AGILE_LOGD("[ffmpeg library]: %s", line);
}

i32 MagDemuxerFFMPEG::AVIO_Read (ui8 *buf, int buf_size){
    ui32 size = (ui32)buf_size;
    
    if (NULL != mDataSource){
        if (MPCP_OK == mDataSource->Read(buf, &size)){
#ifdef FFMPEG_STREAMBUF_DEBUG
            if (mStreamBufFile == NULL){
                mStreamBufFile = fopen(STREAMBUF_DUMP_FILE, "wb+");
            }

            if (NULL != mStreamBufFile){
                fwrite(buf, 1, size, mStreamBufFile);
            }else{
                AGILE_LOGE("failed to open the file: %s", STREAMBUF_DUMP_FILE);
            }
#endif
            return size;
        }
    }
    return 0;
}

i32 MagDemuxerFFMPEG::AVIO_Write(ui8* buf, int buf_size){
    AGILE_LOGE("The operation is not valid!");
    return 0;
}

i64 MagDemuxerFFMPEG::AVIO_Seek (i64 offset, i32 whence){
    MPCP_ORIGINTYPE flag = MPCP_OriginCur;
    i64 size;
    i64 targetPosition = 0;
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

i32 MagDemuxerFFMPEG::AVIO_Static_Read (void *opaque, ui8 *buf, int buf_size){
    if (NULL == opaque){
        AGILE_LOGE("input void *opaque is NULL!");
        return 0;
    }
    
    return static_cast<MagDemuxerFFMPEG *>(opaque)->AVIO_Read(buf, buf_size);
}

i32 MagDemuxerFFMPEG::AVIO_Static_Write(void *opaque, ui8 *buf, int buf_size){
    if (NULL == opaque){
        AGILE_LOGE("input void *opaque is NULL!");
        return 0;
    }
    
    return static_cast<MagDemuxerFFMPEG *>(opaque)->AVIO_Write(buf, buf_size);
}

i64 MagDemuxerFFMPEG::AVIO_Static_Seek (void *opaque, i64 offset, i32 whence){
    if (NULL == opaque){
        AGILE_LOGE("input void *opaque is NULL!");
        return 0;
    }
    
    return static_cast<MagDemuxerFFMPEG *>(opaque)->AVIO_Seek(offset, whence);
}

AVIOContext *MagDemuxerFFMPEG::ffmpeg_utiles_CreateAVIO(){
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

// AVFormatContext *MagDemuxerFFMPEG::ffmpeg_utiles_CreateAVFormat(AVInputFormat *inputFormat, 
//                                                                                  AVIOContext *context, 
//                                                                                  bool seekable, 
//                                                                                  bool probe){
//     AVFormatContext *format;

//     format = avformat_alloc_context();
//     if (NULL == format){
//         AGILE_LOGE("Failed to create the AVFormatContext!");
//         return NULL;
//     }
    
//     format->pb = context;
//     context->seekable = seekable;

//     if (probe){
//         i32 res;
        
//         res = avformat_open_input(
//             &format,
//             DUMMY_FILE,  /* need to pass a filename*/
//             inputFormat,   /* probe the container format*/
//             NULL);         /* no special parameters*/
        
//         if (res < 0) {
//             AGILE_LOGE("Failed to open the input stream.");
//             av_free(format);
//             return NULL;
//         }

//         res = avformat_find_stream_info(format, NULL);
//         if (res < 0) {
//             AGILE_LOGE("Failed to find stream information.");
//             avformat_close_input(&format);
//             return NULL;
//         }
//     }

//     return format;
// }

void MagDemuxerFFMPEG::ffmpeg_utiles_SetOption(const char *opt, const char *arg){
    const AVOption *o;
    const AVClass *fc = avformat_get_class();

    if ((o = av_opt_find(&fc, opt, NULL, 0,
                          AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ))) {
        av_dict_set(&mpFormatOpts, opt, arg, (o->type == AV_OPT_TYPE_FLAGS) ? AV_DICT_APPEND : 0);
        AGILE_LOGD("[SetOption] - %s:%s [ok]", opt, arg);
    }else{
        AGILE_LOGD("[SetOption] - %s:%s [failure]", opt, arg);
    }
}

enum AVCodecID MagDemuxerFFMPEG::convertCodec_OMXToFFMPEG(ui32 omxCodec, TrackType_t type){
    enum AVCodecID codec = AV_CODEC_ID_NONE;
    
    if (type == TRACK_VIDEO){
        switch(omxCodec){
            case OMX_VIDEO_CodingAVC:
                codec = AV_CODEC_ID_H264;
                break;

            case OMX_VIDEO_CodingMPEG4:
                codec = AV_CODEC_ID_MPEG4;
                break;

            case OMX_VIDEO_CodingMPEG2:
                codec = AV_CODEC_ID_MPEG2VIDEO;
                break;
   
            default:
                AGILE_LOGE("the Video omxCodec type[%d] is not supported!", omxCodec);
                codec = AV_CODEC_ID_NONE;
                break;
        }
    }else if (type == TRACK_AUDIO){
        switch(omxCodec){
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
                AGILE_LOGE("the Audio omxCodec type[%d] is not supported!", omxCodec);
                codec = AV_CODEC_ID_NONE;
                break;
        }
    }else{
        AGILE_LOGE("unrecognized type: %d!", type);
    }
    
    return codec;  
}

ui32 MagDemuxerFFMPEG::convertCodec_FFMPEGToOMX(enum AVCodecID ffmpegCodec, TrackType_t type){
    ui32 omxCodec = 0;
    
    if (type == TRACK_VIDEO){
        switch(ffmpegCodec){
            case AV_CODEC_ID_H264:
                omxCodec = OMX_VIDEO_CodingAVC;
                break;

            case AV_CODEC_ID_MPEG4:
                omxCodec = OMX_VIDEO_CodingMPEG4;
                break;

            case AV_CODEC_ID_MPEG2VIDEO:
                omxCodec = OMX_VIDEO_CodingMPEG2;
                break;
            
            default:
                AGILE_LOGE("the Video ffmpegCodec type[%d] is not supported!", ffmpegCodec);
                omxCodec = OMX_VIDEO_CodingUnused;
                break;
        }
    }else if (type == TRACK_AUDIO){
        switch(ffmpegCodec){
            case AV_CODEC_ID_MP3:
                omxCodec = OMX_AUDIO_CodingMP3;
                break;

            case AV_CODEC_ID_MP2:
                omxCodec = OMX_AUDIO_CodingMP2;
                break;

            case AV_CODEC_ID_AAC:
                omxCodec = OMX_AUDIO_CodingAAC;
                break;
                
            case AV_CODEC_ID_AC3:
                omxCodec = OMX_AUDIO_CodingAC3;
                break;

            case AV_CODEC_ID_EAC3:
                omxCodec = OMX_AUDIO_CodingDDPlus;
                break;
                
            default:
                AGILE_LOGE("the Audio ffmpegCodec type[%d] is not supported!", ffmpegCodec);
                omxCodec = OMX_AUDIO_CodingUnused;
                break;
        }
    }else{
        AGILE_LOGE("unrecognized type: %d!", type);
    }

    return omxCodec;  
}

bool MagDemuxerFFMPEG::checkSupportedStreams(AVStream *st){
    bool supported = true;

    if (st->codec->codec_type == AVMEDIA_TYPE_VIDEO){
        switch(st->codec->codec_id){
            case AV_CODEC_ID_H264:
                break;

            default:
                supported = false;
                break;
        }
    }else if (st->codec->codec_type == AVMEDIA_TYPE_AUDIO){
        i32 audioDisable = 0;

        mParamDB->findInt32(mParamDB, kMediaPlayerConfig_AudioDisable, &audioDisable);
        AGILE_LOGV("Disable audio: %s", audioDisable ? "Yes" : "No");

        if (!audioDisable){
            switch(st->codec->codec_id){
                case AV_CODEC_ID_AAC:
                case AV_CODEC_ID_MP2:
                case AV_CODEC_ID_MP3:
                    break;

                default:
                    supported = false;
                    break;
            }
        }else{
            supported = false;
        }
    }else{
        /*do not support subtitle for now*/
        supported = false;
    }

    if (!supported){
        return false;
    }
    return true;
}

_status_t  MagDemuxerFFMPEG::create_track(TrackInfo_t *track, AVCodecContext* codec){
    Stream_Track *strack = NULL;
    
    strack = new Stream_Track(track);
    if (NULL != strack){
        if (codec){
            /*no decoded video/audio frame formatter needed*/
            strack->setFormatter(NULL);
        }
        mpStreamTrackManager->addStreamTrack(strack);
    }else{
        return MAG_NO_MEMORY;
    }
    return MAG_NO_ERROR;
}

_status_t  MagDemuxerFFMPEG::stop_common(){
    i32 i;
    char keyName[64];

    AGILE_LOGV("Enter!");

    if (NULL == mParamDB){
        AGILE_LOGE("the Demuxer is not initialized. Quit!");
        return MAG_NO_INIT;
    }
    
    for (i = 0; i < mTotalTrackNum; i++){
        sprintf(keyName, kDemuxer_Track_Info, i);
        mParamDB->deleteItem(mParamDB, keyName);
    }
    mTotalTrackNum = 0;

    /*destroy ffmpeg staffs*/
    av_dict_free(&mpFormatOpts);

    if (mpAVFormat){
        AGILE_LOGV("do avformat_close_input()");
        avformat_close_input(&mpAVFormat);
        mpAVFormat = NULL;
    }

    mIsPrepared = MAG_FALSE;
    return MAG_NO_ERROR;
}


#if 0
void  MagDemuxerFFMPEG::copy_noprobe_params(MagMiniDBHandle playerParams){
    i32  number;
    i32  total_tracks = 0;
    i32  i;
    char keyValue[64];
    void *pointer;
    _status_t ret = MAG_NO_ERROR;
    
    if (playerParams->findInt32(playerParams, kDemuxer_Video_Track_Number, &number)){
        AGILE_LOGV("video track number: %d", number);
        setParameters(kDemuxer_Video_Track_Number, MagParamTypeInt32, static_cast<void *>(&number));
        ++total_tracks;
    }else{
        AGILE_LOGE("failed to get the %s parameter!", kDemuxer_Video_Track_Number);
    }

    if (playerParams->findInt32(playerParams, kDemuxer_Audio_Track_Number, &number)){
        AGILE_LOGV("audio track number: %d", number);
        setParameters(kDemuxer_Audio_Track_Number, MagParamTypeInt32, static_cast<void *>(&number));
        ++total_tracks;
    }else{
        AGILE_LOGE("failed to get the %s parameter!", kDemuxer_Audio_Track_Number);
    }

    for(i = 0; i < total_tracks; i++){
        sprintf(keyValue, kDemuxer_Track_Info, i);
        if (playerParams->findPointer(playerParams, keyValue, &pointer)){
            setParameters(keyValue, MagParamTypePointer, pointer);
            playerParams->derefItem(playerParams, keyValue);
            
        }else{
            AGILE_LOGE("failed to get the %s track info!", keyValue);
        }
    }
}
#endif

int MagDemuxerFFMPEG::demux_interrupt_cb(void *ctx)
{
    PlayState_t *is = static_cast<PlayState_t *>(ctx);
    return is->abort_request;
}

void MagDemuxerFFMPEG::fillVideoMetaData(TrackInfo_t *track, AVStream *vStream){
    const char *profile;

    track->meta_data.video.width = vStream->codec->width;
    track->meta_data.video.height = vStream->codec->height;
    if (vStream->avg_frame_rate.den && vStream->avg_frame_rate.num)
        track->meta_data.video.fps = av_q2d(vStream->avg_frame_rate);

    profile = av_get_profile_name(avcodec_find_decoder(vStream->codec->codec_id), vStream->codec->profile);
    if (profile)
        strncpy(track->meta_data.video.codec, profile, sizeof(track->meta_data.video.codec));
    track->meta_data.video.bps = vStream->codec->bit_rate / 1000;
}

void MagDemuxerFFMPEG::fillAudioMetaData(TrackInfo_t *track, AVStream *aStream){
    const char *profile;
    int bits_per_sample;
    int bit_rate;

    profile = av_get_profile_name(avcodec_find_decoder(aStream->codec->codec_id), aStream->codec->profile);
    if (profile)
        strncpy(track->meta_data.audio.codec, profile, sizeof(track->meta_data.audio.codec));
    track->meta_data.audio.hz = aStream->codec->sample_rate;

    bits_per_sample = av_get_bits_per_sample(aStream->codec->codec_id);
    bit_rate = bits_per_sample ? aStream->codec->sample_rate * aStream->codec->channels * bits_per_sample : aStream->codec->bit_rate;
    track->meta_data.audio.bps = bit_rate / 1000;
}

_status_t  MagDemuxerFFMPEG::probe_add_streams(){
    ui32 i;
    i32 vNum = 0;
    i32 aNum = 0;
    i32 sNum = 0; 
    TrackInfo_t *track;
    char keyName[64];
    ui32 track_id = 0;
    
    mParamDB->setInt32(mParamDB, kDemuxer_Bit_Rate, mpAVFormat->bit_rate / 1000);
    mParamDB->setInt64(mParamDB, kDemuxer_Stream_Duration, mpAVFormat->duration / 1000);

    for (i = 0; i < mpAVFormat->nb_streams; i++){
        AVStream *st = mpAVFormat->streams[i];
        if (!checkSupportedStreams(st)){
            AGILE_LOGE("the %s codec 0x%x is not supported, ignore!", 
                        st->codec->codec_type == AVMEDIA_TYPE_VIDEO ? "video" : "audio/other",
                        st->codec->codec_id);
            continue;
        }
        sprintf(keyName, kDemuxer_Track_Info, track_id++);
        track = (TrackInfo_t *)mag_mallocz(sizeof(TrackInfo_t));
        if (track){
            track->pid      = st->id;
            track->streamID = i;
            track->status   = TRACK_STOP;
            track->time_base_num = st->time_base.num;
            track->time_base_den = st->time_base.den;
            track->duration      = (mpAVFormat->duration / 1000);
            track->start_time    = mpAVFormat->start_time;

            track->avformat = mpAVFormat;
            track->avstream = st;

            track->message = createMessage(MagDemuxerBase::MagDemuxerMsg_PlayerNotify);
            track->message->setInt32(track->message, "what", MagDemuxerBase::kWhatReadFrame);

            if (st->codec->codec_type == AVMEDIA_TYPE_VIDEO){
                sprintf(track->name, "Stream%d_Video", i);
                track->codec = convertCodec_FFMPEGToOMX(st->codec->codec_id, TRACK_VIDEO);
                mParamDB->setInt32(mParamDB, kDemuxer_Video_Width, st->codec->width);
                mParamDB->setInt32(mParamDB, kDemuxer_Video_Height, st->codec->height);
                fillVideoMetaData(track, st);
                track->type = TRACK_VIDEO;
                vNum++;
            }else if (st->codec->codec_type == AVMEDIA_TYPE_AUDIO){
                sprintf(track->name, "Stream%d_Audio", i);
                track->codec = convertCodec_FFMPEGToOMX(st->codec->codec_id, TRACK_AUDIO);
                track->type = TRACK_AUDIO;
                fillAudioMetaData(track, st);
                aNum++;
            }else if (st->codec->codec_type == AVMEDIA_TYPE_SUBTITLE){
                sprintf(track->name, "Stream%d_Subtitle", i);
                track->codec = convertCodec_FFMPEGToOMX(st->codec->codec_id, TRACK_SUBTITLE);
                track->type = TRACK_SUBTITLE;
                sNum++;
            }else{
                sprintf(track->name, "Stream%d_Other", i);
                track->codec = 0;
                track->type = TRACK_UNKNOWN;
            }
            create_track(track, st->codec);
            mParamDB->setPointer(mParamDB, keyName, static_cast<void *>(track));

            AGILE_LOGV("track %s: duration[%lld], start time[%lld], time_base[%d:%d]", 
                        track->name, track->duration, track->start_time, track->time_base_num, track->time_base_den);
        }else{
            AGILE_LOGE("failed to create the stream track!");
        }
    }

    if ((vNum + aNum + sNum) == 0){
        AGILE_LOGE("None of stream codec is supported, it is wrong stream!");
        return MAG_BAD_TYPE;
    }else{
        mParamDB->setInt32(mParamDB, kDemuxer_Video_Track_Number, vNum);
        mParamDB->setInt32(mParamDB, kDemuxer_Audio_Track_Number, aNum);
        mParamDB->setInt32(mParamDB, kDemuxer_Subtitle_Track_Number, sNum);
    }
    return MAG_NO_ERROR;
}

int MagDemuxerFFMPEG::check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)
{
    int ret = avformat_match_stream_specifier(s, st, spec);
    if (ret < 0)
        av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
    return ret;
}

AVDictionary *MagDemuxerFFMPEG::filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                                  AVFormatContext *s, AVStream *st, AVCodec *codec)
{
    AVDictionary    *ret = NULL;
    AVDictionaryEntry *t = NULL;
    int            flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
                                      : AV_OPT_FLAG_DECODING_PARAM;
    char          prefix = 0;
    const AVClass    *cc = avcodec_get_class();

    if (!codec)
        codec            = s->oformat ? avcodec_find_encoder(codec_id)
                                      : avcodec_find_decoder(codec_id);

    switch (st->codec->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
        {
            prefix  = 'v';
            flags  |= AV_OPT_FLAG_VIDEO_PARAM;
        }
            break;

        case AVMEDIA_TYPE_AUDIO:
        {
            prefix  = 'a';
            flags  |= AV_OPT_FLAG_AUDIO_PARAM;
        }
            break;

        case AVMEDIA_TYPE_SUBTITLE:
        {
            prefix  = 's';
            flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
        }
            break;

        default:
            break;
    }

    while ((t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX))) {
        char *p = strchr(t->key, ':');

        /* check stream specification in opt name */
        if (p)
            switch (check_stream_specifier(s, st, p + 1)) {
            case  1: *p = 0; break;
            case  0:         continue;
            default:         return NULL;
            }

        if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
            (codec && codec->priv_class &&
             av_opt_find(&codec->priv_class, t->key, NULL, flags,
                         AV_OPT_SEARCH_FAKE_OBJ)))
            av_dict_set(&ret, t->key, t->value, 0);
        else if (t->key[0] == prefix &&
                 av_opt_find(&cc, t->key + 1, NULL, flags,
                             AV_OPT_SEARCH_FAKE_OBJ))
            av_dict_set(&ret, t->key + 1, t->value, 0);

        if (p)
            *p = ':';
    }
    return ret;
}

AVDictionary **MagDemuxerFFMPEG::setup_find_stream_info_opts(AVFormatContext *s,
                                           AVDictionary *codec_opts)
{
    ui32 i;
    AVDictionary **opts;

    if (!s->nb_streams)
        return NULL;
    opts = (AVDictionary **)av_mallocz(s->nb_streams * sizeof(*opts));
    if (!opts) {
        AGILE_LOGE("Could not alloc memory for stream options.");
        return NULL;
    }
    for (i = 0; i < s->nb_streams; i++)
        opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codec->codec_id,
                                    s, s->streams[i], NULL);
    return opts;
}

void MagDemuxerFFMPEG::setOptionsByUrl(const char *url){
    const char *str_3gp = strstr(url, ".3gp" );
    const char *str_3g2 = strstr(url, ".3g2" );
    const char *str_rtsp = strstr(url, "rtsp://" );

    if (((str_3gp != NULL) && !strcmp( str_3gp, ".3gp")) ||
        ((str_3g2 != NULL) && !strcmp( str_3g2, ".3g2"))){
        AGILE_LOGE("It is 3gp/3g2 stream, set it as none seekable.\n");
        ffmpeg_utiles_SetOption("3gp_disable_seek", "1");
    }

    if (str_rtsp != NULL){
        AGILE_LOGE("It is rtsp stream, force to RTSP over TCP.\n");
        ffmpeg_utiles_SetOption("rtsp_transport", "tcp");
    }
}

_status_t  MagDemuxerFFMPEG::probe_prepare(const char *url){
    i32 err;
    AVDictionaryEntry *t;
    AVDictionary **opts;
    AVIOContext *IOcontext = NULL;
    i32 orig_nb_streams = 0;
    i32 i;

    mPlayState.abort_request = 0;

    mpAVFormat = avformat_alloc_context();

    mpAVFormat->interrupt_callback.callback = demux_interrupt_cb;
    mpAVFormat->interrupt_callback.opaque = &mPlayState;

    if (url != NULL){
        AGILE_LOGV("enter. url = %s", url);
    }else{
        AGILE_LOGV("enter. url = DataSource");
    }

    if (NULL == url){
        IOcontext = ffmpeg_utiles_CreateAVIO();
        if (NULL == IOcontext){
            return MAG_NO_MEMORY;
        }
        mpAVFormat->pb = IOcontext;
        err = avformat_open_input(&mpAVFormat, "DataSource", NULL, &mpFormatOpts);
    }else{
        setOptionsByUrl(url);
        AGILE_LOGV("before avformat_open_input");
        err = avformat_open_input(&mpAVFormat, url, NULL, &mpFormatOpts);
        AGILE_LOGV("after avformat_open_input");
    }

    if (err < 0) {
        AGILE_LOGE("failed to do avformat_open_input(%s)(abort: %d), ret = %d", 
                    (url == NULL ? "DataSource" : url), 
                    mPlayState.abort_request,
                    err);

        if (mPlayState.abort_request)
            return MAG_DEMUXER_ABORT;
        else
            return MAG_UNKNOWN_ERROR;
    }else{
        AGILE_LOGD("do avformat_open_input(%s) -- OK!", (url == NULL ? "DataSource" : url));
    }

    if ((t = av_dict_get(mpFormatOpts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        AGILE_LOGE("Option %s not found.\n", t->key);
        if (mPlayState.abort_request)
            return MAG_DEMUXER_ABORT;
        else
            return MAG_NAME_NOT_FOUND;
    }

    opts = setup_find_stream_info_opts(mpAVFormat, mCodecOpts);
    orig_nb_streams = mpAVFormat->nb_streams;
    err = avformat_find_stream_info(mpAVFormat, opts);
    if (err < 0) {
        AGILE_LOGE("could not find codec parameters(abort: %d)\n", mPlayState.abort_request);
        avformat_close_input(&mpAVFormat);
        if (mPlayState.abort_request)
            return MAG_DEMUXER_ABORT;
        else
            return MAG_NAME_NOT_FOUND;
    }

    for (i = 0; i < orig_nb_streams; i++)
        av_dict_free(&opts[i]);
    av_freep(&opts);

    if (url != NULL)
        av_dump_format(mpAVFormat, 0, url, 0);
    else
        av_dump_format(mpAVFormat, 0, "DataSource", 0);

    return probe_add_streams();
}

_status_t  MagDemuxerFFMPEG::probe_stop(){
    return stop_common();
}

_status_t  MagDemuxerFFMPEG::prepare(MagContentPipe *contentPipe, MagBufferObserver *pObserver, MagMiniDBHandle paramDB){
    char *value;
    boolean ret;
    _status_t result;
    _status_t rc = MAG_NO_ERROR;
    
    result = MagDemuxerBaseImpl::prepare(contentPipe, pObserver, paramDB);
    if (result != MAG_NO_ERROR)
        return result;

#ifdef FFMPEG_DEMUXER_DEBUG
    mVideoDumpFile = fopen(VIDEO_FRAMES_DUMP_FILE, "wb+");
    if (NULL != mVideoDumpFile)
        AGILE_LOGE("failed to open the file: %s", VIDEO_FRAMES_DUMP_FILE);
    
    mAudioDumpFile = fopen(AUDIO_FRAMES_DUMP_FILE, "wb+");
    if (NULL != mAudioDumpFile)
        AGILE_LOGE("failed to open the file: %s", AUDIO_FRAMES_DUMP_FILE);
#endif

    mDataSource = contentPipe;
    
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
                    mIsPrepared = MAG_FALSE;
                    return MAG_NO_INIT;
                }
            }
        }
    }else{
        ui32 mode;
        char *url = NULL;

        void *p = static_cast<void *>(&mode);
        AGILE_LOGV("do probe_prepare()");
        if (mDataSource->GetConfig(kMPCPConfigName_WoringMode, MagParamTypeUInt32, &p) == MPCP_OK){
            AGILE_LOGV("kMPCPConfigName_WoringMode is %d", mode);
            if ((MPCP_WORKINGMODE)mode == MPCP_MODE_DUMMY){
                if (mDataSource->GetConfig(kMPCPConfigName_URI, MagParamTypeString, &p) == MPCP_OK){
                    url = static_cast<char *>(p);
                    rc = probe_prepare(url);
                    if (rc == MAG_NO_ERROR)
                        mIsPrepared = MAG_TRUE;
                }else{
                    AGILE_LOGE("failed to get the url and hence quit the stream probing!");
                    return MAG_NAME_NOT_FOUND;
                }
            }else{
                rc = probe_prepare(NULL);
                if (rc == MAG_NO_ERROR)
                    mIsPrepared = MAG_TRUE;
            }
        }
    }
    return rc;
}

_status_t  MagDemuxerFFMPEG::start(){
    _status_t ret;

    ret = MagDemuxerBaseImpl::start();
    if (ret != MAG_NO_ERROR)
        return ret;

    return MAG_NO_ERROR;
}

_status_t  MagDemuxerFFMPEG::stopImpl(){
    char *value;
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
                    return MAG_NO_INIT;
                }
            }
        }
    }

    return probe_stop();
}

_status_t  MagDemuxerFFMPEG::flushImpl(){
    return MAG_NO_ERROR;
}

void MagDemuxerFFMPEG::setMediaBufferFields(MagOmxMediaBuffer_t *mb, AVPacket *packet, TrackInfo_t* info){
    AVRational timeBase;
    AVRational newBase;
    i32 i;

    if (NULL == mb)
        return;

    timeBase.num = info->time_base_num;
    timeBase.den = info->time_base_den;
    newBase.num  = 1;
    newBase.den  = 90000;

    if (packet != NULL){
        mb->buffer      = packet->data;
        mb->buffer_size = packet->size;

        if (packet->pts == AV_NOPTS_VALUE){
            mb->pts = -1; /*any value < 0 means the invalid pts*/
            AGILE_LOGD("invalid pts for track %d", packet->stream_index);
        }else{
            mb->pts = av_rescale_q(packet->pts, timeBase, newBase);
        }
        //(fp64)(info->time_base_num / (fp64) info->time_base_den) * packet->pts * AV_TIME_BASE;
        mb->dts         = av_rescale_q(packet->dts, timeBase, newBase);
        //(fp64)(info->time_base_num / (fp64) info->time_base_den) * packet->dts * AV_TIME_BASE;
        if (AV_PKT_FLAG_KEY == packet->flags)
            mb->flag = STREAM_FRAME_FLAG_KEY_FRAME;
        mb->eosFrame    = 0;

        mb->stream_index = packet->stream_index;
        mb->duration = packet->duration;
        mb->pos = packet->pos;
        mb->side_data_elems = packet->side_data_elems;

        if (packet->side_data_elems){
            mb->side_data = (MediaSideData_t *)mag_mallocz(sizeof(MediaSideData_t)*packet->side_data_elems);
            for (i = 0; i < packet->side_data_elems; i++){
                if (packet->side_data[i].size > 0){
                    mb->side_data[i].data = mag_mallocz(packet->side_data[i].size);
                    memcpy(mb->side_data[i].data, packet->side_data[i].data, packet->side_data[i].size);
                    mb->side_data[i].size = packet->side_data[i].size;
                    mb->side_data[i].type = (ui32)packet->side_data[i].type;
                }
            }
        }

#if 0
        if (TRACK_AUDIO == info->type){
            AGILE_LOGD("[audio] size:%d, pts:0x%llx, duration:%d, pos:%lld, side_data_elems:%d", 
                        mb->buffer_size,
                        mb->pts,
                        mb->duration, 
                        mb->pos,
                        mb->side_data_elems);
        }else if (TRACK_VIDEO == info->type){
            AGILE_LOGD("[video] size:%d, pts:0x%llx, duration:%d, pos:%lld, side_data_elems:%d [%s]", 
                        mb->buffer_size,
                        mb->pts,
                        mb->duration, 
                        mb->pos,
                        mb->side_data_elems,
                        mb->flag == STREAM_FRAME_FLAG_KEY_FRAME ? "I" : "P/B");
        }
#endif
    }else{
        /*this is the generated EOS frame*/
        mb->buffer      = NULL;
        mb->buffer_size = 0;
        mb->pts         = 0;
        mb->dts         = 0;
        mb->eosFrame    = 1;
    }
}

void MagDemuxerFFMPEG::fillMediaBuffer(Stream_Track *track, AVPacket *pPacket){
    _status_t ret;
    MagOmxMediaBuffer_t *mb;
    TrackInfo_t* info;

    info = track->getInfo();
    mb = track->getMediaBuffer();
    if (mb != NULL){
        setMediaBufferFields(mb, pPacket, info);
        ret = track->enqueueFrame(mb);
        if (ret != MAG_NO_ERROR){
            AGILE_LOGE("the stream track %s is full or flushed, QUIT!!", info->name);
        }
    }else{
        AGILE_LOGE("the stream track %s is flushed!", info->name);
    }

    if (pPacket)
        av_free_packet(pPacket);
}

_status_t MagDemuxerFFMPEG::readMore(Stream_Track *track, ui32 StreamID){
    AVPacket packet;
    AVPacket *pPacket = &packet;
    MagOmxMediaBuffer_t *mb;
    Stream_Track *other_track = NULL;
    i32 res;
    _status_t ret;

    while (true) {
        if (mpStreamTrackManager->interruptReading()){
            AGILE_LOGD("command to exit the frame reading!");
            return MAG_READ_ABORT;
        }

        if (Mag_TryAcquireMutex(mSeekMutex) != MAG_ErrNone){
            AGILE_LOGD("the mSeekMutex is locked by seek action, return!");
            return MAG_NO_ERROR;
        }
        
        res = av_read_frame(mpAVFormat, pPacket);
        Mag_ReleaseMutex(mSeekMutex);

        if (res >= 0) {
            av_dup_packet(pPacket);
#ifdef FFMPEG_DEMUXER_DEBUG
            if (mpAVFormat->streams[pPacket->stream_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
                if (NULL != mAudioDumpFile){
                    fwrite(pPacket->data, 1, pPacket->size, mAudioDumpFile);
                }
            }else{
                if (NULL != mVideoDumpFile){
                    fwrite(pPacket->data, 1, pPacket->size, mVideoDumpFile);
                }
            }
#endif

            if (StreamID == (ui32)pPacket->stream_index){
                fillMediaBuffer(track, pPacket);
                break;
            }else{
                other_track = mpStreamTrackManager->getStreamTrack(pPacket->stream_index);
                if (NULL != other_track){
                    TrackInfo_t* info;

                    info = other_track->getInfo();
                    if (info->status == TRACK_PLAY){
                        mb = other_track->getMediaBuffer();
                        if (mb != NULL){
                            setMediaBufferFields(mb, pPacket, info);
                            ret = other_track->enqueueFrame(mb);
                            if (ret != MAG_NO_ERROR){
                                AGILE_LOGE("the stream track %s is full, QUIT!!", info->name);
                                av_free_packet(pPacket);
                                break;
                            }
                        }else{
                            AGILE_LOGE("the stream track %s is flushed!", info->name);
                            av_free_packet(pPacket);
                            break;
                        }
                    }
                }
                av_free_packet(pPacket);
            }
        } else {
            if ( (res == AVERROR_EOF || url_feof(mpAVFormat->pb)) ||
                 (mpAVFormat->pb && mpAVFormat->pb->error) ){

                ui32 pStreamIdList[64];
                ui32 playStreamsNum;
                ui32 i;
                Stream_Track *tmpTrack;

                AGILE_LOGI("%s", 
                    (mpAVFormat->pb && mpAVFormat->pb->error) ? "ffmpeg data reading error happens!!!": 
                    res == AVERROR_EOF ? "reach to the stream END!!!" : url_feof(mpAVFormat->pb) ? "url_feof() returns true" : "false eof!");

                playStreamsNum = mpStreamTrackManager->getPlayingStreamsID(pStreamIdList);
                for (i = 0; i < playStreamsNum; i++){
                    tmpTrack = mpStreamTrackManager->getStreamTrack(pStreamIdList[i]);
                    fillMediaBuffer(tmpTrack, NULL);
                }
                setEOS();
                av_free_packet(pPacket);
                /*stop the frame parsing.*/
                return MAG_NO_MORE_DATA;
            }

            AGILE_LOGD("res = %d, No packet reading from ffmpeg!", res);
            return MAG_NOT_ENOUGH_DATA;
        }
    }
    return MAG_NO_ERROR;
}

_status_t   MagDemuxerFFMPEG::readFrameImpl(Stream_Track *track, ui32 StreamID){
    return readMore(track, StreamID);
}

_status_t   MagDemuxerFFMPEG::seekToImpl(i32 msec, i64 mediaTime, TrackInfo_t *track){
    i32 seekFlag = 0;
    i64 seektime;
    i64 seekTarget = 0;
    i32 res = 0;
    _status_t ret = MAG_NO_ERROR;
    i64 seek_min;
    i64 seek_max;

    AGILE_LOGV("Enter!");

    /*if (msec < 0){
        AGILE_LOGE("invalid seek time: %d", msec);
        return MAG_BAD_VALUE;
    }*/

    Mag_AcquireMutex(mSeekMutex);
    seektime = static_cast<i64>(msec);

    /*if (seektime > track->duration){
        AGILE_LOGE("seek time %lld ms > the stream duration %lld ms!", seektime, track->duration);
        seektime = track->duration;
    }else{
        if (static_cast<fp64>(seektime) / static_cast<fp64>(track->duration) > 0.98){
            AGILE_LOGD("seek target: %lld ms VS. duration: %lld ms, near to the end and seek backward!!", 
                        seektime, track->duration);
            seekFlag |= AVSEEK_FLAG_BACKWARD;
        }
    }*/

    // else if (seektime < track->start_time){
    //     AGILE_LOGE("seek time %d ms < the stream start time %d ms!", seektime, track->start_time);
    //     seektime = track->start_time;
    // }

    /*A time t in units of a/b(AV_TIME_BASE_Q[us]) can be converted to units c/d(timeBase)*/
    /*seekTarget = av_rescale_q(seekTimeInStBase + track->start_time, timeBase, AV_TIME_BASE_Q);*/
    seekTarget = mediaTime + seektime * 1000;

    seek_min    = seektime > 0 ? seekTarget - seektime + 2 : std::numeric_limits<int64_t>::min();
    seek_max    = seektime < 0 ? seekTarget - seektime - 2: std::numeric_limits<int64_t>::max();

    AGILE_LOGV("before avformat_seek_file(-1)[seekTarget: %lld, seek_min: %lld, seek_max: %lld][mediaTime: %lld]", 
                seekTarget, seek_min, seek_max, mediaTime);
    /*res = av_seek_frame(mpAVFormat, track->streamID, seekTarget, seekFlag);*/
    res = avformat_seek_file(mpAVFormat, -1, seek_min, seekTarget, seek_max, seekFlag);
    if (res < 0) {
        AGILE_LOGE("seek %s to %lld ms[ffmpeg seek target %lld] failed, err code = %d", 
                    seekFlag ? "backward" : "forward",
                    seektime, seekTarget,
                    res);

        ret = MAG_UNKNOWN_ERROR;
    }else{
        AGILE_LOGD("seek %s to %lld ms[ffmpeg seek target %lld] -- OK!", 
                    seekFlag ? "backward" : "forward",
                    seektime, seekTarget);
    }
    Mag_ReleaseMutex(mSeekMutex);

    return ret;
}


