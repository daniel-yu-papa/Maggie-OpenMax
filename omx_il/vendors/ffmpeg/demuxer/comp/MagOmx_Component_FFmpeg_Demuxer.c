/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MagOmx_Component_FFmpeg_Demuxer.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME "OMX.Mag.demuxer.ffmpeg"

AllocateClass(MagOmxComponent_FFmpeg_Demuxer, MagOmxComponentDemuxer);

static void ffmpeg_utiles_PrintLogCallback(void* ptr, int level, const char* fmt, va_list vl){
    static int print_prefix = 1;
    char line[1024];
    
    /*if (level > AV_LOG_INFO)
        return;*/
    
    av_log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);

    AGILE_LOGD("[ffmpeg library]: %s", line);
}

static AVIOContext *ffmpeg_utiles_CreateAVIO(MAG_DEMUXER_DATA_SOURCE *ds){
    OMX_U32 size;
    OMX_U8  *buffer;
    AVIOContext *context;
    
    size   = kAVIOBufferSize + FF_INPUT_BUFFER_PADDING_SIZE;
    buffer = (OMX_U8 *)(mag_malloc(size));

    if (NULL == buffer){
        AGILE_LOGE("Failed to create the AVIO buffer for ffmpeg!");
        return NULL;
    }
    
    context = avio_alloc_context
                    (buffer,
                     size ,
                     0,
                     ds->opaque,
                     ds->Data_Read,
                     ds->Data_Write,
                     ds->Data_Seek);
        
    if (NULL == context){
        AGILE_LOGE("Failed to create the AVIO context!");
        mag_free(buffer);
        return NULL;
    }
    return context;
}

static int ffmpeg_utiles_CheckStreamSpecifier(AVFormatContext *s, AVStream *st, const char *spec)
{
    int ret = avformat_match_stream_specifier(s, st, spec);
    if (ret < 0)
        AGILE_LOGE("Invalid stream specifier: %s", spec);
    return ret;
}

