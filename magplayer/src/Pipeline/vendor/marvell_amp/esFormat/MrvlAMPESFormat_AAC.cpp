#include "MrvlAMPESFormat_AAC.h"

MrvlAMPESFormat_AAC::MrvlAMPESFormat_AAC(AVCodecContext* codec):
										mFound(false){
	if (parseCodecExtraData(codec)){
		mFound = true;
		AGILE_LOGD("AAC Codec formatter is found.");
	}
}

MrvlAMPESFormat_AAC::~MrvlAMPESFormat_AAC(){

}

void MrvlAMPESFormat_AAC::reset(){

}

bool MrvlAMPESFormat_AAC::parseCodecExtraData(AVCodecContext* codec){
	if (codec->extradata && codec->extradata_size > 0) {
	    ui32 smp_frq_idx = 0;
	    ui32 channel_cfg = 0;
	    ui32 profile_index = 0;
	    ui32 tmp = 0;

	    smp_frq_idx = ((codec->extradata[0] << 1) |
                       (codec->extradata[1] >> 7)) & 0xF;

	    channel_cfg = codec->extradata[1] >> 3;

	    profile_index = (codec->extradata[0] >> 3) - 1;

	    mpADTSHeaderBase[0] = 0xFF;
	    mpADTSHeaderBase[1] = 0xF1;
	    mpADTSHeaderBase[2] = ((profile_index << 6) |
                   (smp_frq_idx << 2) |
                   ((channel_cfg >> 2) & 0x1)) & 0xFF;
	    mpADTSHeaderBase[3] = (channel_cfg << 6) & 0xFF;
	    mpADTSHeaderBase[4] = 0;
	    mpADTSHeaderBase[5] = 0x1F;
	    mpADTSHeaderBase[6] = 0xFC;
	    return true;
	}else{
		AGILE_LOGE("Failed to parese! codec->extradata = 0x%p, codec->extradata_size = %d",
			        codec->extradata, codec->extradata_size);
	}
	return false;
}

ui32 MrvlAMPESFormat_AAC::formatES(ui8 *pHeader, ui8 *esPacket, ui32 esLength){
	if (!mFound) {
    	AGILE_LOGE("AVC Codec header has not been set!");
    	return 0;
  	}

  	if ((esLength > 2 && (ReadU16At(esPacket) & 0xfff0) != 0xfff0)){
		// AGILE_LOGV("it is ESDS AAC, need to convert it to ADTS AAC");
		memcpy(pHeader, mpADTSHeaderBase, 7);
		pHeader[3] |= esLength >> 11;
		pHeader[4]  = esLength >> 3 & 0xFF;
		pHeader[5] |= esLength << 5 & 0xFF;
		return 7;
	}
	return 0;
}

ui32 MrvlAMPESFormat_AAC::addESHeader(ui8 *pHeader){
	return 0;
}