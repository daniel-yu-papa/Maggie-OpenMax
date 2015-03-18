#ifndef OMX_Other_Ext_h
#define OMX_Other_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Each OMX header must include all required header files to allow the
 *  header to compile without errors.  The includes below are required
 *  for this header file to compile successfully 
 */

#include <OMX_Core.h>

typedef enum OMX_BUFFER_MODE{
    OMX_BUFFER_MODE_NONE = 0,
    OMX_BUFFER_MODE_PUSH,
    OMX_BUFFER_MODE_PULL
}OMX_BUFFER_MODE;

typedef enum OMX_BUFFER_TYPE{
    OMX_BUFFER_TYPE_BYTE = 0,
    OMX_BUFFER_TYPE_FRAME
}OMX_BUFFER_TYPE;

typedef enum OMX_SEEK_WHENCE{
    OMX_SEEK_SET = 0,
    OMX_SEEK_CUR,
    OMX_SEEK_END
}OMX_SEEK_WHENCE;

typedef struct OMX_BUFFER_PARAM {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_BUFFER_MODE mode;         /*push or pull*/
    OMX_U32 uHighPercent;         /*High threshold for buffering to finish. Allowed [0, 100]. default: 99*/
    OMX_U32 uLowPercent;          /*Low threshold for buffering to start. Allowed [0, 100]. default: 10*/
    OMX_U32 uMaxSizeBuffers;      /*Max. number of buffers in the queue*/
    OMX_U32 uMaxSizeBytes;        /*Max. amount of data in the queue in Bytes*/
    OMX_U32 uMaxSizeTime;         /*Max. amount of data in the queue in ns*/
    OMX_U32 uRingBufferMaxSize;   /*The maximum size of the ring buffer in bytes. If it is 0, the ring buffer is disabled*/
    OMX_U32 uUseRateEstimate;     /*Estimate the bitrate of the stream to calculate time level.*/
} OMX_BUFFER_PARAM;

typedef struct OMX_BUFFER_STATUS {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 uCurrentLevelFrames;         /*Current number of buffers in the queue*/
    OMX_U32 uCurrentLevelBytes;          /*Current amount of data in the queue (bytes).*/
    OMX_U32 uCurrentLevelTime;           /*Current amount of data in the queue (in ns).*/
} OMX_BUFFER_STATUS;

typedef struct OMX_CONFIG_DATABUFFER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U8* pBuffer;
    OMX_U32 nLen;
    OMX_U32 nDoneLen;          /*Returned: actual read/write data length*/
} OMX_CONFIG_DATABUFFER;

typedef struct OMX_CONFIG_SEEKDATABUFFER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_S64 sOffset;
    OMX_SEEK_WHENCE sWhence;
    OMX_S64 sCurPos;            /*Returned: current position in the source after the seeking*/
} OMX_CONFIG_SEEKDATABUFFER;

typedef struct OMX_DEMUXER_SETTING {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_BUFFER_TYPE *peBufferType;
    OMX_STRING      *ppcStreamUrl;
    OMX_U32         nUrlNumber;
} OMX_DEMUXER_SETTING;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif