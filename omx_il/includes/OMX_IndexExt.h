/*
 * Copyright (c) 2011 The Khronos Group Inc. 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions: 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 *
 */

/*
 *  OMX_IndexExt.h - OpenMax IL version 1.2.0
 *  The OMX_Index header file contains the definitions for both applications
 *  and components .
 */

#ifndef OMX_Index_Ext_h
#define OMX_Index_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Each OMX header must include all required header files to allow the
 *  header to compile without errors.  The includes below are required
 *  for this header file to compile successfully 
 */
#include <OMX_Types.h>

typedef enum OMX_INDEXTYPE_EXT {
	OMX_IndexExtensionsStart = OMX_IndexVendorStartUnused,
	/*Used for dynamically adding/removing the port to the component.
	* SetConfig: delete the port
	* GetConfig: add the port
	*/
	OMX_IndexConfigExtAddPort, 	       /**< reference: OMX_CONFIG_UI32TYPE */
	OMX_IndexConfigExtFFMpegData, 	   /**< reference: OMX_CONFIG_FFMPEG_DATA_TYPE */
    OMX_IndexConfigExtStartTime,       /**< reference: OMX_CONFIG_START_TIME_TYPE */
    OMX_IndexConfigExtReadData,        /**< reference: OMX_CONFIG_DATABUFFER */
    OMX_IndexConfigExtWriteData,       /**< reference: OMX_CONFIG_DATABUFFER */
    OMX_IndexConfigExtSeekData,        /**< reference: OMX_CONFIG_SEEKDATABUFFER */
    OMX_IndexParamExtBufferSetting,    /**< reference: OMX_BUFFER_PARAM write-only*/ 
    OMX_IndexParamExtBufferStatus,     /**< reference: OMX_BUFFER_STATUS read-only*/
} OMX_INDEXTYPE_EXT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */
