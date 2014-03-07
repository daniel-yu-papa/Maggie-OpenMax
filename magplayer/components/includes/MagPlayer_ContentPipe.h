#ifndef __MAGPLAYER_CONTENT_PIPE_H__
#define __MAGPLAYER_CONTENT_PIPE_H__

#include "StreamBufferDef.h"

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

#define MPCP_POSITION_NA -1

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


class MagPlayer_Component_CP{
public:
    MagPlayer_Component_CP();
    virtual ~MagPlayer_Component_CP();
    
    MPCP_RESULTTYPE SetConfig( MPCP_IN ui8 *szKey, MPCP_IN void *value);    
    MPCP_RESULTTYPE GetConfig( MPCP_IN ui8 *szKey, MPCP_OUT void *value);

    MPCP_RESULTTYPE Open( );    
    MPCP_RESULTTYPE Create( MPCP_IN ui8 *szURI );
    MPCP_RESULTTYPE Create( MPCP_IN StreamBufferUser *buffer );
    MPCP_RESULTTYPE Close( );

    MPCP_RESULTTYPE SetPosition( MPCP_IN MPCP_POSITIONTYPE nOffset, MPCP_IN MPCP_ORIGINTYPE eOrigin );   
    MPCP_RESULTTYPE GetCurrentPosition( MPCP_OUT MPCP_POSITIONTYPE* pPosition );
    MPCP_RESULTTYPE GetSize( MPCP_OUT MPCP_POSITIONINFOTYPE* pSize );

    MPCP_RESULTTYPE Read( MPCP_OUT ui8* pData, MPCP_INOUT ui32* pSize );

private:
    enum Source_t{
        MPCP_INVALID = 0,
        MPCP_URL,
        MPCP_MEMORY,
    };
    StreamBufferUser *mStreamBuffer;

    enum Source_t mSourceType;
};


#endif