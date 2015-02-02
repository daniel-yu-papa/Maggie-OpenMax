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

#ifndef __MAGPLAYER_CONTENT_PIPE_H__
#define __MAGPLAYER_CONTENT_PIPE_H__

#include "framework/MagFramework.h"
#include "MagParameters.h"

#ifdef INTER_PROCESSES
#include "MagStreamBufferDef.h"
#endif

#ifndef MPCP_IN
#define MPCP_IN
#endif

#ifndef MPCP_OUT
#define MPCP_OUT
#endif

#ifndef MPCP_INOUT
#define MPCP_INOUT
#endif

typedef enum MPCP_RESULTTYPE{
    MPCP_OK,
    MPCP_OKEOS,
    MPCP_OKPOSITIONEXCEED2GB,
    MPCP_EPOSNOTAVAIL,
    MPCP_EUNKNOWN,
    MPCP_EACCESS,
    MPCP_EAGAIN,
    MPCP_EALREADY,
    MPCP_EBUSY,
    MPCP_ECONNREFUSED,
    MPCP_ECONNRESET,
    MPCP_EEXIST,
    MPCP_EFBIG,
    MPCP_EINVAL,
    MPCP_EIO,
    MPCP_ENOENT,
    MPCP_EURINOTSUPP,
    MPCP_ENOMEM,
    MPCP_ENOSPC,
    MPCP_ENO_RECOVERY,
    MPCP_EOPNOTSUPP,
    MPCP_ETIMEDOUT,
    MPCP_EVERSION,
    MPCP_RESULTKhronosExtensions = 0x6F000000, /* Reserved region for introducing Khronos Standard Extensions */
    MPCP_RESULTVendorStartUnused = 0x7F000000, /* Reserved region for introducing Vendor Extensions */
    MPCP_RESULTMax = 0x7FFFFFFF
} MPCP_RESULTTYPE;

typedef enum MPCP_WorkingMode{
    MPCP_MODE_DUMMY, /*No Real Content Pipe entity*/
    MPCP_MODE_NORMAL, /*The real entity with full implementation*/
}MPCP_WORKINGMODE;

#define MPCP_POSITION_NA -1
#define MPCP_64BITSSUPPORTED

#ifdef MPCP_64BITSSUPPORTED
#define MPCP_POSITION_32_MAX 0x000000007FFFFFFF
typedef i64 MPCP_POSITIONTYPE;
#else
#define MPCP_POSITION_32_MAX 0x7FFFFFFF
typedef i32 MPCP_POSITIONTYPE;
#endif

typedef enum MPCP_ORIGINTYPE {
    MPCP_OriginBegin,
    //MPCP_OriginFirst,
    MPCP_OriginCur,
    //MPCP_OriginLast,
    MPCP_OriginEnd,
    MPCP_OriginKhronosExtensions = 0x6F000000, /* Reserved region for introducing Khronos Standard Extensions */
    MPCP_OriginVendorStartUnused = 0x7F000000, /* Reserved region for introducing Vendor Extensions */
    MPCP_OriginMax = 0X7FFFFFFF
} MPCP_ORIGINTYPE;

typedef struct MPCP_POSITIONINFOTYPE {
    MPCP_POSITIONTYPE nDataBegin;
    MPCP_POSITIONTYPE nDataFirst;
    MPCP_POSITIONTYPE nDataCur;
    MPCP_POSITIONTYPE nDataLast;
    MPCP_POSITIONTYPE nDataEnd;
} MPCP_POSITIONINFOTYPE;

/*config Name Definitions*/
#define kMPCPConfigName_WoringMode  "WoringMode"
#define kMPCPConfigName_URI         "URI"

class MagContentPipe{
public:
    MagContentPipe();
    virtual ~MagContentPipe();
    
    MPCP_RESULTTYPE SetConfig(MPCP_IN const char *name, MagParamType_t type, MPCP_IN void *value);
    MPCP_RESULTTYPE GetConfig(MPCP_IN const char *name, MagParamType_t type, MPCP_OUT void **value);

    MPCP_RESULTTYPE Create( MPCP_IN const char *szURI );
#ifdef INTER_PROCESSES
    MPCP_RESULTTYPE Create( MPCP_IN StreamBufferUser *buffer );
#endif
    MPCP_RESULTTYPE Open ( );
    MPCP_RESULTTYPE Close ( );

    MPCP_RESULTTYPE SetPosition( MPCP_IN MPCP_POSITIONTYPE nOffset, MPCP_IN MPCP_ORIGINTYPE eOrigin );   
    MPCP_RESULTTYPE GetCurrentPosition( MPCP_OUT MPCP_POSITIONTYPE* pPosition );
    MPCP_RESULTTYPE GetSize( MPCP_OUT MPCP_POSITIONTYPE* pSize );

    MPCP_RESULTTYPE Read( MPCP_OUT ui8* pData, MPCP_INOUT ui32* pSize );
    void            SetDemuxerNotifier(MagMessageHandle msg);
    MPCP_RESULTTYPE Flush();
    
private:
    enum{
        MagCPMsg_DataObserver,
    };
    
    enum Source_t{
        MPCP_INVALID = 0,
        MPCP_URL,
        MPCP_MEMORY,
    };

#ifdef INTER_PROCESSES
    StreamBufferUser *mStreamBuffer;
#endif

    enum Source_t mSourceType;

    void onDataObserver(MagMessageHandle msg);
    MagMessageHandle createMessage(ui32 what);
    _status_t getLooper();

    static void onMessageReceived(const MagMessageHandle msg, void *priv);
    
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    MagMessageHandle mDataObserverMsg;
    MagMessageHandle mDemuxerNotifier;

    MagMiniDBHandle mConfigDB;

    bool mIsOpened;
};


#endif