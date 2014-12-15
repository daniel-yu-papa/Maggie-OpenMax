#ifndef OMX_Component_Ext_h
#define OMX_Component_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Each OMX header must include all required header files to allow the
 *  header to compile without errors.  The includes below are required
 *  for this header file to compile successfully 
 */

#include <OMX_Component.h>

typedef enum OMX_PORTDOMAINEXTTYPE {
    OMX_PortDomain_ExtVendorStartUnused =OMX_PortDomainVendorStartUnused,
    OMX_PortDomainOther_Clock         /**< Clock domain port belongs to Other domain */
} OMX_PORTDOMAINEXTTYPE;

typedef struct OMX_CONFIG_UI32TYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 uValue;    
} OMX_CONFIG_UI32TYPE;

typedef struct OMX_CONFIG_FFMPEG_DATA_TYPE{
	OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
	OMX_PTR avformat;
	OMX_PTR avstream;
}OMX_CONFIG_FFMPEG_DATA_TYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif