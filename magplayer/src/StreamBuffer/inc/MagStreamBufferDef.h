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

#ifndef __MAG_STREAM_BUFFER_DEFINITION_H__
#define __MAG_STREAM_BUFFER_DEFINITION_H__

#include "framework/MagFramework.h"

/*
* w == r: buffer is empty
* w - r == 1 OR r -w == 1: buffer is full
*/
typedef struct{
    i32 readIndex;
    i32 writeIndex;
    i32 totalSize;
    ui8  *pointer;
    MagMutexHandle lock;
}CircularBufferMgr_t;

struct StreamBuffer : public StreamBuffer_Server{
    StreamBuffer();
    virtual void setUser(const sp<IStreamBufferUser> &user);
    virtual void setBuffers(List_t *bufListHead);
    virtual void onBufferEmpty(_size_t index, _size_t size);
    virtual Type  getType(void);

    void    setType(Type t);
    _size_t WriteData(void *data, _size_t size, bool block);

    sp<IStreamBufferUser> &getUser();

    void reset();
protected:
    virtual ~StreamBuffer();


private:
    _size_t writeData_CircularBuffer(void *data, _size_t size, bool block);
    void drain_CircularBuffer(_size_t size);
    
    CircularBufferMgr_t   *mCBMgr;
    
    List_t                *mBufferHead;
    ui32                  mBufferNum;
    sp<IStreamBufferUser> mUser;
    
    MagEventGroupHandle   mBufStatusEvtGrp;
    MagEventHandle        mBufFreeEvt;
    bool                  mbQuit;
    IStreamBuffer::Type   mType;
};


struct StreamBufferUser : public StreamBufferUser_Server {
    enum Read_Policy{
        READ_ANY,   /*read out the data even if the available data size is less than the expected size. it is unblock operation*/
        READ_FULL,  /*read out the data only if the available data size meets the expected size. It is block operation.*/
    };
    
    StreamBufferUser(const sp<IStreamBuffer> &buffer, _size_t bufSize, _size_t bufNum);
    virtual void onBufferFilled(_size_t index, _size_t size);
    
    virtual void issueCommand(
            Command cmd, bool synchronous);


    void reset(void);
    _size_t read(void *data, _size_t size);
    
    bool isEOS();
    void start(enum Read_Policy p);
    _size_t getDataSize();
    
protected:
    virtual ~StreamBufferUser();
    
private:
    void init();
    _size_t readData_CircularBuffer(void *data, _size_t size);
    void fill_CircularBuffer(_size_t size);
    static void doRestSetupCB(void *arg);

    sp<IStreamBuffer> mStreamBuffer;
    sp<MemoryDealer>  mMemDealer;
    
    _size_t mBufferSize;
    _size_t mBufferNumber;

    List_t  mBufferListHead;

    CircularBufferMgr_t *mCBMgr;

    bool mEOS;
    bool mQuitRead; /*temporaily use the stupid looping read method untill the buffer overflow/underflow logic is added*/
    
    enum Read_Policy mReadPolicy;
};

#endif