static AVDictionary *ffmpeg_utiles_FilterCodecOpts(AVDictionary *opts, enum AVCodecID codec_id,
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
            switch (ffmpeg_utiles_CheckStreamSpecifier(s, st, p + 1)) {
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

static AVDictionary **ffmpeg_utiles_SetupStreamInfoOpts(AVFormatContext *s,
                                                  AVDictionary *codec_opts)
{
    int i;
    AVDictionary **opts;

    if (!s->nb_streams)
        return NULL;
    opts = (AVDictionary **)av_mallocz(s->nb_streams * sizeof(*opts));
    if (!opts) {
        AGILE_LOGE("Could not alloc memory for stream options.");
        return NULL;
    }
    for (i = 0; i < s->nb_streams; i++)
        opts[i] = ffmpeg_utiles_FilterCodecOpts(codec_opts, s->streams[i]->codec->codec_id,
                                    s, s->streams[i], NULL);
    return opts;
}

static OMX_U32 ffmpeg_utiles_ConvertCodecIdToOMX(enum AVCodecID ffmpegCodec, enum AVMediaType type){
    OMX_U32 omxCodec = 0;
    
    if (type == AVMEDIA_TYPE_VIDEO){
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
    }else if (type == AVMEDIA_TYPE_AUDIO){
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

static void ffmpeg_utiles_ReportStreamInfo(MagOmxComponent_FFmpeg_Demuxer thiz,
                                           MAG_DEMUXER_DATA_SOURCE *dataSource, 
                                           cbNewStreamAdded callBack){
    OMX_DEMUXER_STREAM_INFO *pStInfo;
    OMX_U32    i;
    AVStream   *st;
    const char *profile;
    AVFormatContext  *avFormat;

    avFormat = (AVFormatContext *)dataSource->hDemuxer;
    for (i = 0; i < avFormat->nb_streams; i++){
        st        = avFormat->streams[i];
        stream_id = i + thiz->mStreamIndex;
        pStInfo = &thiz->mStreamInfo[stream_id];

        if (st->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            pStInfo->type = OMX_DynamicPort_Video;
            pStInfo->format.video.witdh  = st->codec->width;
            pStInfo->format.video.height = st->codec->height;

            if (st->avg_frame_rate.den && st->avg_frame_rate.num)
                pStInfo->format.video.fps = av_q2d(st->avg_frame_rate);
            else
                pStInfo->format.video.fps = 0;

            pStInfo->format.video.bps    = st->codec->bit_rate / 1000;

            profile = av_get_profile_name(avcodec_find_decoder(st->codec->codec_id), st->codec->profile);
            if (profile)
                strncpy(pStInfo->codec_info, profile, sizeof(pStInfo->codec_info));

            pStInfo->codec_id = ffmpeg_utiles_ConvertCodecIdToOMX(st->codec->codec_id, AVMEDIA_TYPE_VIDEO);
        }else if (st->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            int bits_per_sample;
            int bit_rate;

            pStInfo->type = OMX_DynamicPort_Audio;
            profile = av_get_profile_name(avcodec_find_decoder(st->codec->codec_id), st->codec->profile);
            if (profile)
                strncpy(pStInfo->codec_info, profile, sizeof(pStInfo->codec_info));

            pStInfo->format.audio.hz = st->codec->sample_rate;

            bits_per_sample = av_get_bits_per_sample(st->codec->codec_id);
            bit_rate = bits_per_sample ? st->codec->sample_rate * st->codec->channels * bits_per_sample : aStream->codec->bit_rate;
            pStInfo->format.audio.bps = bit_rate / 1000;

            pStInfo->codec_id = ffmpeg_utiles_ConvertCodecIdToOMX(st->codec->codec_id, AVMEDIA_TYPE_AUDIO);
        }else if (st->codec->codec_type == AVMEDIA_TYPE_SUBTITLE){
            pStInfo->type = OMX_DynamicPort_Subtitle;
        }else{
            AGILE_LOGE("Stream%d_Other", i);
            continue;
        }

        pStInfo->time_base_num = st->time_base.num;
        pStInfo->time_base_den = st->time_base.den;
        pStInfo->duration      = avFormat->duration / 1000;
        pStInfo->stream_id     = i;
        pStInfo->stream_hanlde = (OMX_PTR)st;

        if (dataSource->hPort){
            pStInfo->url_data_source = OMX_FALSE;
        }else{
            pStInfo->url_data_source = OMX_TRUE;
        }

        dataSource->streamNumber = avFormat->nb_streams;
        *dataSource->streamTable = (MAG_DEMUXER_PORT_DESC *)mag_mallocz(sizeof(MAG_DEMUXER_PORT_DESC *) * avFormat->nb_streams);

        callBack(thiz, dataSource, pStInfo);
    }

    /*report the end of streams list*/
    callBack(thiz, NULL, NULL);

    thiz->mStreamIndex += avFormat->nb_streams;
}

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponent demuxComp;

    AGILE_LOGV("enter!");

    av_log_set_callback(ffmpeg_utiles_PrintLogCallback);

    av_register_all();
    avformat_network_init();

    demuxComp = ooc_cast(hComponent, MagOmxComponent);
    demuxComp->setName(demuxComp, (OMX_U8 *)COMPONENT_NAME);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Demuxer_DetectStreams(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    MAG_DEMUXER_DATA_SOURCE *pDataSource,
                                    OMX_IN cbNewStreamAdded fn){
    MagOmxComponent_FFmpeg_Demuxer thiz;
    AVIOContext *avioContext = NULL;
    OMX_S32 err;
    AVDictionaryEntry *t;
    AVDictionary **opts;
    OMX_S32 orig_nb_streams = 0;
    AVFormatContext  *avFormat;
    AVDictionary     *avFormatOpts;
    AVDictionary     *avCodecOpts;

    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Demuxer);

    avFormat = avformat_alloc_context();

    avFormat->interrupt_callback.callback = thiz->demux_interrupt_cb;
    avFormat->interrupt_callback.opaque   = &thiz->mPlayState;

    if (pDataSource->hPort == NULL){
        /*directly retrieve the data from URL*/
        err = avformat_open_input(&avFormat, 
                                  pDataSource->url, 
                                  NULL, 
                                  &avFormatOpts);
    }else{
        avioContext = ffmpeg_utiles_CreateAVIO(pDataSource);
        if (NULL == avioContext){
            return OMX_ErrorInsufficientResources;
        }
        avFormat->pb = avioContext;
        err = avformat_open_input(&avFormat,
                                  "DataSource", 
                                  NULL, 
                                  &avFormatOpts);
    }

    if (err < 0) {
        AGILE_LOGE("failed to do avformat_open_input(%s)(abort: %d), ret = %d", 
                    pDataSource->url, 
                    thiz->mPlayState.abort_request,
                    err);

        return OMX_ErrorUndefined;
    }else{
        AGILE_LOGD("do avformat_open_input(%s) -- OK!", pDataSource->url);
    }

    if ((t = av_dict_get(avFormatOpts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        AGILE_LOGE("Option %s not found.\n", t->key);
        return OMX_ErrorUndefined;
    }

    opts = ffmpeg_utiles_SetupStreamInfoOpts(avFormat, avCodecOpts);
    orig_nb_streams = avFormat->nb_streams;
    err = avformat_find_stream_info(avFormat, opts);
    if (err < 0) {
        AGILE_LOGE("could not find codec parameters(abort: %d)\n", thiz->mPlayState.abort_request);
        avformat_close_input(&avFormat);
        return OMX_ErrorUndefined;
    }

    for (i = 0; i < orig_nb_streams; i++)
        av_dict_free(&opts[i]);
    av_freep(&opts);

    pDataSource->hDemuxer     = (OMX_PTR)avFormat;
    pDataSource->hDemuxerOpts = (OMX_PTR)avFormatOpts;
    pDataSource->hCodecOpts   = (OMX_PTR)avCodecOpts;

    ffmpeg_utiles_ReportStreamInfo(thiz, pDataSource, fn);
    av_dump_format(avFormat, 0, pDataSource->url, 0);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Demuxer_ReadFrame(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN MAG_DEMUXER_DATA_SOURCE *pDataSource){
    OMX_S32 res;
    AVPacket packet;
    AVPacket *pPacket = &packet;
    AVFormatContext  *avFormat;
    MAG_DEMUXER_AVFRAME *avFrame;
    MagOmxComponentDemuxer parent;
    MagOmxComponent_FFmpeg_Demuxer thiz;

    parent = ooc_cast(hComponent, MagOmxComponentDemuxer);
    thiz   = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Demuxer);

    avFormat = (AVFormatContext *)pDataSource->hDemuxer;

    res = av_read_frame(avFormat, pPacket);

    avFrame = parent->getAVFrame(parent);
    if (res >= 0) {
        av_dup_packet(pPacket);
        thiz->fillAVFrame(thiz, pPacket, &avFrame->frame, pDataSource);
    }else{
        if ( (res == AVERROR_EOF || url_feof(avFormat->pb)) ||
                 (avFormat->pb && avFormat->pb->error) ){
            thiz->fillAVFrame(thiz, NULL, &avFrame->frame, pDataSource);
        }
    }

    pDataSource->sendFrameMessage->setPointer(pDataSource->sendFrameMessage, "av_frame", avFrame, MAG_FALSE);
    pDataSource->sendFrameMessage->postMessage(pDataSource->sendFrameMessage, 0);

    return OMX_ErrorNone;
}

static int FFmpeg_Demuxer_demux_interrupt_cb(void *ctx){
    MAG_FFMPEG_DEMUXER_PLAY_STATE *is = (MAG_FFMPEG_DEMUXER_PLAY_STATE *)(ctx);
    return is->abort_request;
}

static void FFmpeg_Demuxer_fillAVFrame(MagOmxComponent_FFmpeg_Demuxer thiz, 
                                       AVPacket *pPacket, 
                                       OMX_DEMUXER_AVFRAME *avFrame, 
                                       OMX_IN MAG_DEMUXER_DATA_SOURCE *pDataSource){
    AVRational timeBase;
    AVRational newBase;

    timeBase.num = pDataSource->time_base_num;
    timeBase.den = pDataSource->time_base_den;
    newBase.num  = 1;
    newBase.den  = 90000;

    if (pPacket != NULL){
        avFrame->buffer = pPacket->data;
        avFrame->size   = pPacket->size;

        if (pPacket->pts == AV_NOPTS_VALUE){
            avFrame->pts = -1; /*any value < 0 means the invalid pts*/
            AGILE_LOGD("invalid pts in stream %d", pPacket->stream_index);
        }else{
            avFrame->pts = av_rescale_q(pPacket->pts, timeBase, newBase);
        }
        avFrame->dts         = av_rescale_q(pPacket->dts, timeBase, newBase);
        if (AV_PKT_FLAG_KEY == pPacket->flags)
            avFrame->flag = MAG_AVFRAME_FLAG_KEY_FRAME;

        avFrame->stream_id = pPacket->stream_index;
        avFrame->duration  = pPacket->duration;
        avFrame->position  = pPacket->pos;
    }else{
        /*this is the generated EOS frame*/
        avFrame->buffer = NULL;
        avFrame->size   = 0;
        avFrame->pts    = 0;
        avFrame->dts    = 0;
        avFrame->flag   = MAG_AVFRAME_FLAG_EOS;
    }
    avFrame->priv = pPacket;
    avFrame->releaseFrame = thiz->releaseAVFrame;
}

static void FFmpeg_Demuxer_releaseAVFrame(OMX_HANDLETYPE hComponent, OMX_PTR frame){
    AVPacket *pPacket;
    MagOmxComponentDemuxer *demuxer;
    OMX_DEMUXER_AVFRAME *avframe;
    MAG_DEMUXER_AVFRAME *avframeItem;

    demuxer = ooc_cast(hComponent, MagOmxComponentDemuxer);

    avframe = (OMX_DEMUXER_AVFRAME *)frame;
    avframeItem = (MAG_DEMUXER_AVFRAME *)avframe->opaque;
    pPacket = (AVPacket *)avframe->priv;

    av_free_packet(pPacket);
    demuxer->putAVFrame(demuxer, avframeItem);
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_FFmpeg_Demuxer_initialize(Class this){
    AGILE_LOGV("Enter!");
    
    /*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_DemuxerVtableInstance.MagOmxComponentDemuxer.MagOMX_Demuxer_DetectStreams = virtual_FFmpeg_Demuxer_DetectStreams;
    MagOmxComponent_FFmpeg_DemuxerVtableInstance.MagOmxComponentDemuxer.MagOMX_Demuxer_ReadFrame     = virtual_FFmpeg_Demuxer_ReadFrame;
}

static void MagOmxComponent_FFmpeg_Demuxer_constructor(MagOmxComponent_FFmpeg_Demuxer thiz, const void *params){
    AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Demuxer));
    chain_constructor(MagOmxComponent_FFmpeg_Demuxer, thiz, params);

    thiz->demux_interrupt_cb = FFmpeg_Demuxer_demux_interrupt_cb;
    thiz->releaseAVFrame     = FFmpeg_Demuxer_releaseAVFrame;

    INIT_LIST(&thiz->mDataSourceList);
    Mag_CreateMutex(&thiz->mhFFMpegMutex);
}

static void MagOmxComponent_FFmpeg_Demuxer_destructor(MagOmxComponent_FFmpeg_Demuxer thiz, MagOmxComponent_FFmpeg_DemuxerVtable vtab){
    AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhFFMpegMutex);
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Demuxer_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                                  OMX_IN  OMX_PTR pAppData,
                                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
    MagOmxComponent_FFmpeg_Demuxer hDemuxComp;
    MagOmxComponentImpl     parent;
    OMX_U32 param[1];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_Demuxer);

    param[0] = MAG_DEMUX_START_PORT_INDEX;

    hDemuxComp = (MagOmxComponent_FFmpeg_Demuxer) ooc_new( MagOmxComponent_FFmpeg_Demuxer, (void *)param);
    MAG_ASSERT(hDemuxComp);

    parent = ooc_cast(hDemuxComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hDemuxComp, pAppData, pCallBacks);
    if (*hComponent){
        return localSetupComponent(hDemuxComp);
    }else{
        return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Demuxer_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
    OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
    MagOmxComponent_FFmpeg_Demuxer hDemuxComp;

    AGILE_LOGD("enter!");
    hDemuxComp = (MagOmxComponent_FFmpeg_Demuxer)compType->pComponentPrivate;
    ooc_delete((Object)hDemuxComp);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
    static char * roles[] = {OMX_ROLE_CONTAINER_DEMUXER_EXT_AUTO, 
                             OMX_ROLE_CONTAINER_DEMUXER_3GP,
                             OMX_ROLE_CONTAINER_DEMUXER_ASF,
                             OMX_ROLE_CONTAINER_DEMUXER_REAL};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 4, MagOmxComponent_FFmpeg_Demuxer_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
    MagOmxComponent_FFmpeg_Demuxer_DeInit(hComponent);
}

#undef COMPONENT_NAME