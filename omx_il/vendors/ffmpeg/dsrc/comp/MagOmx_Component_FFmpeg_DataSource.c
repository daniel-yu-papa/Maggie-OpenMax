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

#include "MagOmx_Component_FFmpeg_DataSource.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME "OMX.Mag.datasource.ffmpeg"

#define START_PORT_INDEX kCompPortStartNumber

AllocateClass(MagOmxComponent_FFmpeg_DataSource, MagOmxComponentDataSource);

static OMX_ERRORTYPE virtual_FFmpeg_DataSource_Init(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING     url){
    int ret;
    char errbuf[50];
    MagOmxComponent_FFmpeg_DataSource thiz;

    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_DataSource);
    ret = avio_open2(&thiz->mAVIO, url, AVIO_FLAG_READ, NULL, NULL);
    if (ret) {
        av_strerror(ret, errbuf, sizeof(errbuf));
        AGILE_LOGE("Unable to open %s: %s\n", url, errbuf);
        return OMX_ErrorContentURIError;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_DataSource_Read(
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_BUFFERHEADERTYPE *bufHeader){
    int n;

    n = avio_read(thiz->mAVIO, bufHeader->pBuffer, bufHeader->nAllocLen);
    if (n != bufHeader->nAllocLen){
        AGILE_LOGD("Expected reading %d, actual %d", bufHeader->nAllocLen, n);
    }
    if (n > 0){
        bufHeader->nFilledLen = n;  
    }else{
        AGILE_LOGE("Error happened, to read nothing(%d)", n);
        bufHeader->nFilledLen = 0;
        return OMX_ErrorNotReady;
    }

    return OMX_ErrorNone;
}

static OMX_S64       virtual_FFmpeg_DataSource_Seek(
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_S64 offset,
                                OMX_IN OMX_SEEK_WHENCE whence){
    OMX_S64 position;
    MagOmxComponent_FFmpeg_DataSource thiz;

    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_DataSource);
    position = avio_seek(thiz->mAVIO, offset, whence);
    if (position < 0){
        AGILE_LOGE("failed to seek[offset: %lld, whence: %d](ret: %lld)", 
                    offset, whence, position);
    }
    return position;
}

static OMX_ERRORTYPE virtual_FFmpeg_DataSource_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    OMX_HANDLETYPE dsOutPort;
    MagOmxComponentImpl dsCompImpl;

    AGILE_LOGV("enter!");
    dsCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    dsOutPort  = dsCompImpl->getPort(dsCompImpl, START_PORT_INDEX + 0);

    ooc_delete((Object)dsOutPort);

    return OMX_ErrorNone;
}


static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPortDataSource           dsOutPort;
    MagOmxPort_Constructor_Param_t param;
    MagOmxComponentImpl            dsCompImpl;
    MagOmxComponent                dsComp;

    AGILE_LOGV("enter!");

    param.portIndex    = START_PORT_INDEX + 0;
    param.isInput      = OMX_FALSE;
    param.bufSupplier  = OMX_BufferSupplyOutput;
    param.formatStruct = 0;
    sprintf((char *)param.name, "%s-Out", DATA_SOURCE_PORT_NAME);

    ooc_init_class(MagOmxPortDataSource);
    dsOutPort = ooc_new(MagOmxPortDataSource, &param);
    MAG_ASSERT(dsOutPort);

    dsCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    dsComp     = ooc_cast(hComponent, MagOmxComponent);
    
    dsComp->setName(dsComp, (OMX_U8 *)COMPONENT_NAME);
    dsCompImpl->addPort(dsCompImpl, START_PORT_INDEX + 0, dsOutPort);

    return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_FFmpeg_DataSource_initialize(Class this){
    AGILE_LOGV("Enter!");
    
    /*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_DataSourceVtableInstance.MagOmxComponentDataSource.MagOMX_DataSource_Init = virtual_FFmpeg_DataSource_Init;
    MagOmxComponent_FFmpeg_DataSourceVtableInstance.MagOmxComponentDataSource.MagOMX_DataSource_Read = virtual_FFmpeg_DataSource_Read;
    MagOmxComponent_FFmpeg_DataSourceVtableInstance.MagOmxComponentDataSource.MagOMX_DataSource_Seek = virtual_FFmpeg_DataSource_Seek;

    MagOmxComponent_FFmpeg_DataSourceVtableInstance.MagOmxComponentDataSource.MagOmxComponentImpl.MagOMX_Deinit = virtual_FFmpeg_DataSource_Deinit;
}

static void MagOmxComponent_FFmpeg_DataSource_constructor(MagOmxComponent_FFmpeg_DataSource thiz, const void *params){
    AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_DataSource));
    chain_constructor(MagOmxComponent_FFmpeg_DataSource, thiz, params);
}

static void MagOmxComponent_FFmpeg_DataSource_destructor(MagOmxComponent_FFmpeg_DataSource thiz, MagOmxComponent_FFmpeg_DataSourceVtable vtab){
    AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_DataSource_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                                            OMX_IN  OMX_PTR pAppData,
                                                            OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
    MagOmxComponent_FFmpeg_DataSource hDataSourceComp;
    MagOmxComponentImpl     parent;
    OMX_U32 param[1];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_DataSource);

    param[0] = MAG_START_PORT_INDEX;

    hDataSourceComp = (MagOmxComponent_FFmpeg_DataSource) ooc_new( MagOmxComponent_FFmpeg_DataSource, (void *)param);
    MAG_ASSERT(hDataSourceComp);

    parent = ooc_cast(hDataSourceComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hDataSourceComp, pAppData, pCallBacks);
    if (*hComponent){
        return localSetupComponent(hDataSourceComp);
    }else{
        return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_DataSource_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
    OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
    MagOmxComponent_FFmpeg_DataSource hDataSourceComp;

    AGILE_LOGD("enter!");
    hDataSourceComp = (MagOmxComponent_FFmpeg_DataSource)compType->pComponentPrivate;
    ooc_delete((Object)hDataSourceComp);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
    static char * roles[] = {OMX_ROLE_DATA_SOURCE_AUTO};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponent_FFmpeg_DataSource_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
    MagOmxComponent_FFmpeg_DataSource_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef START_PORT_INDEX