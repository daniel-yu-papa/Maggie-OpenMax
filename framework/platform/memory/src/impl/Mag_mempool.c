#include "Mag_mempool.h"


#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-Memory"

//#define MEMPOOL_DEBUG

/*
* the design of the memory pool is specially for OpenMax Port Buffer management.
* 1) the memory block allocation and free are running at the same direction. For the factor of multimedia playing, the buffers are handled in the strict turn.
*     for example: buffer #1, #2, #3 are allocated -> consume buffer #1 -> allocate buffer #4 -> consume buffer #2, #3 ....
*     
*/

static magMemBlock_t *createMemBlock(magMempoolHandle hMemPool, magMempoolInternal_t *parent, unsigned int start, unsigned int end, unsigned char *p){
    magMemBlock_t *mb = NULL;
    List_t *tmpNode = NULL;

    if (end <= start){
        AGILE_LOGE("[0x%x]: ERROR: end:%d is less than start:%d", hMemPool, end, start);
        return NULL;
    }

    /*double check the validation of the mb*/
    if ((parent->pMemPoolBuf) && (parent->pMemPoolBuf + start != p)){
        AGILE_LOGE("[0x%x]: pool buffer 0x%x, start=%d does not match the buffer pointer 0x%x!!!", 
                    hMemPool, parent->pMemPoolBuf, start, p);
    }
    
    tmpNode = hMemPool->unusedMBListHead.next;

    if (tmpNode != &hMemPool->unusedMBListHead){
        mb = list_entry(tmpNode, magMemBlock_t, node);
        list_del(tmpNode);
    }else{
        mb = (magMemBlock_t *)mag_mallocz(sizeof(magMemBlock_t));
    }
    
    if (NULL == mb)
        return NULL;

    mb->start = start;
    mb->end   = end - 1;
    mb->pBuf  = p;
    mb->pMemPool = parent;
    INIT_LIST(&mb->node);

    return mb;
}

static magMempoolInternal_t *createMemPoolInter(magMempoolHandle hMemPool, unsigned int bytes){
    magMempoolInternal_t *pInterMP = NULL;
    unsigned char *pMPBuf = NULL;
    magMemBlock_t *fmb = NULL;
    
    pInterMP = (magMempoolInternal_t *)mag_mallocz(sizeof(magMempoolInternal_t));
    if (NULL == pInterMP){
        return NULL;
    }

    pMPBuf = (unsigned char *)mag_mallocz(bytes);
    if (NULL == pMPBuf){
        goto failure;
    }

    INIT_LIST(&pInterMP->node);
    list_add(&pInterMP->node, &hMemPool->memPoolListHead);

    INIT_LIST(&pInterMP->freeMBListHead);
    fmb = createMemBlock(hMemPool, pInterMP, 0, bytes, pMPBuf);
    if (NULL == fmb){
        goto failure;
    }else{
        AGILE_LOGV("[0x%x]: physical buffer - start: 0x%x, end :0x%x", hMemPool, fmb->pBuf, fmb->pBuf + fmb->end);
    }
    
    list_add_tail(&fmb->node, &pInterMP->freeMBListHead);

    pInterMP->memPoolSize = bytes;
    pInterMP->pMemPoolBuf = pMPBuf;
    pInterMP->activeMBNode = &fmb->node;
    pInterMP->index = ++hMemPool->MemPoolTotal;
    
    return pInterMP;
    
failure:
    mag_free(pInterMP);
    mag_free(fmb);
    mag_free(pMPBuf);
    return NULL;
    
}

