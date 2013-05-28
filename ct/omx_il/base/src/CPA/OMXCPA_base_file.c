#include "CPA/OMXCPA_base.h"
#include "CPA/OMXCPA_base_file.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static CPA_RESULTTYPE OMXCPA_file_SetConfig( CPA_IN CPA_HANDLE hPipe, 
                                              CPA_IN CPA_HANDLE hContent,
                                              CPA_IN CPA_STRING szKey, 
                                              CPA_IN CPA_PTR value){

}

static CPA_RESULTTYPE OMXCPA_file_GetConfig( CPA_IN CPA_HANDLE hPipe, 
                                              CPA_IN CPA_HANDLE hContent,
                                              CPA_IN CPA_STRING szKey, 
                                              CPA_OUT CPA_PTR value){

}

static CPA_RESULTTYPE OMXCPA_file_Open( CPA_IN CPA_HANDLE hPipe, 
                                         CPA_OUT CPA_HANDLE *hContent, 
                                         CPA_IN CPA_STRING szURI, 
                                         CPA_IN CPA_ACCESSTYPE eAccess ){
    CPA_STRING fname;
    CPA_S32 flags;
    OMXCPABaseFile_t *pCPAFile;
    int fd;
    
    if ((NULL == szURI) || (NULL == hPipe))
        return CPA_EINVAL;

    pCPAFile = (OMXCPABaseFile_t *)hPipe;
    
    fname = (strstr(szURI, "file://") == szURI)? szURI + strlen("file://") : szURI;

    switch (eAccess){
        case CPA_AccessRead:
            flags = O_RDONLY;
            break;

        case CPA_AccessWrite:
            flags = O_WRONLY | O_CREAT;
            break;

        case CPA_AccessReadWrite:
            flags = O_RDWR | O_CREAT;
            break;
            
        default:
            AGILE_LOGE("wrong access type: 0x%x", eAccess);
            return CPA_EACCESS;
    }

    fd = open(fname, flags);
    
    if (-1 == fd){
        AGILE_LOGE("failed to open the file: %s. [error: %s]", fname, strerror(errno));
        return CPA_EURINOTSUPP;
    }else{
        pCPAFile->fileHandle = fd;
        pCPAFile->szURI = strdup(fname);

        struct stat st;
        if (fstat(fd, &st))
        {
            AGILE_LOGW("unable to get file status, file size is hard-coded.\n");
            //something has gone wrong, or maybe the
            //_LARGEFILE_SOURCE support is not enabled.
            pCPAFile->fileLen = 0x7fffffff;
        } else
        {
            pCPAFile->fileLen = st.st_size;
        }
    }

    *hContent = hPipe;

    return CPA_OK;
}

static CPA_RESULTTYPE OMXCPA_file_Create( CPA_IN CPA_HANDLE hPipe, 
                                          CPA_OUT CPA_HANDLE *hContent, 
                                          CPA_IN CPA_STRING szURI ){
    return OMXCPA_file_Open(hPipe, hContent, szURI, CPA_AccessReadWrite);
}

static CPA_RESULTTYPE OMXCPA_file_Close( CPA_IN CPA_HANDLE hPipe, 
                                         CPA_INOUT CPA_HANDLE *hContent ){
    OMXCPABaseFile_t *pCPAFile;
    int ret;
    
    pCPAFile = (OMXCPABaseFile_t *)hContent;

    ret = close(pCPAFile->fileHandle);

    if (-1 == ret){
        AGILE_LOGE("failed to close file (%s). [error: %s]", pCPAFile->szURI, strerror(errno));
        return CPA_EIO;
    }

    return CPA_OK;
}

static CPA_RESULTTYPE OMXCPA_file_ReleaseContentPipeType( CPA_INOUT CPA_HANDLE *hPipe){
    OMXCPABaseFile_t *pCPAFile;

    if (NULL == *hPipe)
        return CPA_EINVAL;

    pCPAFile = (OMXCPABaseFile_t *)(*hPipe);

    if (pCPAFile->szURI){
        free(pCPAFile->szURI);
        pCPAFile->szURI = NULL;
    }
    
    OMXCPA_file_Close(*hPipe, hPipe);
}

