#include "MagPlayer_ContentPipe.h"

MagPlayer_Component_CP::MagPlayer_Component_CP(){
    mStreamBuffer = NULL;
    mSourceType   = MPCP_INVALID;
}

MagPlayer_Component_CP::~MagPlayer_Component_CP(){

}

MPCP_RESULTTYPE MagPlayer_Component_CP::Open( ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::Create( MPCP_IN ui8 *szURI ){
    return MPCP_OK;
}


MPCP_RESULTTYPE MagPlayer_Component_CP::Create( MPCP_IN StreamBufferUser *buffer ){
    mStreamBuffer = buffer;
    mSourceType   = MPCP_MEMORY;
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::Close( ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::SetPosition( MPCP_IN MPCP_POSITIONTYPE nOffset, MPCP_IN MPCP_ORIGINTYPE eOrigin ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::GetCurrentPosition( MPCP_OUT MPCP_POSITIONTYPE* pPosition ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MPCP_RESULTTYPE MagPlayer_Component_CP::GetSize( MPCP_OUT MPCP_POSITIONINFOTYPE* pSize ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::Read( MPCP_OUT ui8* pData, MPCP_INOUT ui32* pSize ){
    if (mSourceType == MPCP_MEMORY){
        ui32 readSize = *pSize;
        if (mStreamBuffer){
            *pSize = mStreamBuffer->read(static_cast<void *>(pData), readSize);
        }else{
            return MPCP_EINVAL;
        }
    }else{

    }
    return MPCP_OK;
}

