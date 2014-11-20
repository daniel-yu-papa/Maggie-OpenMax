#include "MagESFormat.h"

class MrvlAMPESFormat_AVC : public MagESFormat{
public:
	MrvlAMPESFormat_AVC(AVCodecContext* codec);
	virtual ~MrvlAMPESFormat_AVC();

	virtual ui32 formatES(ui8 *pHeader, ui8 *esPacket, ui32 esLength);
	virtual ui32 addESHeader(ui8 *pHeader);
	virtual void reset();

private:
	ui8 mpCodecExtraData[1024]; /*the size of the header should be less than 1024*/
	ui32 mCodecExtraDataLen;
	ui32 mNALLengthSize;
	bool mFound;
	ui8  mFirstIDR;

	bool parseCodecExtraData(AVCodecContext* codec);
};