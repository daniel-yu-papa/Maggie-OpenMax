#include "MrvlAMPESFormat_AVC.h"

MrvlAMPESFormat_AVC::MrvlAMPESFormat_AVC(AVCodecContext* codec):
											mCodecExtraDataLen(0),
											mFound(false),
											mFirstIDR(0){
	if (parseCodecExtraData(codec)){
		mFound = true;
		mFirstIDR = 1;
        AGILE_LOGD("AVC Codec formatter is found.");
	}else{
        AGILE_LOGD("No need AVC Codec formatter.");
    }
}

MrvlAMPESFormat_AVC::~MrvlAMPESFormat_AVC(){

}

void MrvlAMPESFormat_AVC::reset(){
    mFirstIDR = 1;
}

bool MrvlAMPESFormat_AVC::parseCodecExtraData(AVCodecContext* codec) {
    ui16 unit_size;
    ui32 total_size = 0;
    ui8  unit_nb;
    ui8  sps_done = 0;
    ui8  sps_seen = 0;
    ui8  pps_seen = 0;
    const ui8 *extradata = NULL;
    static const ui8 nalu_header[4] = { 0, 0, 0, 1 };
    i32 length_size = 0; // retrieve length coded size
    FILE *fDump;

    AGILE_LOGD("the avc codec extradata=%p, extradata_siz=%d", codec->extradata, codec->extradata_size);

    if (codec->extradata != NULL && codec->extradata_size != 0){
        /*fDump = fopen("/data/lmp/extra.data", "wb+");
        fwrite(codec->extradata, 1, codec->extradata_size, fDump);
        fclose(fDump);*/
        AGILE_LOGD("codec->extradata[0]=0x%x, [1]=0x%x, [2]=0x%x, [3]=0x%x", 
                    codec->extradata[0],
                    codec->extradata[1],
                    codec->extradata[2],
                    codec->extradata[3]);

        if (codec->extradata[0] == 0x00 && 
            codec->extradata[1] == 0x00 &&
            codec->extradata[2] == 0x00 &&
            codec->extradata[3] == 0x01){
            AGILE_LOGD("extradata contains the complete SPS & PPS header, just copy them");
            mCodecExtraDataLen = codec->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE;
            mNALLengthSize = 4;

            memcpy(mpCodecExtraData, codec->extradata, codec->extradata_size);
            memset(mpCodecExtraData + codec->extradata_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
            return true;
        }else{
            if (codec->extradata[0] == 1){
                /*add the avc header*/
                extradata = codec->extradata + 4;
                length_size = (*extradata++ & 0x3) + 1;

                /* retrieve sps and pps unit(s) */
                unit_nb = *extradata++ & 0x1f; /* number of sps unit(s) */

                AGILE_LOGD("nal_length_size=%d, unit_nb = %d", length_size, unit_nb); 
                if (!unit_nb) {
                    goto pps;
                } else {
                    sps_seen = 1;
                }

                while (unit_nb--) {
                    int err;

                    unit_size   = ReadU16At(extradata);
                    total_size += unit_size + 4;
                    AGILE_LOGD("[%s]unit_size=%d, total_size = %d", pps_seen ? "pps" : sps_seen ? "sps" : "none",
                                unit_size, total_size);

                    if (total_size > INT_MAX) {
                        AGILE_LOGE("Too big extradata size, corrupted stream or invalid MP4/AVCC bitstream");
                        return false;
                    }
                    if (extradata + 2 + unit_size > codec->extradata + codec->extradata_size) {
                        AGILE_LOGE("Packet header is not contained in global extradata, corrupted stream or invalid MP4/AVCC bitstream");
                        return false;
                    }
                    memcpy(mpCodecExtraData + total_size - unit_size - 4, nalu_header, 4);
                    memcpy(mpCodecExtraData + total_size - unit_size, extradata + 2, unit_size);
                    extradata += 2 + unit_size;
            pps:
                    if (!unit_nb && !sps_done++) {
                        unit_nb = *extradata++; /* number of pps unit(s) */
                        if (unit_nb)
                            pps_seen = 1;
                    }
                }

                if (mpCodecExtraData)
                    memset(mpCodecExtraData + total_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

                if (!sps_seen)
                    AGILE_LOGW("Warning: SPS NALU missing or invalid, The resulting stream may not play.");

                if (!pps_seen)
                    AGILE_LOGW("Warning: PPS NALU missing or invalid, The resulting stream may not play.");

                mCodecExtraDataLen = total_size + FF_INPUT_BUFFER_PADDING_SIZE;
                if ((mCodecExtraDataLen < 6) || (mCodecExtraDataLen > 1024)){
                	AGILE_LOGE("The size of codec extra data is %d - Error! Nothing filtered!", 
                		        mCodecExtraDataLen);
                	return false;
                }

                mNALLengthSize = length_size;
                return true;
            }
        }
    }

    mCodecExtraDataLen = 0;
    mNALLengthSize = 0;

    return false;
}

ui32 MrvlAMPESFormat_AVC::formatES(ui8 *pHeader, ui8 *esPacket, ui32 esLength){
    i32  i;
    ui8  unit_type;
    i32  nal_size;
    ui32 cumul_size    = 0;
    const ui8 *esPacket_end = esPacket + esLength;
    ui32 header_size = mCodecExtraDataLen;

    if (!mFound) {
    	AGILE_LOGE("AVC Codec header has not been set!");
    	return 0;
  	}
  	// AGILE_LOGV("enter!");

    if (esPacket + mNALLengthSize > esPacket_end){
    	AGILE_LOGE("Error: mNALLengthSize[%d] > esLength[%d]", mNALLengthSize, esLength);
        goto fail;
    }

    unit_type = *(esPacket + mNALLengthSize) & 0x1f;

#if 0
    /* prepend only to the first type 5 NAL unit of an IDR picture */
    if (mFirstIDR && (unit_type == 5 || unit_type == 7 || unit_type == 8)) {
    	// ui8 *p = mpCodecExtraData + mCodecExtraDataLen;
    	// WriteU32At(p, 1);
    	// header_size += 4;
        mFirstIDR = 0;
    } else {
        header_size = 0;
        if (!mFirstIDR && unit_type == 1)
            mFirstIDR = 1;
    }
#else
    if (mFirstIDR) {
    	// ui8 *p = mpCodecExtraData + mCodecExtraDataLen;
    	// WriteU32At(p, 1);
    	// header_size += 4;
        mFirstIDR = 0;
    } else {
        header_size = 0;
    }
#endif
    if (header_size > 0)
    	memcpy(pHeader, mpCodecExtraData, header_size);

    {
    	ui8 *rp = esPacket;

    	while (rp + 4 <= esPacket_end){
    		for (nal_size = 0, i = 0; i < mNALLengthSize; i++)
	    		nal_size = (nal_size << 8) | rp[i];

	    	AGILE_LOGV("nal_size = %d, esLength = %d", nal_size, esLength);

    		WriteU32At(rp, 1);
    		if (nal_size > 0)
    			rp += (nal_size + 4);
    		else
    			break;
    	}
    }

    AGILE_LOGV("header size = %d, unit_type = %d", header_size, unit_type);
    return header_size;

fail:
    return 0;
    
}

ui32 MrvlAMPESFormat_AVC::addESHeader(ui8 *pHeader){
    ui32 len = 0;

    if (!mFound) {
        AGILE_LOGE("AVC Codec header has not been set!");
        return 0;
    }

    if (mFirstIDR) {
        mFirstIDR = 0;
        memcpy(pHeader, mpCodecExtraData, mCodecExtraDataLen);
        len = mCodecExtraDataLen;
    }
    return len;
}