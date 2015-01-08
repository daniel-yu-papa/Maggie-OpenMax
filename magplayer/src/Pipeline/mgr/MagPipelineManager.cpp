#include "MagPipelineManager.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

MagPipelineManager::MagPipelineManager(MagClock *mpClockComp):
										mpClockComp(mpClockComp){
	INIT_LIST(&mVideoPipelineHead);
	INIT_LIST(&mAudioPipelineHead);

    mpClockComp->init();
}

MagPipelineManager::~MagPipelineManager(){
}

_status_t MagPipelineManager::addVideoPipeline(MagVideoPipeline *pVideoPipeline, boolean connectToClk){
	MAG_PIPELINE_ENTRY_t *new_node;
	_status_t ret;

	new_node = (MAG_PIPELINE_ENTRY_t *)mag_mallocz(sizeof(MAG_PIPELINE_ENTRY_t));
	INIT_LIST(&new_node->node);
	new_node->pPipeline = static_cast<void *>(pVideoPipeline);
	new_node->connected = MAG_FALSE;
	list_add_tail(&new_node->node, &mVideoPipelineHead);

	if (connectToClk){
		ret = mpClockComp->connectVideoPipeline(static_cast<void *>(pVideoPipeline));
		if(ret == MAG_NO_ERROR){
			new_node->connected = MAG_TRUE;
		}else{
			AGILE_LOGE("failed to connect video pipepline[%p] to the clock component");
			return ret;
		}
	}
	return MAG_NO_ERROR;
}

_status_t MagPipelineManager::addAudioPipeline(MagAudioPipeline *pAudioPipeline, boolean connectToClk){
	MAG_PIPELINE_ENTRY_t *new_node;
	_status_t ret;

	new_node = (MAG_PIPELINE_ENTRY_t *)mag_mallocz(sizeof(MAG_PIPELINE_ENTRY_t));
	INIT_LIST(&new_node->node);
	new_node->pPipeline = static_cast<void *>(pAudioPipeline);
	new_node->connected = MAG_FALSE;
	list_add_tail(&new_node->node, &mAudioPipelineHead);

	if (connectToClk){
		ret = mpClockComp->connectAudioPipeline(static_cast<void *>(pAudioPipeline));
		if(ret == MAG_NO_ERROR){
			new_node->connected = MAG_TRUE;
		}else{
			AGILE_LOGE("failed to connect audio pipepline[%p] to the clock component");
			return ret;
		}
	}

	return MAG_NO_ERROR;
} 
	