static CPA_RESULTTYPE OMXCPA_file_CheckAvailableBytesToRead( CPA_IN CPA_HANDLE hPipe, 
                                                                    CPA_IN CPA_HANDLE hContent, 
                                                                    CPA_IN CPA_U32 nBytesRequested, 
                                                                    CPA_OUT CPA_CHECKBYTESRESULTTYPE* peResult ){

}

static CPA_RESULTTYPE OMXCPA_file_CheckAvailableBytesToWrite( CPA_IN CPA_HANDLE hPipe, 
                                                                    CPA_IN CPA_HANDLE hContent, 
                                                                    CPA_IN CPA_U32 nBytesRequested, 
                                                                    CPA_OUT CPA_CHECKBYTESRESULTTYPE* peResult ){

}

static CPA_RESULTTYPE OMXCPA_file_SetPosition( CPA_IN CPA_HANDLE hPipe, 
                                                CPA_IN CPA_HANDLE hContent, 
                                                CPA_IN CPA_POSITIONTYPE nOffset, 
                                                CPA_IN CPA_ORIGINTYPE eOrigin ){
    int whence;
    OMXCPABaseFile_t *pCPAFile;
    off_t ret;
    
    if ((NULL == hContent) || (NULL == hPipe))
        return CPA_EINVAL;

    switch (eOrigin){
        case CPA_OriginBegin:
            whence = SEEK_SET;
            break;

        case CPA_OriginCur:
            whence = SEEK_CUR;
            break;

        case CPA_OriginEnd:
            whence = SEEK_END;
            break;

        default:
            AGILE_LOGE("wrong eOrigin parameter: %d", eOrigin);
            return CPA_EPOSNOTAVAIL;
    }

    pCPAFile = (OMXCPABaseFile_t *)hContent;

    ret = lseek(pCPAFile->fileHandle, nOffset, whence);

    if (-1 == ret){
        AGILE_LOGE("failed to seek the file(%s) to position[%d: %d]. [error: %s]", pCPAFile->szURI, eOrigin, nOffset, strerror(errno));
        return CPA_EPOSNOTAVAIL;
    }else{
        AGILE_LOGV("Succeed to seek the file(%s) to offset: %d against the beginning", pCPAFile->szURI, ret);
        return CPA_OK;
    }
}

static CPA_RESULTTYPE OMXCPA_file_GetPositions( CPA_IN CPA_HANDLE hPipe, 
                                                  CPA_IN CPA_HANDLE hContent, 
                                                  CPA_OUT CPA_POSITIONINFOTYPE* pPosition ){
    return CPA_EUNKNOWN;
}

static CPA_RESULTTYPE OMXCPA_file_GetCurrentPosition( CPA_IN CPA_HANDLE hPipe, 
                                                         CPA_IN CPA_HANDLE hContent, 
                                                         CPA_OUT CPA_POSITIONINFOTYPE* pPosition ){
    return CPA_EUNKNOWN;
}

static CPA_RESULTTYPE OMXCPA_file_Read( CPA_IN CPA_HANDLE hPipe, 
                                        CPA_IN CPA_HANDLE hContent, 
                                        CPA_OUT CPA_BYTE* pData, 
                                        CPA_INOUT CPA_U32* pSize ){
    OMXCPABaseFile_t *pCPAFile;
    ssize_t ret;
    
    if ((NULL == hPipe) || (NULL == hContent)){
        return CPA_EINVAL;
    }

    pCPAFile = (OMXCPABaseFile_t *)hContent;

    ret = read(pCPAFile->fileHandle, pData, *pSize);

    if (-1 == ret){
        AGILE_LOGE("failed to read the file(%s). [error: %s]", pCPAFile->szURI, strerror(errno));
        *pSize = 0;
        return CPA_EIO;
    }else{
        if (ret < (*pSize)){
            AGILE_LOGW("read the %d bytes at the end of file(%s)", ret, pCPAFile->szURI);
            *pSize = ret;
            return CPA_OKEOS;
        }else{
            return CPA_OK;
        }
    }
}