/*in any time, there are at most 2 free MBs are available.*/
static MagErr_t allocateBuffer(magMempoolHandle hMemPool, magMempoolInternal_t *mpInter, unsigned int bytes, unsigned char **getBuffer){
    magMemBlock_t *mb = NULL;
    magMemBlock_t *mb2 = NULL;
    magMemBlock_t *allocmb = NULL;
    // List_t *tmpNodeInter = NULL;
    List_t *nextActive = NULL;
    // unsigned int order = 0;
    int offset = 0;
    
    /*for debugging*/
    // magMemBlock_t *mbdebug;
    
    *getBuffer = NULL;    
    
    mb = (magMemBlock_t *)list_entry(mpInter->activeMBNode, magMemBlock_t, node);

    if (mpInter->activeMBNode->next != &mpInter->freeMBListHead){
        mb2 = (magMemBlock_t *)list_entry(mpInter->activeMBNode->next, magMemBlock_t, node); 
        nextActive = mpInter->activeMBNode->next;
    }else if (mpInter->activeMBNode->prev != &mpInter->freeMBListHead){
        mb2 = (magMemBlock_t *)list_entry(mpInter->activeMBNode->prev, magMemBlock_t, node);
        nextActive = mpInter->activeMBNode->prev;
    }

#ifdef MEMPOOL_DEBUG
    if (mb2)
        AGILE_LOGV("[0x%x]: Before allocateBuffer: MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d]. Request %d bytes", 
                    hMemPool, mb->start, mb->end, 
                    mb2->start, mb2->end, mpInter->index, bytes);
    else
        AGILE_LOGV("[0x%x]: Before allocateBuffer: MB1(s:%d - e:%d)[mp index: %d]. Request %d bytes", 
                    hMemPool, mb->start, mb->end, mpInter->index, bytes);
#endif

    if ((mb->end - mb->start + 1) >= bytes){
        allocmb = createMemBlock(hMemPool, mpInter, mb->start, mb->start + bytes, mb->pBuf);
        if (NULL == allocmb){
            return MAG_NoMemory;
        }
        list_add_tail(&allocmb->node, &hMemPool->allocatedMBListHead);
        *getBuffer = allocmb->pBuf;

        if ((mb->start + bytes) < (mb->end + 1)){
            mb->start += bytes;
            mb->pBuf  += bytes;
        }else if ((mb->start + bytes) == (mb->end + 1)){
            mb->start += bytes;
            mb->end   += 1;
            mb->pBuf  += bytes;
        }
    }else{
        if (mb->start == mb->end)
            offset = 0;
        else
            offset = 1;
            
        if (mb->end + offset == mpInter->memPoolSize){
            /*the MB1 is used up, start to use MB2*/
            if (NULL != mb2){
                if ((mb2->end - mb2->start + 1) >= bytes){
                    allocmb = createMemBlock(hMemPool, mpInter, mb2->start, mb2->start + bytes, mb2->pBuf);
                    if (NULL == allocmb){
                        return MAG_NoMemory;
                    }
                    list_add_tail(&allocmb->node, &hMemPool->allocatedMBListHead);
                    *getBuffer = allocmb->pBuf;

                    mb2->start += bytes;
                    mb2->pBuf  += bytes;

                    if ((mb2->start + bytes) == (mb2->end + 1)){
                        mb2->end += 1;
                    }
                    mpInter->activeMBNode = nextActive;
                }
            }
        }else{
            AGILE_LOGV("MB[s:%d - e:%d] is not the nearest end MB, don't switch activeMB", mb->start, mb->end);
        }
    }

#ifdef MEMPOOL_DEBUG
    if (NULL == *getBuffer){
        if (mb2)
            AGILE_LOGD("[0x%x]: MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d] do not fit the size(%d bytes)", 
                        hMemPool, mb->start, mb->end, 
                        mb2->start, mb2->end, mpInter->index, bytes);
        else
            AGILE_LOGD("[0x%x]: MB1(s:%d - e:%d)[mp index: %d] does not fit the size(%d bytes)", 
                        hMemPool, mb->start, mb->end, mpInter->index, bytes);
    }else{
        if (mb2)
            AGILE_LOGV("[0x%x]: After allocateBuffer: MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d]. Request %d bytes(get buffer:0x%x)", 
                        hMemPool, mb->start, mb->end, 
                        mb2->start, mb2->end, mpInter->index, bytes, *getBuffer);
        else
            AGILE_LOGV("[0x%x]: After allocateBuffer: MB1(s:%d - e:%d)[mp index: %d]. Request %d bytes(get buffer:0x%x)", 
                        hMemPool, mb->start, mb->end, mpInter->index, bytes, *getBuffer);
    }

#else
    if (NULL == *getBuffer){
        if (mb2)
            AGILE_LOGD("[0x%x]: MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d] do not fit the size(%d bytes)", 
                        hMemPool,
                        mb->start, mb->end, 
                        mb2->start, mb2->end, mpInter->index, bytes);
        else
            AGILE_LOGD("[0x%x]: MB1(s:%d - e:%d)[mp index: %d] does not fit the size(%d bytes)(get buffer:0x%x)", 
                        hMemPool, mb->start, mb->end, mpInter->index, bytes, *getBuffer);
    }
#endif

    return MAG_ErrNone;

#if 0    
    tmpNodeInter = mpInter->freeMBListHead.next;
    
    while (tmpNodeInter != &mpInter->freeMBListHead){
        mb = (magMemBlock_t *)list_entry(tmpNodeInter, magMemBlock_t, node);

        if(mb->end == mpInter->memPoolSize){
            if (order != 0){
                AGILE_LOGE("the MB nearest to the end of MemPool is NOT the first node. WRONG!!!!!!!!");
                return MAG_NoMemory;
            }
            
            if ((mb->end - mb->start) > bytes){
                allocmb = createMemBlock(hMemPool, mpInter, mb->start, mb->start + bytes, mb->pBuf);
                if (NULL == allocmb){
                    return MAG_NoMemory;
                }
                list_add_tail(&allocmb->node, &hMemPool->allocatedMBListHead);
                *getBuffer = mb->pBuf;
                
                mb->start += bytes;
                mb->pBuf  += bytes;
                break;
            }else{
                AGILE_LOGD("[B1]MB(s:%d - e:%d) does not fit for required size(%d bytes)", mb->start, mb->end, bytes);
            }
        }else{
            if ((mb->end - mb->start) > bytes){
                allocmb = createMemBlock(hMemPool, mpInter, mb->start, mb->start + bytes, mb->pBuf);
                if (NULL == allocmb){
                    return MAG_NoMemory;
                }
                list_add_tail(&allocmb->node, &hMemPool->allocatedMBListHead);
                *getBuffer = mb->pBuf;
                
                mb->start += bytes;
                mb->pBuf  += bytes;
                break;
            }else{
                AGILE_LOGD("[B2]MB(s:%d - e:%d) does not fit for required size(%d bytes)", mb->start, mb->end, bytes);
            }
        }
        order++;
        tmpNodeInter = tmpNodeInter->next;
    }
    return MAG_ErrNone;
#endif
}

static magMemBlock_t *findAllocMemBlock(magMempoolHandle hMemPool, void *pBuf){
    List_t *pNode = NULL;
    magMemBlock_t *mb = NULL;
    magMemBlock_t *match_mb = NULL;
    
    List_t *debugNode = NULL;
    int debugCount = 1;

    pNode = hMemPool->allocatedMBListHead.next;
    
    if (pNode != &hMemPool->allocatedMBListHead){
        mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
        if (pBuf != mb->pBuf){
            AGILE_LOGE("[0x%x]: the free buffer[0x%x] is NOT the first node.", hMemPool, pBuf);
            /*only for debugging purpose*/
            debugNode = pNode->next;
            debugCount++;
            while (debugNode != &hMemPool->allocatedMBListHead){
                match_mb = (magMemBlock_t *)list_entry(debugNode, magMemBlock_t, node);
                AGILE_LOGE("buffer list: #%d - 0x%x", debugCount, match_mb->pBuf);
                if (pBuf == match_mb->pBuf){
                    mb = match_mb;
                    AGILE_LOGE("[0x%x]: the pBuf[0x%x] is at the %dth places in AllocList", hMemPool, pBuf, debugCount);
                }
                debugCount++;
                debugNode = debugNode->next;
            }
            /*debugging completes*/
        }      
    }else{
        AGILE_LOGE("[0x%x]: The allocated MB list is empty! pBuf[0x%x] is the wild buffer.", hMemPool, pBuf);
    }
    return mb;
}

static void sortMemPoolList(magMempoolHandle hMemPool){
    List_t *l1 = NULL;
    List_t *l2 = NULL;
    magMempoolInternal_t *current = NULL;
    magMempoolInternal_t *next = NULL;
    unsigned int switch_flag = 0;
    
    l1 = hMemPool->memPoolListHead.next;
    while (1){
        current = (magMempoolInternal_t *)list_entry(l1, magMempoolInternal_t, node);
        if (l1->next != &hMemPool->memPoolListHead){
            l2 = l1->next;
            next = (magMempoolInternal_t *)list_entry(l2, magMempoolInternal_t, node);
        }else{
            if (switch_flag){
                l1 = hMemPool->memPoolListHead.next;
                switch_flag = 0;
                continue;
            }else{
                break;
            }
        }
        
        if (current->index > next->index){
            list_del(l1);
            list_add(l1, l2);
            switch_flag++;
        }else{
            l1 = l1->next;
        }
    }

    //For debugging
    l1 = hMemPool->memPoolListHead.next;
    while (l1 != &hMemPool->memPoolListHead){
        current = (magMempoolInternal_t *)list_entry(l1, magMempoolInternal_t, node);
        AGILE_LOGD("[0x%x]: MemPool Index: %d", hMemPool, current->index);
        l1 = l1->next;
    }
}
magMempoolHandle magMemPoolCreate(unsigned int bytes, ui32 flags){
    magMempoolHandle hMP = NULL;
    magMempoolInternal_t *pInterMP = NULL;
    int rc = 0;
    
    hMP = (magMempoolHandle)mag_mallocz(sizeof(magMemPoolMgr_t));
    if (NULL == hMP){
        return NULL;
    }  
    
    INIT_LIST(&hMP->allocatedMBListHead);
    INIT_LIST(&hMP->unusedMBListHead);
    INIT_LIST(&hMP->memPoolListHead);
    if (flags == 0)
        hMP->ctrlFlag = NO_AUTO_EXPANDING;
    else
        hMP->ctrlFlag = AUTO_EXPANDING;

    rc = pthread_mutex_init(&hMP->mutex, NULL /* default attributes */);
    if (rc != 0)
        goto failure;   
    
    pInterMP = createMemPoolInter(hMP, bytes);
    if(NULL == pInterMP)
        goto failure1;
    
    return hMP;

failure1:
    pthread_mutex_destroy(&hMP->mutex);
    
failure:
    mag_free(hMP);
        
    return NULL;
}

void *magMemPoolGetBuffer(magMempoolHandle hMemPool, unsigned int bytes){
    List_t *tmpNodeOuter = NULL;
    magMempoolInternal_t *mpInter = NULL;
    unsigned char * getBuffer = NULL;
    
    pthread_mutex_lock(&hMemPool->mutex);

    tmpNodeOuter = hMemPool->memPoolListHead.next;

    while (tmpNodeOuter != &hMemPool->memPoolListHead){
        mpInter = (magMempoolInternal_t *)list_entry(tmpNodeOuter, magMempoolInternal_t, node);
        
        if (MAG_ErrNone == allocateBuffer(hMemPool, mpInter, bytes, &getBuffer)){
            if (getBuffer){
                /* if the memory pool has the free space for the allocation, move it to the head of the memory pool list. 
                           * It would be firstly used in next allocation.
                          */
                list_del(tmpNodeOuter);
                list_add(tmpNodeOuter, &hMemPool->memPoolListHead);
                goto find;
            }
        }else{
            pthread_mutex_unlock(&hMemPool->mutex);
            return NULL;
        }
        
        tmpNodeOuter = tmpNodeOuter->next;
    }

    if (hMemPool->ctrlFlag == AUTO_EXPANDING){
        AGILE_LOGD("[0x%x]: failed to find suitable size mb for size(%d) in exised mempool. create another mempool", 
                    hMemPool, bytes);

        /*reorder the mempool list in the index descending*/
        sortMemPoolList(hMemPool);
        
        /*create new memory pool with 4 x "required buffer size"*/
        mpInter = createMemPoolInter(hMemPool, BUF_ALIGN((bytes < 4*1024 ? 4*4*1024 : 4*bytes), 4*1024));
        if (MAG_ErrNone == allocateBuffer(hMemPool, mpInter, bytes, &getBuffer)){
            if (getBuffer)
                goto find;
            else
                AGILE_LOGE("[0x%x]: Should not be here. the newly created mem pool should always meet the requirments", hMemPool);
        }else{ 
            pthread_mutex_unlock(&hMemPool->mutex);
            return NULL;
        }
    }
    
find:
    // AGILE_LOGV("[0x%x]: find the matching buffer - 0x%x", hMemPool, getBuffer);
    pthread_mutex_unlock(&hMemPool->mutex);
    return (void *)getBuffer;
}

MagErr_t magMemPoolPutBuffer(magMempoolHandle hMemPool, void *pBuf){
    magMempoolInternal_t *mpool = NULL;
    magMemBlock_t *mbfreed = NULL;
    List_t *tmpNode = NULL;
    magMemBlock_t *mb = NULL;

    List_t *lfirst = NULL;
    magMemBlock_t *firstMB = NULL;
    List_t *lsecond = NULL;
    magMemBlock_t *secondMB = NULL;
    
    int debugCount = 0;
    int offset = 0;
    
    if ((NULL == hMemPool) || (NULL == pBuf))
        return MAG_BadParameter;

    pthread_mutex_lock(&hMemPool->mutex);
    
    mb = findAllocMemBlock(hMemPool, pBuf);

    if (NULL != mb){
        list_del(&mb->node);
        mpool = mb->pMemPool;
        tmpNode = mpool->freeMBListHead.next;

        while(tmpNode != &mpool->freeMBListHead){
            mbfreed = (magMemBlock_t *)list_entry(tmpNode, magMemBlock_t, node);
            debugCount++;
#ifdef MEMPOOL_DEBUG
            AGILE_LOGV("[0x%x]: traverse free mb list: node (start:%d - end:%d). alloc node[start:%d - end:%d]", 
                        hMemPool, mbfreed->start, mbfreed->end, mb->start, mb->end);
#endif
            if (mbfreed->start == mbfreed->end){
                offset = 0;
            }else{
                offset = 1;
            }
            
            if(mbfreed->start + offset == mb->end){
                /*the put MB locates right brefore the freed MB*/
                mbfreed->start = mb->start;
                list_add(&mb->node, &hMemPool->unusedMBListHead);
                goto done;
            }else if (mbfreed->end + offset == mb->start){
                /*the put MB locates right after the freed MB*/
                mbfreed->end = mb->end;
                list_add(&mb->node, &hMemPool->unusedMBListHead);
                goto done;
            }
            
            tmpNode = tmpNode->next;
        }
        AGILE_LOGD("[0x%x]: Can't find the free MB(total %d) that is consecutive to the alloc MB", hMemPool, debugCount);
        list_add_tail(&mb->node, &mpool->freeMBListHead);        
done:
        /*revisit the free list to merge the MB if any*/
        lfirst = mpool->freeMBListHead.next;
        firstMB = (magMemBlock_t *)list_entry(lfirst, magMemBlock_t, node);
        lsecond = lfirst->next;
        secondMB = (magMemBlock_t *)list_entry(lsecond, magMemBlock_t, node);;

        if ((lsecond != &mpool->freeMBListHead) && (lsecond->next != &mpool->freeMBListHead))
                AGILE_LOGE("[0x%x]: ERROR: has more than 2 free nodes in the list. WRONG!!!", hMemPool);
        
        if (lsecond != &mpool->freeMBListHead){
            if (secondMB->start == secondMB->end){
                offset = 0;
            }else{
                offset = 1;
            }
            if (secondMB->end + offset == firstMB->start){
                AGILE_LOGV("[0x%x]: merge the adjacent MB[1]: 1st[s:%d - e:%d] and 2nd[s:%d - e:%d]", 
                            hMemPool,
                            firstMB->start, firstMB->end, 
                            secondMB->start, secondMB->end);
                firstMB->start = secondMB->start;
                list_del(lsecond);
                list_add(lsecond, &hMemPool->unusedMBListHead); 
                mpool->activeMBNode = lfirst;
            }else if (secondMB->start + offset == firstMB->end){
                AGILE_LOGV("[0x%x]: merge the adjacent MB[2]: 1st[s:%d - e:%d] and 2nd[s:%d - e:%d]", 
                            hMemPool,
                            firstMB->start, firstMB->end, 
                            secondMB->start, secondMB->end);
                firstMB->end = secondMB->end;
                list_del(lsecond);
                list_add(lsecond, &hMemPool->unusedMBListHead); 
                mpool->activeMBNode = lfirst; 
            } 
            secondMB->pBuf = mpool->pMemPoolBuf + secondMB->start;
        }
        firstMB->pBuf  = mpool->pMemPoolBuf + firstMB->start;

#ifdef MEMPOOL_DEBUG        
        //debugging
        tmpNode = mpool->freeMBListHead.next;
        AGILE_LOGV("**********");
        while(tmpNode != &mpool->freeMBListHead){
            mbfreed = (magMemBlock_t *)list_entry(tmpNode, magMemBlock_t, node);
            AGILE_LOGV("[0x%x]: free mb list: node (start:%d - end:%d)[mempool index: %d]", 
                        hMemPool, mbfreed->start, mbfreed->end, mpool->index);
            tmpNode = tmpNode->next;
        }
        AGILE_LOGV("**********");
#endif
        pthread_mutex_unlock(&hMemPool->mutex);
        return MAG_ErrNone;
    }else{
        AGILE_LOGV("[0x%x]: failed to find the buffer!", hMemPool);
        pthread_mutex_unlock(&hMemPool->mutex);
        return MAG_Failure;
    }  
}

MagErr_t magMemPoolReset(magMempoolHandle hMemPool){
    return MAG_ErrNone;
}

void magMemPoolDestroy(magMempoolHandle hMemPool){
    List_t *tmpNode = NULL;
    List_t *pNode = NULL;
    magMempoolInternal_t * mpool = NULL;
    magMemBlock_t *mb = NULL;

    AGILE_LOGV("enter!");
    if (NULL == hMemPool){
        AGILE_LOGD("hMemPool is NULL. quit!");
        return;
    }
    
    pthread_mutex_lock(&hMemPool->mutex);
    
    tmpNode = hMemPool->memPoolListHead.next;

    while (tmpNode != &hMemPool->memPoolListHead){
        mpool = (magMempoolInternal_t *)list_entry(tmpNode, magMempoolInternal_t, node);
        pNode = mpool->freeMBListHead.next;
        while (pNode != &mpool->freeMBListHead){
            list_del(pNode);
            mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
            mag_free(mb);
            pNode = mpool->freeMBListHead.next;
        }
        list_del(tmpNode);
        mag_free(mpool->pMemPoolBuf);
        mag_free(mpool);
        
        tmpNode = hMemPool->memPoolListHead.next;
    }

    pNode = hMemPool->allocatedMBListHead.next;
    while (pNode != &hMemPool->allocatedMBListHead){
        list_del(pNode);
        mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
        mag_free(mb);
        pNode = hMemPool->allocatedMBListHead.next;
    }

    pNode = hMemPool->unusedMBListHead.next;
    while (pNode != &hMemPool->unusedMBListHead){
        list_del(pNode);
        mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
        mag_free(mb);
        pNode = hMemPool->unusedMBListHead.next;
    }

    pthread_mutex_unlock(&hMemPool->mutex);

    pthread_mutex_destroy(&hMemPool->mutex);
    mag_free(hMemPool);
    AGILE_LOGV("exit!");
}

void magMemPoolDump(magMempoolHandle hMemPool){
    List_t *tmpNode = NULL;
    List_t *pNode = NULL;
    magMempoolInternal_t * mpool = NULL;
    magMemBlock_t *mb = NULL;

    pthread_mutex_lock(&hMemPool->mutex);
    
    tmpNode = hMemPool->memPoolListHead.next;
    while (tmpNode != &hMemPool->memPoolListHead){
        mpool = (magMempoolInternal_t *)list_entry(tmpNode, magMempoolInternal_t, node);
        AGILE_LOGD("**********");
        AGILE_LOGD("[0x%x]: mem pool index = %d", hMemPool, mpool->index);
        pNode = mpool->freeMBListHead.next;
        while (pNode != &mpool->freeMBListHead){
            mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
            AGILE_LOGD("[0x%x]: MB [start: %d - end: %d]", hMemPool, mb->start, mb->end);
            pNode = pNode->next;
        }
        AGILE_LOGD("**********");
        tmpNode = tmpNode->next;
    }

    pthread_mutex_unlock(&hMemPool->mutex);
}
