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

#include "MagOmx_Pipeline_FFmpeg_Playback.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_PipelineVendor"

#define COMPONENT_NAME "OMX.Mag.pl.playback.ffmpeg"
#define START_PORT_INDEX kCompPortStartNumber

#define VIDEO_PIPELINE_BUFFER_NUMBER 16
#define AUDIO_PIPELINE_BUFFER_NUMBER 32

AllocateClass(MagOmxPipeline_FFmpeg_Playback, MagOmxPipelinePlayback);

static OMX_STRING localDecideVideoRole(OMX_U32 codec_id){
    switch(codec_id){
        case OMX_VIDEO_CodingAVC:
            return OMX_ROLE_VIDEO_DECODER_AVC;

        case OMX_VIDEO_CodingMPEG4:
            return OMX_ROLE_VIDEO_DECODER_MPEG4;

        case OMX_VIDEO_CodingMPEG2:
            return OMX_ROLE_VIDEO_DECODER_EXT_MPEG2;

        case OMX_VIDEO_CodingWMV:
            return OMX_ROLE_VIDEO_DECODER_WMV;

        case OMX_VIDEO_CodingMJPEG:
            return OMX_ROLE_VIDEO_DECODER_EXT_MJPEG;

        case OMX_VIDEO_CodingH263:
            return OMX_ROLE_VIDEO_DECODER_H263;

        default:
            AGILE_LOGE("Unrecognized Video OMX Codec: 0x%x", OMXCodec);
            return NULL;
    }
}

static OMX_STRING localDecideAudioRole(OMX_U32 codec_id){
    switch(codec_id){
        case OMX_AUDIO_CodingAAC:
            return OMX_ROLE_AUDIO_DECODER_AAC;

        case OMX_AUDIO_CodingMP3:
            return OMX_ROLE_AUDIO_DECODER_MP3;

        case OMX_AUDIO_CodingMP2:
            return OMX_ROLE_AUDIO_DECODER_EXT_MP2;

        case OMX_AUDIO_CodingAC3:
            return OMX_ROLE_AUDIO_DECODER_EXT_AC3;

        case OMX_AUDIO_CodingDDPlus:
            return OMX_ROLE_AUDIO_DECODER_EXT_DDPLUS;

        case OMX_AUDIO_CodingDTS:
            return OMX_ROLE_AUDIO_DECODER_EXT_DTS;

        default:
            AGILE_LOGE("Unrecognized Audio OMX Codec: 0x%x", OMXCodec);
            return NULL;
    }
}

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Playback_CreateCodecPipeline(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_DYNAMIC_PORT_TYPE plt,
                                    OMX_IN OMX_PTR pAppData,
                                    OMX_IN OMX_CALLBACKTYPE *pCb,
                                    OMX_OUT MagOmxPipelineCodec *phCodecPipeline){
    MagOmxPipelineCodec pl_codec;
    OMX_ERRORTYPE err;
    char compName[128];
    OMX_CODEC_PIPELINE_SETTING setting;
    OMX_CODEC_PIPELINE_COMP_PARAM param_table[3];
    CodecPipelineEntry *ple;
    OMX_U32 i;

    ple = (CodecPipelineEntry *)pAppData;

    if (plt == OMX_DynamicPort_Video){
        err = OMX_ComponentOfRoleEnum(compName, OMX_ROLE_PIPELINE_VIDEO_DECODER, 1);
        if (err == OMX_ErrorNone){
            err = OMX_GetHandle(&pl_codec, compName, pAppData, pCb);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return err;
            }

            param_table[0].role          = localDecideVideoRole(ple->portDesc->stream_info->codec_id);
            param_table[0].stream_handle = ple->portDesc->stream_info->stream_hanlde;

            param_table[1].role          = OMX_ROLE_VIDEO_SCHEDULER_BINARY;
            param_table[1].stream_handle = NULL;

            param_table[2].role          = OMX_ROLE_IV_RENDERER_YUV_BLTER;
            param_table[2].stream_handle = NULL;
            for (i = 0; i < 3; i++){
                param_table[i].buffer_number = VIDEO_PIPELINE_BUFFER_NUMBER;
                param_table[i].buffer_size   = 0;
                param_table[i].type          = OMX_COMPONENT_VIDEO;
                param_table[i].codec_id      = ple->portDesc->stream_info->codec_id;
            }
            
            initHeader(&setting, sizeof(OMX_CODEC_PIPELINE_SETTING));
            setting.domain   = OMX_PortDomainVideo;
            setting.compList = param_table;
            setting.compNum  = 3;

            err = OMX_SetParameter(pl_codec, OMX_IndexParamCodecPipelineSetting, &setting);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Failed to set video codec pipeline OMX_IndexParamCodecPipelineSetting parameter");
                return err;
            }
        }else{
            AGILE_LOGE("Failed to get the component name with role %s", OMX_ROLE_PIPELINE_VIDEO_DECODER);
        }
    }else if (plt == OMX_DynamicPort_Audio){
        err = OMX_ComponentOfRoleEnum(compName, OMX_ROLE_PIPELINE_AUDIO_DECODER, 1);
        if (err == OMX_ErrorNone){
            err = OMX_GetHandle(&pl_codec, compName, pAppData, pCb);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return err;
            }

            param_table[0].role          = localDecideAudioRole(ple->portDesc->stream_info->codec_id);
            param_table[0].stream_handle = ple->portDesc->stream_info->stream_hanlde;

            param_table[1].role          = OMX_ROLE_AUDIO_RENDERER_PCM;
            param_table[1].stream_handle = NULL;
            for (i = 0; i < 2; i++){
                param_table[i].buffer_number = AUDIO_PIPELINE_BUFFER_NUMBER;
                param_table[i].buffer_size   = 0;
                param_table[i].type          = OMX_COMPONENT_AUDIO;
                param_table[i].codec_id      = ple->portDesc->stream_info->codec_id;
            }
            
            initHeader(&setting, sizeof(OMX_CODEC_PIPELINE_SETTING));
            setting.domain   = OMX_PortDomainAudio;
            setting.compList = param_table;
            setting.compNum  = 2;

            err = OMX_SetParameter(pl_codec, OMX_IndexParamCodecPipelineSetting, &setting);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Failed to set audio codec pipeline OMX_IndexParamCodecPipelineSetting parameter");
                return err;
            }
        }else{
            AGILE_LOGE("Failed to get the component name with role %s", OMX_ROLE_PIPELINE_AUDIO_DECODER);
        }
    }else{
        AGILE_LOGE("unsupported codec pipline type[%d]", plt);
    }
    *phCodecPipeline = pl_codec;

    return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPipeline_FFmpeg_Playback_initialize(Class this){
    AGILE_LOGV("Enter!");
    
    /*Override the base component pure virtual functions*/
    MagOmxPipeline_FFmpeg_PlaybackVtableInstance.MagOmxPipelinePlayback.MagOMX_Playback_CreateCodecPipeline  = virtual_FFmpeg_Playback_CreateCodecPipeline;
}

