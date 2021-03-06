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

#ifndef OMX_Audio_Ext_h
#define OMX_Audio_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Each OMX header must include all required header files to allow the
 *  header to compile without errors.  The includes below are required
 *  for this header file to compile successfully
 */

#include <OMX_Core.h>


typedef enum OMX_AUDIO_CODINGEXTTYPE {
    OMX_AUDIO_ExtCodingUnused =OMX_AUDIO_CodingVendorStartUnused,
    OMX_AUDIO_CodingAC3,         /**< Any variant of AC3 encoded data */
    OMX_AUDIO_CodingDTS,         /**< Any variant of DTS encoded data */
    OMX_AUDIO_CodingMP1,         /**< Any variant of MP1 encoded data */
    OMX_AUDIO_CodingMP2,         /**< Any variant of MP2 encoded data */
    OMX_AUDIO_CodingDDPlus       /**< Any variant of DD+ encoded data */
} OMX_AUDIO_CODINGEXTTYPE;


typedef enum OMX_AUDIO_PCMMODEEXTTYPE {
    OMX_AUDIO_PCMModeExtUnused=OMX_AUDIO_PCMModeVendorStartUnused,
    OMX_AUDIO_PCMModeBluray    /**< Blu-ray LPCM */
} OMX_AUDIO_PCMMODEEXTTYPE;


/** MP1 params */
typedef struct OMX_AUDIO_PARAM_MP1TYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_U32 nAudioBandWidth;       /**< Audio band width (in Hz) to which an encoder should
                                        limit the audio signal. Use 0 to let encoder decide */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} OMX_AUDIO_PARAM_MP1TYPE;


/** MP2 params */
typedef struct OMX_AUDIO_PARAM_MP2TYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_U32 nAudioBandWidth;       /**< Audio band width (in Hz) to which an encoder should
                                        limit the audio signal. Use 0 to let encoder decide */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} OMX_AUDIO_PARAM_MP2TYPE;


/** AC3 params */
typedef struct OMX_AUDIO_PARAM_AC3TYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    /*TODO: check if we need to differentiates EAC3 (DD+) and define AC3 format
          type if needed.*/
} OMX_AUDIO_PARAM_AC3TYPE;


/** DTS params */
/*TODO: check the DTS decoder proposal from DD and sync to it.*/
typedef struct OMX_AUDIO_PARAM_DTSTYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
} OMX_AUDIO_PARAM_DTSTYPE;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_AudioExt_h */