_status_t MagPipelineManager::removeVideoPipeline(MagVideoPipeline *pVideoPipeline){
	List_t *next;

    MAG_PIPELINE_ENTRY_t *item = NULL;
    next = mVideoPipelineHead.next;
    while (next != &mVideoPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        if (item->pPipeline == pVideoPipeline){
        	list_del(&item->node);
        	if (item->connected){
        		mpClockComp->disconnectVideoPipeline(static_cast<void *>(pVideoPipeline));
        	}
        	mag_free(item);
        	break;
        }

        next = mVideoPipelineHead.next;
    }

    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::removeAudioPipeline(MagAudioPipeline *pAudioPipeline){
	List_t *next;

    MAG_PIPELINE_ENTRY_t *item = NULL;
    next = mAudioPipelineHead.next;
    while (next != &mAudioPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        if (item->pPipeline == pAudioPipeline){
        	list_del(&item->node);
        	if (item->connected){
        		mpClockComp->disconnectAudioPipeline(static_cast<void *>(pAudioPipeline));
        	}
        	mag_free(item);
        	break;
        }

        next = mAudioPipelineHead.next;
    }

    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::setup(){
    List_t *next;
    MAG_PIPELINE_ENTRY_t *item = NULL;
    MagAudioPipeline *pAudioPipeline;
    MagVideoPipeline *pVideoPipeline;

    AGILE_LOGV("Enter!");

    mpClockComp->setup();

    next = mVideoPipelineHead.next;
    while (next != &mVideoPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        pVideoPipeline = static_cast<MagVideoPipeline *>(item->pPipeline);
        pVideoPipeline->setup();
        next = next->next;
    }

    next = mAudioPipelineHead.next;
    while (next != &mAudioPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        pAudioPipeline = static_cast<MagAudioPipeline *>(item->pPipeline);
        pAudioPipeline->setup();
        next = next->next;
    }

    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::start(){
	List_t *next;
	MAG_PIPELINE_ENTRY_t *item = NULL;
	MagVideoPipeline *video;
	MagAudioPipeline *audio;
	_status_t ret;

	mpClockComp->start();

    next = mVideoPipelineHead.next;
    while (next != &mVideoPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        video = (MagVideoPipeline *)item->pPipeline;
        ret = video->start();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to start the video pipeline[%p]", video);
        	return ret;
        }
        next = next->next;
    }

    next = mAudioPipelineHead.next;
    while (next != &mAudioPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        audio = (MagAudioPipeline *)item->pPipeline;
        ret = audio->start();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to start the audio pipeline[%p]", audio);
        	return ret;
        }
        next = next->next;
    }

    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::stop(){
	List_t *next;
	MAG_PIPELINE_ENTRY_t *item = NULL;
	MagVideoPipeline *video;
	MagAudioPipeline *audio;
	_status_t ret;

    mpClockComp->stop();

    next = mVideoPipelineHead.next;
    while (next != &mVideoPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        video = (MagVideoPipeline *)item->pPipeline;
        ret = video->stop();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to stop the video pipeline[%p]", video);
        	return ret;
        }
        next = next->next;
    }

    next = mAudioPipelineHead.next;
    while (next != &mAudioPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        audio = (MagAudioPipeline *)item->pPipeline;
        ret = audio->stop();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to stop the audio pipeline[%p]", audio);
        	return ret;
        }
        next = next->next;
    }

    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::pause(ui8 flag){
	List_t *next;
	MAG_PIPELINE_ENTRY_t *item = NULL;
	MagVideoPipeline *video;
	MagAudioPipeline *audio;
	_status_t ret;

	/*if (flag & PAUSE_CLOCK_FLAG)*/
		mpClockComp->pause();

	/*if (flag & PAUSE_AV_FLAG){*/
	    next = mVideoPipelineHead.next;
	    while (next != &mVideoPipelineHead){
	        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
	        video = (MagVideoPipeline *)item->pPipeline;
	        ret = video->pause();
	        if (ret != MAG_NO_ERROR){
	        	AGILE_LOGE("failed to pause the video pipeline[%p]", video);
	        	return ret;
	        }
	        next = next->next;
	    }

	    next = mAudioPipelineHead.next;
	    while (next != &mAudioPipelineHead){
	        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
	        audio = (MagAudioPipeline *)item->pPipeline;
	        ret = audio->pause();
	        if (ret != MAG_NO_ERROR){
	        	AGILE_LOGE("failed to pause the audio pipeline[%p]", audio);
	        	return ret;
	        }
	        next = next->next;
	    }
	/*}*/
    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::resume(){
	List_t *next;
	MAG_PIPELINE_ENTRY_t *item = NULL;
	MagVideoPipeline *video;
	MagAudioPipeline *audio;
	_status_t ret;

	mpClockComp->resume();

    next = mVideoPipelineHead.next;
    while (next != &mVideoPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        video = (MagVideoPipeline *)item->pPipeline;
        ret = video->resume();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to resume the video pipeline[%p]", video);
        	return ret;
        }
        next = next->next;
    }

    next = mAudioPipelineHead.next;
    while (next != &mAudioPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        audio = (MagAudioPipeline *)item->pPipeline;
        ret = audio->resume();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to resume the audio pipeline[%p]", audio);
        	return ret;
        }
        next = next->next;
    }
    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::flush(){
	List_t *next;
	MAG_PIPELINE_ENTRY_t *item = NULL;
	MagVideoPipeline *video;
	MagAudioPipeline *audio;
	_status_t ret;

	mpClockComp->flush();

    next = mVideoPipelineHead.next;
    while (next != &mVideoPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        video = (MagVideoPipeline *)item->pPipeline;
        ret = video->flush();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to flush the video pipeline[%p]", video);
        	return ret;
        }
        next = next->next;
    }

    next = mAudioPipelineHead.next;
    while (next != &mAudioPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        audio = (MagAudioPipeline *)item->pPipeline;
        ret = audio->flush();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to flush the audio pipeline[%p]", audio);
        	return ret;
        }
        next = next->next;
    }
    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::reset(){
	List_t *next;
	MAG_PIPELINE_ENTRY_t *item = NULL;
	MagVideoPipeline *video;
	MagAudioPipeline *audio;
	_status_t ret;

    next = mVideoPipelineHead.next;
    while (next != &mVideoPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        video = (MagVideoPipeline *)item->pPipeline;
        mpClockComp->disconnectVideoPipeline(video);
        ret = video->reset();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to reset the video pipeline[%p]", video);
        	return ret;
        }
        next = next->next;
    }

    next = mAudioPipelineHead.next;
    while (next != &mAudioPipelineHead){
        item = (MAG_PIPELINE_ENTRY_t *)list_entry(next, MAG_PIPELINE_ENTRY_t, node);
        audio = (MagAudioPipeline *)item->pPipeline;
        mpClockComp->disconnectAudioPipeline(audio);
        ret = audio->reset();
        if (ret != MAG_NO_ERROR){
        	AGILE_LOGE("failed to reset the audio pipeline[%p]", audio);
        	return ret;
        }
        next = next->next;
    }

    mpClockComp->reset();

    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::setVolume(fp32 leftVolume, fp32 rightVolume){
    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::getDecodedFrame(MAG_PIPELINE_TYPE_t type, void *pPipeline, void **ppFrame){
    if (type == MAG_PIPELINE_VIDEO_TYPE){
        MagVideoPipeline *pVideoPipeline;

        pVideoPipeline = static_cast<MagVideoPipeline *>(pPipeline);
        return pVideoPipeline->getDecodedFrame(ppFrame);
    }else if (type == MAG_PIPELINE_AUDIO_TYPE){
        MagAudioPipeline *pAudioPipeline;

        pAudioPipeline = static_cast<MagAudioPipeline *>(pPipeline);
        return pAudioPipeline->getDecodedFrame(ppFrame);
    }else if (type == MAG_PIPELINE_SUBTITLE_TYPE){

    }

    return MAG_NO_ERROR;
}

_status_t MagPipelineManager::putUsedFrame(MAG_PIPELINE_TYPE_t type, void *pPipeline, void *pFrame){
    if (type == MAG_PIPELINE_VIDEO_TYPE){
        MagVideoPipeline *pVideoPipeline;

        pVideoPipeline = static_cast<MagVideoPipeline *>(pPipeline);
        return pVideoPipeline->putUsedFrame(pFrame);
    }else if (type == MAG_PIPELINE_AUDIO_TYPE){
        MagAudioPipeline *pAudioPipeline;

        pAudioPipeline = static_cast<MagAudioPipeline *>(pPipeline);
        return pAudioPipeline->putUsedFrame(pFrame);
    }else if (type == MAG_PIPELINE_SUBTITLE_TYPE){
        
    }

    return MAG_NO_ERROR;
}
