#ifndef __MAG_PARAMETERS_H__
#define __MAG_PARAMETERS_H__

typedef enum{
    MagParamTypeInt32,
    MagParamTypeInt64,
    MagParamTypeUInt32,
    MagParamTypeFloat,
    MagParamTypeDouble,
    MagParamTypePointer,
    MagParamTypeString,
}MagParamType_t;

typedef enum parameter_key_t{
    PARAM_KEY_None      = 0,
    PARAM_KEY_CP_AVAIL,
    
    PARAM_KEY_VendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    PARAM_KEY_Max = 0x7FFFFFFF,
}PARAMETER_KEY_t;

#define kMediaPlayerConfig_CPAvail        "ContentPipe.Available"            /*Int32: 0/1*/
#define kMediaPlayerConfig_AvSyncDisable  "AVSync.Disable"                   /*Int32: 0 - enable, 1 - disable*/
#define kMediaPlayerConfig_AudioDisable   "Audio.Disable"                    /*Int32: 0 - enable, 1 - disable*/

#endif