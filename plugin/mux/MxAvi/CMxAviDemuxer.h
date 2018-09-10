#ifndef __CMXAVIDEMUXER_H__
#define __CMXAVIDEMUXER_H__

#include "MxCodec.h"
#include "CMxDemuxer.h"

class CMxAviDemuxer: public CMxDemuxer
{
public:
	CMxAviDemuxer(MxSource* source, uint32 type);
	~CMxAviDemuxer();

protected:
	bool		m_hyperindexed;
	bool		m_biavs;
	bool		m_bamv;
	MainAVIHeader m_mainheader;

protected:
	bool		_parseFile(CMxList<CAVIStreamNode *>& streamlist);
	bool		_parseStreamHeader(CMxList<CAVIStreamNode *>& streams, DWORD dwLengthLeft);
	bool		_parseIndexBlock(CMxList<CAVIStreamNode *>& streams, int count, __int64);
	void		_parseExtendedIndexBlock(CMxList<CAVIStreamNode *>& streams, CAVIStreamNode *pasn, __int64 fpos, DWORD dwLength);
};

#endif