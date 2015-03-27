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

#include "MagPipeline_Codec.h"

#define DEFAULT_PIPELINE_LOAD_PATH "/system/lib/pipeline" 

static i32 addPipelineList(MagCodecPipelineHandle hcpl,
                           void *hLib,
                           pipeline_reg_func_t   regFunc,
                           pipeline_dereg_func_t deregFunc){
    MagPipeline_Registration_t *regInfo;
    MagPipeline_Entry_t        *entry;
    OMX_U32 u = 0;
    
    regInfo = regFunc();
    if (NULL != regInfo){
        entry = (MagPipeline_Entry_t *)mag_mallocz(sizeof(MagPipeline_Entry_t));
        MAG_ASSERT(entry != NULL);
        
        INIT_LIST(&entry->node);
        entry->hLib        = hLib;
        entry->regInfo     = regInfo;
        entry->deregFunc   = deregFunc;
        entry->initialized = MAG_FALSE;

        AGILE_LOGD("add the component name = %s", regInfo->name);
        list_add_tail(&entry->node, &hcpl->mPipelineList);

        return 0;
    }else{
        AGILE_LOGE("failed to do regFunc[%p]()", regFunc);
        return -1;
    }   
}

static void loadPipelineLib(MagCodecPipelineHandle hcpl, const char *file, void *arg){
    void *so;
    void *reg;
    void *dereg;
    OMX_S32 ret;
    
    so = dlopen(file, RTLD_NOW);

    if(NULL == so){
        AGILE_LOGE("failed to load the pipeline lib: %s (error code: %s)", file, dlerror());
        return;
    }

    AGILE_LOGI("Loading the pipeline lib: %s", file);
    
    reg   = dlsym(so, "MagPipeline_Registration");
    dereg = dlsym(so, "MagPipeline_Deregistration");

    ret = addPipelineList(hcpl, so, (pipeline_reg_func_t)(_size_t)reg, (pipeline_dereg_func_t)(_size_t)dereg);
    if(ret == 0)
        *(int *)arg = *(int *)arg + 1;
}

static void loadPipelineRecursive( MagCodecPipelineHandle hcpl
                                   char *loadPath,
                                   void (*loader)(MagCodecPipelineHandle hcpl, const char *file, void *arg),
                                   void *arg){
    DIR *dir;
    struct dirent *fileInfo;
    char dirPathFull[PATH_MAX];
    
    dir = opendir(loadPath);

    if (NULL == dir){
        AGILE_LOGE("failed to open the dir: %s (err = %s)", loadPath, strerror(errno));
        return;
    }

    do{
        fileInfo = readdir(dir);

        if (NULL == fileInfo){
            AGILE_LOGE("failed to read the dir: %s (err = %s)", loadPath, strerror(errno));
            continue;
        }
        
        if( (fileInfo->d_type == DT_DIR) && 
            !(!strcmp(fileInfo->d_name, ".") || !strcmp(fileInfo->d_name, ".."))){
            sprintf(dirPathFull, "%s/%s", loadPath, fileInfo->d_name);
            AGILE_LOGI("find dir: %s", dirPathFull);
            loadPipelineRecursive(hcpl, dirPathFull, loader, arg);
        }else if ((fileInfo->d_type == DT_REG) || (fileInfo->d_type == DT_LNK)){
            sprintf(dirPathFull, "%s/%s", loadPath, fileInfo->d_name);
            AGILE_LOGI("find file: %s", dirPathFull);
            (*loader)(hcpl, dirPathFull, arg);
        }
    }while (fileInfo != NULL);

    closedir(dir);
}

static i32 loadPipelines(MagCodecPipelineHandle hcpl, const char *pSearchPath){
    i32 number = 0;

    loadPipelineRecursive(hcpl, pSearchPath, loadPipelineLib, &number);
    AGILE_LOGD("Loaded %d pipelines in total", number);
    return number;
}

static i32 MagPipelineCodec_Init(MagCodecPipelineHandle hcpl){
    char    loadPath[PATH_MAX];
    i32     num;

    sprintf(loadPath, "%s", getenv("PIPELINE_LOAD_PATH") ? getenv("PIPELINE_LOAD_PATH") : DEFAULT_PIPELINE_LOAD_PATH);
    AGILE_LOGD("try to load the pipelines from %s", loadPath);

    num = loadPipelines(hcpl, loadPath);

    hcpl->mPipelineNum = num;

    return MAG_ErrNone;
}

static i32 MagPipelineCodec_UnInit(MagCodecPipelineHandle hcpl){
    List_t *tmpNode;
    MagPipeline_Entry_t *pl;
    
    tmpNode = hcpl->mPipelineList.next;

    while (tmpNode != &hcpl->mPipelineList){
        pl = (MagPipeline_Entry_t *)list_entry(tmpNode, MagPipeline_Entry_t, node);
        list_del(tmpNode);
        if (pl->initialized){
            if (NULL != pl->deregFunc){
                pl->deregFunc(pl->hPipeline);
            }
        }
        
        dlclose(pl->hLib);
        mag_freep((void **)&pl);
        
        tmpNode = hcpl->mPipelineList.next;
    }

    return MAG_ErrNone;
}

MagCodecPipelineHandle createMagCodecPipeline(void){
    MagCodecPipelineHandle hcpl;

    hcpl = (MagCodecPipelineHandle)mag_mallocz(sizeof(MagPipeline_Codec_t));

    if (hcpl == NULL){
        AGILE_LOGE("failed to create the handler!");
        return NULL;
    }
    
    hcpl->init   = MagPipelineCodec_Init;
    hcpl->uninit = MagPipelineCodec_UnInit;
}   

void destroyMagCodecPipeline(MagCodecPipelineHandle *pdpl){

}