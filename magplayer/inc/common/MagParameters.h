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

#ifndef __MAG_PARAMETERS_H__
#define __MAG_PARAMETERS_H__

typedef enum{
    MagParamTypeInt32,
    MagParamTypeInt64,
    MagParamTypeUInt32,
    MagParamTypeFloat,
    MagParamTypeDouble,
    MagParamTypePointer,
    MagParamTypeString,
}MagParamType_t;

typedef enum parameter_key_t{
    PARAM_KEY_None      = 0,
    PARAM_KEY_CP_AVAIL,
    
    PARAM_KEY_VendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    PARAM_KEY_Max = 0x7FFFFFFF,
}PARAMETER_KEY_t;

#define kMediaPlayerConfig_CPAvail        "ContentPipe.Available"            /*Int32: 0/1*/
#define kMediaPlayerConfig_AvSyncDisable  "AVSync.Disable"                   /*Int32: 0 - enable, 1 - disable*/
#define kMediaPlayerConfig_AudioDisable   "Audio.Disable"                    /*Int32: 0 - enable, 1 - disable*/

#endif