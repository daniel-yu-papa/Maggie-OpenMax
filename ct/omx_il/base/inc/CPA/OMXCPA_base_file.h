#ifndef __OMX_CPA_BASE_FILE_H__
#define __OMX_CPA_BASE_FILE_H__

DERIVEDCLASS(OMXCPABaseFile_t, OMXCPABase_t)
#define OMXCPABaseFile_t_FIELDS \
    OMXCPABase_t_FIELDS \
    int       fileHandle; \
    CPA_U64   fileLen;
ENDCLASS(OMXCPABaseFile_t)
#endif