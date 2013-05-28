#include "CPA/OMXCPA_base.h"

CPA_RESULTTYPE OMXCPA_base_Constructor(CPA_INOUT OMXCPABase_t **hPipe){
    if (NULL == *hPipe){
        *hPipe = (OMXCPABase_t *)calloc(1, sizeof(OMXCPABase_t));
        if (NULL == *hPipe){
            AGILE_LOGE("failed to allocate the memory for OMXCPABase_t");
            return CPA_ENOMEM;
        }
    }

    if (NULL == (*hPipe)->pCPA){
        (*hPipe)->pCPA = (CPA_PIPETYPE *)calloc(1, sizeof(CPA_PIPETYPE));
        if (NULL == (*hPipe)->pCPA){
            AGILE_LOGE("failed to allocate the memory for CPA_PIPETYPE*");
            goto err_nomem;
        }
    }

    (*hPipe)->pCPA->nApiVersion.nVersionMajor = CPA_VERSION_MAJOR;
    (*hPipe)->pCPA->nApiVersion.nVersionMinor = CPA_VERSION_MINOR;
    (*hPipe)->pCPA->nApiVersion.nRevision     = CPA_VERSION_REVISION;

    return CPA_OK;
    
err_nomem:
    if (*hPipe){
        free(*hPipe);
        *hPipe = NULL;
    }
}