static void MagOmxPipeline_FFmpeg_Playback_constructor(MagOmxPipeline_FFmpeg_Playback thiz, const void *params){
    AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxPipeline_FFmpeg_Playback));
    chain_constructor(MagOmxPipeline_FFmpeg_Playback, thiz, params);
}

static void MagOmxPipeline_FFmpeg_Playback_destructor(MagOmxPipeline_FFmpeg_Playback thiz, MagOmxPipeline_FFmpeg_PlaybackVtable vtab){
    AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxPipeline_FFmpeg_Playback_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                                         OMX_IN  OMX_PTR pAppData,
                                                         OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
    MagOmxPipeline_FFmpeg_Playback hPlayback;
    MagOmxComponentImpl     parent;
    OMX_U32 param[1];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxPipeline_FFmpeg_Playback);

    param[0] = START_PORT_INDEX;
    /*param[1] = PORT_NUMBER;*/

    hPlayback = (MagOmxPipeline_FFmpeg_Playback) ooc_new( MagOmxPipeline_FFmpeg_Playback, (void *)param);
    MAG_ASSERT(hPlayback);

    parent = ooc_cast(hPlayback, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hPlayback, pAppData, pCallBacks);
    if (*hComponent){
        return localSetupComponent(hPlayback);
    }else{
        return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxPipeline_FFmpeg_Playback_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
    OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
    MagOmxPipeline_FFmpeg_Playback hPlayback;

    AGILE_LOGD("enter!");
    hPlayback = (MagOmxPipeline_FFmpeg_Playback)compType->pComponentPrivate;
    ooc_delete((Object)hPlayback);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
    static char * roles[] = {OMX_ROLE_PIPELINE_PLAYBACK};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxPipeline_FFmpeg_Playback_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
    MagOmxPipeline_FFmpeg_Playback_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef START_PORT_INDEX