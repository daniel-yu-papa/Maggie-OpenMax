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

#ifndef __MAGOMX_COMPONENT_OTHER_DATA_SOURCE_H__
#define __MAGOMX_COMPONENT_OTHER_DATA_SOURCE_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

enum{
    MagOmxComponentDataSource_ReadDataMsg = 0
};

DeclareClass(MagOmxComponentDataSource, MagOmxComponentImpl);

Virtuals(MagOmxComponentDataSource, MagOmxComponentImpl) 
    /*Pure virtual functions. Must be overrided by sub-components*/
    OMX_ERRORTYPE (*MagOMX_DataSource_Init)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING     url);

    OMX_ERRORTYPE (*MagOMX_DataSource_Read)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_BUFFERHEADERTYPE *bufHeader);

    OMX_S64       (*MagOMX_DataSource_Seek)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_S64 offset,
                                    OMX_IN OMX_SEEK_WHENCE whence);
EndOfVirtuals;

ClassMembers(MagOmxComponentDataSource, MagOmxComponentImpl, \
    _status_t getReadDataLooper(OMX_HANDLETYPE handle); \
    MagMessageHandle createReadDataMessage(OMX_HANDLETYPE handle, ui32 what);  \
)
    MagMutexHandle         mhMutex;

    MagLooperHandle        mReadDataLooper;
    MagHandlerHandle       mReadDataMsgHandler;
    MagMessageHandle       mReadDataMsg;

    OMX_STRING             mUrl;

EndOfClassMembers;

#endif