static CPA_RESULTTYPE OMXCPA_file_ReadBuffer( CPA_IN CPA_HANDLE hPipe, 
                                                CPA_IN CPA_HANDLE hContent, 
                                                CPA_OUT CPA_BYTE** ppBuffer, 
                                                CPA_INOUT CPA_U32* pSize, 
                                                CPA_IN CPA_BOOL bForbidCopy ){

}

static CPA_RESULTTYPE OMXCPA_file_ReleaseReadBuffer( CPA_IN CPA_HANDLE hPipe, 
                                                         CPA_IN CPA_HANDLE hContent, 
                                                         CPA_IN CPA_BYTE* pBuffer ){

}

static CPA_RESULTTYPE OMXCPA_file_Write( CPA_IN CPA_HANDLE hPipe, 
                                         CPA_IN CPA_HANDLE hContent, 
                                         CPA_IN CPA_BYTE *pData, 
                                         CPA_INOUT CPA_U32* pSize ){

}

static CPA_RESULTTYPE OMXCPA_file_RegisterCallback( CPA_IN CPA_HANDLE hPipe, 
                                                      CPA_IN CPA_CALLBACKTYPE ClientCallback, 
                                                      CPA_IN CPA_PTR ClientContext ){

}

CPA_RESULTTYPE OMXCPA_base_file_Constructor(CPA_OUT OMXCPABaseFile_t **hPipe){
    CPA_RESULTTYPE ret;
    
    *hPipe = (OMXCPABaseFile_t *)calloc(1, sizeof(OMXCPABaseFile_t));

    ret = OMXCPA_base_Constructor((OMXCPABase_t *)(*hPipe));

    if (CPA_OK != ret){
        *hPipe = NULL;
        return CPA_ENOMEM;
    }
    
    (*hPipe)->pCPA->CPA_ReleaseContentPipeType = OMXCPA_file_ReleaseContentPipeType;
    (*hPipe)->pCPA->CPA_SetConfig = OMXCPA_file_SetConfig;
    (*hPipe)->pCPA->CPA_GetConfig = OMXCPA_file_GetConfig;
    (*hPipe)->pCPA->CPA_Open = OMXCPA_file_Open;
    (*hPipe)->pCPA->CPA_Create = OMXCPA_file_Create;
    (*hPipe)->pCPA->CPA_Close = OMXCPA_file_Close;
    (*hPipe)->pCPA->CPA_CheckAvailableBytesToRead = OMXCPA_file_CheckAvailableBytesToRead;
    (*hPipe)->pCPA->CPA_CheckAvailableBytesToWrite = OMXCPA_file_CheckAvailableBytesToWrite;
    (*hPipe)->pCPA->CPA_SetPosition = OMXCPA_file_SetPosition;
    (*hPipe)->pCPA->CPA_GetPositions = OMXCPA_file_GetPositions;
    (*hPipe)->pCPA->CPA_GetCurrentPosition = OMXCPA_file_GetCurrentPosition;
    (*hPipe)->pCPA->CPA_Read = OMXCPA_file_Read;
    (*hPipe)->pCPA->CPA_ReadBuffer = OMXCPA_file_ReadBuffer;
    (*hPipe)->pCPA->CPA_ReleaseReadBuffer = OMXCPA_file_ReleaseReadBuffer;
    (*hPipe)->pCPA->CPA_Write = OMXCPA_file_Write;
    (*hPipe)->pCPA->CPA_RegisterCallback = OMXCPA_file_RegisterCallback;

    return CPA_OK;
}

CPA_RESULTTYPE CPA_GetContentPipe( CPA_OUT CPA_HANDLE *hPipe){
    return OMXCPA_base_file_Constructor(hPipe);
}

