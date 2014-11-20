#include "MagESFormat.h"

/*convert the esds AAC to adts AAC*/
class MrvlAMPESFormat_AAC : public MagESFormat{
public:
	MrvlAMPESFormat_AAC(AVCodecContext* codec);
	virtual ~MrvlAMPESFormat_AAC();

	virtual ui32 formatES(ui8 *pHeader, ui8 *esPacket, ui32 esLength);
	virtual ui32 addESHeader(ui8 *pHeader);
    virtual void reset();
    
private:
	ui8  mpADTSHeaderBase[7];
	bool mFound;

	bool parseCodecExtraData(AVCodecContext* codec);
};