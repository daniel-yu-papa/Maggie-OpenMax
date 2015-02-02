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

#ifndef __OMXIL_BUFFER_MANAGER_H__
#define __OMXIL_BUFFER_MANAGER_H__

#include "framework/MagFramework.h"
#include "MagOMX_IL.h"

typedef struct{
    List_t Node;    
    OMX_BUFFERHEADERTYPE *pBufHeader;
}Omxil_BufferNode_t;

class OmxilBufferMgr{
public:
    OmxilBufferMgr(ui32 buf_size, ui32 buf_num, bool block);
    ~OmxilBufferMgr();

    OMX_ERRORTYPE create(OMX_HANDLETYPE hComp, ui32 portIdx, OMX_HANDLETYPE privData);
    OMX_BUFFERHEADERTYPE *get();
    void put(OMX_BUFFERHEADERTYPE *bufHeader);
    
    bool needPushBuffers(void);

private:
    ui32 mBufSize;
    ui32 mBufNum;
    ui32 mFreeNodeNum;

    MagMutexHandle mListMutex;
    List_t mBufFreeListHead;
    List_t mBufBusyListHead;

    bool mBlock;

    MagEventHandle         mPutBufEvent;
    MagEventGroupHandle    mWaitPutBufEventGroup;

    MagEventHandle         mGetBufEvent;
    MagEventGroupHandle    mWaitGetBufEventGroup;

    OMX_HANDLETYPE mhComponent;
};

#endif
