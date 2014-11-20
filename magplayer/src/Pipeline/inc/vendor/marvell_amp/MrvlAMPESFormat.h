#ifndef __MRVL_AMP_ES_FORMAT_H__
#define __MRVL_AMP_ES_FORMAT_H__

#include "framework/MagFramework.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavformat/avio.h"
#include "libavformat/internal.h"
#include "libavutil/avr32/intreadwrite.h"
}

class MrvlAMPESFormat{
public:
	MrvlAMPESFormat();
	virtual ~MrvlAMPESFormat();

	virtual ui32 formatES(ui8 *pHeader, ui8 *esPacket, ui32 esLength) = 0;
	virtual void reset() = 0;
};

static inline u16 U16ValueAt(const u8 *ptr){
	return ptr[0] << 8 | ptr[1];
}

static inline u32 U24ValueAt(const u8 *ptr){
	return ptr[0] << 16 | ptr[1] << 8 | ptr[2];
}

static inline u32 U32ValueAt(const u8 *ptr){
	return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

#endif