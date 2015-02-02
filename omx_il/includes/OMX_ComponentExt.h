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

#ifndef OMX_Component_Ext_h
#define OMX_Component_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Each OMX header must include all required header files to allow the
 *  header to compile without errors.  The includes below are required
 *  for this header file to compile successfully 
 */

#include <OMX_Component.h>

typedef enum OMX_PORTDOMAINEXTTYPE {
    OMX_PortDomain_ExtVendorStartUnused =OMX_PortDomainVendorStartUnused,
    OMX_PortDomainOther_Clock         /**< Clock domain port belongs to Other domain */
} OMX_PORTDOMAINEXTTYPE;

typedef struct OMX_CONFIG_UI32TYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 uValue;    
} OMX_CONFIG_UI32TYPE;

typedef struct OMX_CONFIG_FFMPEG_DATA_TYPE{
	OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
	OMX_PTR avformat;
	OMX_PTR avstream;
}OMX_CONFIG_FFMPEG_DATA_TYPE;

typedef struct OMX_CONFIG_START_TIME_TYPE{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_TICKS start_time;  /*in us*/
}OMX_CONFIG_START_TIME_TYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif