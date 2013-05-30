#include "Mag_mem.h"
#include "Mag_mempool.h"
#include "Mag_list.h"
#include "agilelog.h"
/*
* the design of the memory pool is specially for OpenMax Port Buffer management.
* 1) the memory block allocation and free are running at the same direction. For the factor of multimedia playing, the buffers are handled in the strict turn.
*     for example: buffer #1, #2, #3 are allocated -> consume buffer #1 -> allocate buffer #4 -> consume buffer #2, #3 ....
*     
*/

static magMemBlock_t *createMemBlock(magMempoolHandle hMemPool, magMempoolInternal_t *parent, unsigned int start, unsigned int end, unsigned char *p){
    magMemBlock_t *mb;
    List_t *tmpNode;

    if (end <= start){
        AGILE_LOGE("ERROR: end:%d is less than start:%d", end, start);
        return NULL;
    }
    
    tmpNode = hMemPool->unusedMBListHead.next;

    if (tmpNode != &hMemPool->unusedMBListHead){
        mb = list_entry(tmpNode, magMemBlock_t, node);
        list_del(tmpNode);
    }else{
        mb = mag_mallocz(sizeof(magMemBlock_t));
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
    magMempoolInternal_t *pInterMP;
    unsigned char *pMPBuf;
    magMemBlock_t *fmb;
    int rc;
    
    pInterMP = (magMempoolInternal_t *)mag_mallocz(sizeof(magMempoolInternal_t));
    if (NULL == pInterMP){
        return NULL;
    }

    pMPBuf = (unsigned char *)mag_mallocz(bytes);
    if (NULL == pMPBuf){
        goto bail;
    }

    INIT_LIST(&pInterMP->node);
    list_add(&pInterMP->node, &hMemPool->memPoolListHead);

    INIT_LIST(&pInterMP->freeMBListHead);
    fmb = createMemBlock(hMemPool, pInterMP, 0, bytes, pMPBuf);
    if (NULL == fmb){
        goto bail;
    }
    
    list_add_tail(&fmb->node, &pInterMP->freeMBListHead);

    pInterMP->memPoolSize = bytes;
    pInterMP->pMemPoolBuf = pMPBuf;
    pInterMP->activeMBNode = &fmb->node;
    pInterMP->index = ++hMemPool->MemPoolTotal;
    
    return pInterMP;
    
bail:
    mag_free(pInterMP);
    mag_free(fmb);
    mag_free(pMPBuf);
    return NULL;
    
}

static MagErr_t allocateBuffer(magMempoolHandle hMemPool, magMempoolInternal_t *mpInter, unsigned int bytes, unsigned char **getBuffer){
    magMemBlock_t *mb = NULL;
    magMemBlock_t *mb2 = NULL;
    magMemBlock_t *allocmb;
    List_t *tmpNodeInter;
    List_t *nextActive;
    unsigned int order = 0;
    int offset = 0;
    
    /*for debugging*/
    magMemBlock_t *mbdebug;
    
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
        AGILE_LOGV("Before allocateBuffer: MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d]. Request %d bytes", mb->start, mb->end, 
                    mb2->start, mb2->end, mpInter->index, bytes);
    else
        AGILE_LOGV("Before allocateBuffer: MB1(s:%d - e:%d)[mp index: %d]. Request %d bytes", mb->start, mb->end, mpInter->index, bytes);
#endif

    if ((mb->end - mb->start + 1) >= bytes){
        allocmb = createMemBlock(hMemPool, mpInter, mb->start, mb->start + bytes, mb->pBuf);
        if (NULL == allocmb){
            return MAG_NoMemory;
        }
        list_add_tail(&allocmb->node, &hMemPool->allocatedMBListHead);
        *getBuffer = mb->pBuf;

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
            if (NULL != mb2){
                if ((mb2->end - mb2->start + 1) >= bytes){
                    allocmb = createMemBlock(hMemPool, mpInter, mb2->start, mb2->start + bytes, mb2->pBuf);
                    if (NULL == allocmb){
                        return MAG_NoMemory;
                    }
                    list_add_tail(&allocmb->node, &hMemPool->allocatedMBListHead);
                    *getBuffer = mb2->pBuf;

                    mb2->start += bytes;
                    mb2->pBuf  += bytes;
                    if ((mb->start + bytes) == (mb->end + 1)){
                        mb->end   += 1;
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
            AGILE_LOGD("MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d] do not fit the size(%d bytes)", mb->start, mb->end, 
                        mb2->start, mb2->end, mpInter->index, bytes);
        else
            AGILE_LOGD("MB1(s:%d - e:%d)[mp index: %d] does not fit the size(%d bytes)", mb->start, mb->end, mpInter->index, bytes);
    }else{
        if (mb2)
            AGILE_LOGV("After allocateBuffer: MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d]. Request %d bytes", mb->start, mb->end, 
                        mb2->start, mb2->end, mpInter->index, bytes);
        else
            AGILE_LOGV("After allocateBuffer: MB1(s:%d - e:%d)[mp index: %d]. Request %d bytes", mb->start, mb->end, mpInter->index, bytes);
    }
#else
    if (NULL == *getBuffer){
        if (mb2)
            AGILE_LOGD("MB1(s:%d - e:%d) or MB2(s:%d - e:%d)[mp index: %d] do not fit the size(%d bytes)", mb->start, mb->end, 
                        mb2->start, mb2->end, mpInter->index, bytes);
        else
            AGILE_LOGD("MB1(s:%d - e:%d)[mp index: %d] does not fit the size(%d bytes)", mb->start, mb->end, mpInter->index, bytes);
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
    List_t *allocMBList = &hMemPool->allocatedMBListHead;
    List_t *pNode = allocMBList->next;
    magMemBlock_t *mb;
    
    List_t *debugNode;
    int debugCount = 1;
    
    if (pNode != allocMBList){
        mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
        if (pBuf != mb->pBuf){
            AGILE_LOGE("the free buffer[0x%x] is NOT the first node.", pBuf);
            /*only for debugging purpose*/
            debugNode = pNode->next;
            debugCount++;
            while (debugNode != allocMBList){
                mb = (magMemBlock_t *)list_entry(debugNode, magMemBlock_t, node);
                if (pBuf == mb->pBuf)
                    AGILE_LOGE("the pBuf[0x%x] is at the %dth places in AllocList", pBuf, debugCount);
                debugCount++;
                debugNode = debugNode->next;
            }
            /*debugging completes*/
            return NULL;
        }
        return mb;      
    }else{
        AGILE_LOGE("The allocated MB list is empty! pBuf[0x%x] is the wild buffer.", pBuf);
    }
    return NULL;
}

static void sortMemPoolList(magMempoolHandle hMemPool){
    List_t *l1, *l2;
    magMempoolInternal_t *current, *next;
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
        AGILE_LOGD("MemPool Index: %d", current->index);
        l1 = l1->next;
    }
}
magMempoolHandle magMemPoolCreate(unsigned int bytes){
    magMempoolHandle hMP;
    magMempoolInternal_t *pInterMP;
    int rc;
    
    hMP = (magMempoolHandle)mag_mallocz(sizeof(magMemPoolMgr_t));
    if (NULL == hMP){
        return NULL;
    }  
    
    INIT_LIST(&hMP->allocatedMBListHead);
    INIT_LIST(&hMP->unusedMBListHead);
    INIT_LIST(&hMP->memPoolListHead);
    rc = pthread_mutex_init(&hMP->mutex, NULL /* default attributes */);
    if (rc != 0)
        goto failure;   
    
    pInterMP = createMemPoolInter(hMP, bytes);
    if(NULL == pInterMP)
        goto failure;
    
    return hMP;
    
failure:
    mag_free(hMP);
        
    return NULL;
}

void *magMemPoolGetBuffer(magMempoolHandle hMemPool, unsigned int bytes){
    List_t *tmpNodeOuter;
    magMempoolInternal_t *mpInter;
    unsigned char * getBuffer = NULL;
    
    pthread_mutex_lock(&hMemPool->mutex);

    tmpNodeOuter = hMemPool->memPoolListHead.next;

    while (tmpNodeOuter != &hMemPool->memPoolListHead){
        mpInter = (magMempoolInternal_t *)list_entry(tmpNodeOuter, magMempoolInternal_t, node);
        
        if (MAG_ErrNone == allocateBuffer(hMemPool, mpInter, bytes, &getBuffer)){
            if (getBuffer){
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
    AGILE_LOGD("failed to find suitable size mb for size(%d) in exised mempool. create another mempool", bytes);

    /*reorder the mempool list in the index descending*/
    sortMemPoolList(hMemPool);
    
    /*create new memory pool with 4 x "required buffer size"*/
    mpInter = createMemPoolInter(hMemPool, BUF_ALIGN(4*bytes, 4*1024));
    if (MAG_ErrNone == allocateBuffer(hMemPool, mpInter, bytes, &getBuffer)){
        if (getBuffer)
            goto find;
        else
            AGILE_LOGE("Should not be here. the newly created mem pool should always meet the requirments");
    }else{ 
        pthread_mutex_unlock(&hMemPool->mutex);
        return NULL;
    }
    
find:
    AGILE_LOGD("find the matching buffer");
    pthread_mutex_unlock(&hMemPool->mutex);
    return (void *)getBuffer;
}

MagErr_t magMemPoolPutBuffer(magMempoolHandle hMemPool, void *pBuf){
    magMempoolInternal_t *mpool;
    magMemBlock_t *mbfreed;
    List_t *tmpNode;
    magMemBlock_t *mb;

    List_t *lfirst;
    magMemBlock_t *firstMB;
    List_t *lsecond;
    magMemBlock_t *secondMB;
    
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
            AGILE_LOGV("traverse free mb list: node (start:%d - end:%d). alloc node[start:%d - end:%d]", mbfreed->start, 
                                                                                     mbfreed->end, mb->start, mb->end);
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
        AGILE_LOGD("Can't find the free MB(total %d) that is consecutive to the alloc MB", debugCount);
        list_add_tail(&mb->node, &mpool->freeMBListHead);        
done:
        /*revisit the free list to merge the MB if any*/
        lfirst = mpool->freeMBListHead.next;
        firstMB = (magMemBlock_t *)list_entry(lfirst, magMemBlock_t, node);
        lsecond = lfirst->next;
        secondMB = (magMemBlock_t *)list_entry(lsecond, magMemBlock_t, node);;

        if ((lsecond != &mpool->freeMBListHead) && (lsecond->next != &mpool->freeMBListHead))
                AGILE_LOGE("ERROR: has more than 2 free nodes in the list. WRONG!!!");
        
        if (lsecond != &mpool->freeMBListHead){
            if (secondMB->start == secondMB->end){
                offset = 0;
            }else{
                offset = 1;
            }
            if (secondMB->end + offset == firstMB->start){
                AGILE_LOGV("merge the adjacent MB[1]: 1st[s:%d - e:%d] and 2nd[s:%d - e:%d]", firstMB->start, firstMB->end, 
                            secondMB->start, secondMB->end);
                firstMB->start = secondMB->start;
                list_del(lsecond);
                list_add(lsecond, &hMemPool->unusedMBListHead); 
                mpool->activeMBNode = lfirst;
            }else if (secondMB->start + offset == firstMB->end){
                AGILE_LOGV("merge the adjacent MB[2]: 1st[s:%d - e:%d] and 2nd[s:%d - e:%d]", firstMB->start, firstMB->end, 
                            secondMB->start, secondMB->end);
                firstMB->end = secondMB->end;
                list_del(lsecond);
                list_add(lsecond, &hMemPool->unusedMBListHead); 
                mpool->activeMBNode = lfirst; 
            }   
        }

        //debugging
        tmpNode = mpool->freeMBListHead.next;
        AGILE_LOGV("**********");
        while(tmpNode != &mpool->freeMBListHead){
            mbfreed = (magMemBlock_t *)list_entry(tmpNode, magMemBlock_t, node);
            AGILE_LOGV("free mb list: node (start:%d - end:%d)[mempool index: %d]", mbfreed->start, mbfreed->end, mpool->index);
            tmpNode = tmpNode->next;
        }
        AGILE_LOGV("**********");
        pthread_mutex_unlock(&hMemPool->mutex);
        return MAG_ErrNone;
    }else{
        pthread_mutex_unlock(&hMemPool->mutex);
        return MAG_Failure;
    }  
}

MagErr_t magMemPoolReset(magMempoolHandle hMemPool){

}

void magMemPoolDestroy(magMempoolHandle hMemPool){
    List_t *tmpNode;
    List_t *pNode;
    magMempoolInternal_t * mpool;
    magMemBlock_t *mb;

    pthread_mutex_lock(&hMemPool->mutex);
    
    tmpNode = hMemPool->memPoolListHead.next;

    while (tmpNode != &hMemPool->memPoolListHead){
        mpool = (magMempoolInternal_t *)list_entry(tmpNode, magMempoolInternal_t, node);
        pNode = mpool->freeMBListHead.next;
        while (pNode != &mpool->freeMBListHead){
            list_del(pNode);
            mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
            mag_freep(mb);
            pNode = mpool->freeMBListHead.next;
        }
        list_del(tmpNode);
        mag_freep(mpool->pMemPoolBuf);
        mag_freep(mpool);
        
        tmpNode = hMemPool->memPoolListHead.next;
    }

    pNode = hMemPool->allocatedMBListHead.next;
    while (pNode != &hMemPool->allocatedMBListHead){
        list_del(pNode);
        mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
        mag_freep(mb);
        pNode = hMemPool->allocatedMBListHead.next;
    }

    pNode = hMemPool->unusedMBListHead.next;
    while (pNode != &hMemPool->unusedMBListHead){
        list_del(pNode);
        mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
        mag_freep(mb);
        pNode = hMemPool->unusedMBListHead.next;
    }

    pthread_mutex_unlock(&hMemPool->mutex);

    pthread_mutex_destroy(&hMemPool->mutex);
    mag_freep(hMemPool);
}

void magMemPoolDump(magMempoolHandle hMemPool){
    List_t *tmpNode;
    List_t *pNode;
    magMempoolInternal_t * mpool;
    magMemBlock_t *mb;

    pthread_mutex_lock(&hMemPool->mutex);
    
    tmpNode = hMemPool->memPoolListHead.next;
    while (tmpNode != &hMemPool->memPoolListHead){
        mpool = (magMempoolInternal_t *)list_entry(tmpNode, magMempoolInternal_t, node);
        AGILE_LOGD("**********");
        AGILE_LOGD("mem pool index = %d", mpool->index);
        pNode = mpool->freeMBListHead.next;
        while (pNode != &mpool->freeMBListHead){
            mb = (magMemBlock_t *)list_entry(pNode, magMemBlock_t, node);
            AGILE_LOGD("MB [start: %d - end: %d]", mb->start, mb->end);
            pNode = pNode->next;
        }
        AGILE_LOGD("**********");
        tmpNode = tmpNode->next;
    }

    pthread_mutex_unlock(&hMemPool->mutex);
}