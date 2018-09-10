#include "stdafx.h"
#include "CMxAviDemuxer.h"
#include "MxAvi.h"

mxinline LONG vx_gcd(LONG a, LONG b) {
	if (b) return vx_gcd(b, a%b);
	else  return a;
}

int _get_mpegvinfo(PBYTE ptr, int len, MxVideoInfo aviinfo)
{
	double db = mpeg2_picture_rate_prob(ptr, ptr + len);
	int ret = -1;
	mpeg2dec_t *dec = mpeg2_init();
	mpeg2_buffer(dec, ptr, ptr + len);
	while (true)
	{
		mpeg2_state_t state = mpeg2_parse(dec);
		const mpeg2_info_t *info = mpeg2_info(dec);
		if ((state == STATE_BUFFER) || (state == STATE_INVALID))
		{
			break;
		}
		else if (state == STATE_SEQUENCE)
		{
			aviinfo->fourcc = (info->sequence->flags & SEQ_FLAG_MPEG2) ? vxFormat_MPEG2 : vxFormat_MPEG1;
			aviinfo->scale = info->sequence->frame_period.scale;
			aviinfo->rate = info->sequence->frame_period.rate * db;

			aviinfo->display_width = info->sequence->display_width;
			aviinfo->display_height = info->sequence->display_height;
			aviinfo->picture_width = info->sequence->picture_width;
			aviinfo->picture_height = info->sequence->picture_height;
			if (aviinfo->display_height > aviinfo->picture_height)
			{
				aviinfo->display_height = aviinfo->picture_height;
			}
			if ((info->sequence->display_height == 160) && (info->sequence->picture_height == 1088))
			{
				aviinfo->display_height = 1080;
			}

			if ((info->sequence->profile_level_id >> 7) & 1)
			{
				// 4:2:2 Profile @ Main Level
				if ((info->sequence->profile_level_id & 15) == 5)
				{
					aviinfo->profile = (128 + 5);
					aviinfo->level = 8;
				}
			}
			else
			{
				aviinfo->profile = info->sequence->profile_level_id >> 4;
				aviinfo->level = info->sequence->profile_level_id & 0xF;
			}
			aviinfo->chromafmt = (info->sequence->chroma_height != info->sequence->height) ? 1 : 2;
			int aspectw = info->sequence->pixel_width * info->sequence->picture_width;
			int aspecth = info->sequence->pixel_height * info->sequence->picture_height;
			int aspectgcd = vx_gcd(aspectw, aspecth);
			int aspectx = aspectw / aspectgcd;
			int aspecty = aspecth / aspectgcd;
			if ((aspectx > 0x0FFF) || (aspecty > 0x0FFF))
			{
				if (aspectx > aspecty)
				{
					aspecty = 1.0 * aspecty / aspectx * 0x0FFF;
					aspectx = 0x0FFF;
				}
				else if (aspectx < aspecty)
				{
					aspectx = 1.0 * aspectx / aspecty * 0x0FFF;
					aspecty = 0x0FFF;
				}
				else
				{
					aspectx = 0x0FFF;
					aspecty = 0x0FFF;
				}
			}
			if (!aviinfo->reserved2[3])
			{
				aviinfo->aspect = (aspecty << 16) | (aspectx);
				//aviinfo->aspect = ((aspecth / aspectgcd) << 16) | (aspectw / aspectgcd);
			}
			aviinfo->suggestedsize = aviinfo->picture_width * aviinfo->display_height * 2;
			aviinfo->max_bitrate = info->sequence->byte_rate * 8;
			if (info->sequence->flags & SEQ_FLAG_LOW_DELAY)
			{
				aviinfo->dectype = 1;
			}
			if (aviinfo->fourcc == vxFormat_MPEG1 && info->sequence->byte_rate == 0x3FFFF)
			{
				aviinfo->vbr = 1;
			}
		}
		else if (state == STATE_PICTURE)
		{
			aviinfo->topfirst = 0;
			if (info->current_picture->flags & PIC_FLAG_TOP_FIELD_FIRST)
			{
				aviinfo->topfirst = 1;
			}
			if (info->current_picture->flags & PIC_FLAG_PROGRESSIVE_FRAME)
			{
				aviinfo->topfirst = 2;
			}
			if (aviinfo->fourcc == MxFormatType_mpeg2 && info->current_picture->vbv_delay == 0xFFFF)
			{
				aviinfo->vbr = 1;
			}
			ret = 0;
			break;
		}
	}
	mpeg2_close(dec);
	if ((aviinfo->picture_height == 608) || (aviinfo->picture_height == 512))
	{
		aviinfo->display_yoffset = 32;
		aviinfo->display_height = aviinfo->picture_height - 32;
	}
	return ret;
}

CMxAviDemuxer::CMxAviDemuxer(MxSource* source, uint32 type)
	: CMxDemuxer(source, type)
	, m_hyperindexed(FALSE)
	, m_biavs(FALSE)
	, m_bamv(FALSE)
{
}


CMxAviDemuxer::~CMxAviDemuxer()
{
}

bool CMxAviDemuxer::_parseFile(CMxList<CAVIStreamNode *>& streamlist) {
	FOURCC fccType;
	DWORD dwLength;
	DWORD dwLengthLeft;
	bool index_found = FALSE;
	bool size_invalid = FALSE;
	bool fAcceptIndexOnly = TRUE;
	CAVIStreamNode *pasn;

	__int64	i64ChunkMoviPos = 0;
	DWORD	dwChunkMoviLength = 0;

	if (!_readChunkHeader(fccType, dwLength))
		return FALSE;

	if (fccType != FOURCC_RIFF)
		return FALSE;

	if (dwLength < 4)
	{
		dwLength = 0xFFFFFFF0;
		size_invalid = TRUE;
	}

	dwLengthLeft = dwLength - 4;

	_readFile(&fccType, 4);

	if ((fccType != ' IVA') && (fccType != ' VMA'))
		return FALSE;
	m_bamv = fccType == ' VMA';

	// begin parsing chunks

	while (dwLengthLeft >= 8 && _readChunkHeader(fccType, dwLength))
	{
		if (!size_invalid)
		{
			dwLengthLeft -= 8;

			if (dwLength > dwLengthLeft)
				return FALSE;

			dwLengthLeft -= (dwLength + (dwLength & 1));
		}

		switch (fccType)
		{
		case FOURCC_LIST:
			_readFile(&fccType, 4);

			if (!m_bamv&&dwLength<4 && (fccType != 'ivom' || !size_invalid))
				return FALSE;
			dwLength -= 4;

			switch (fccType)
			{
			case 'ivom':

				if (dwLength < 8 && size_invalid)
				{
					i64ChunkMoviPos = _posFile();
					dwChunkMoviLength = 0xFFFFFFF0;
					dwLength = 0;
				}
				else
				{
					i64ChunkMoviPos = _posFile();
					dwChunkMoviLength = dwLength;
				}

				if (fAcceptIndexOnly)
					goto terminate_scan;
				break;
			case 'lrdh'://listtypeAVIHEADER:
				dwLengthLeft += (dwLength + (dwLength & 1)) + 4;
				dwLength = 0;	// silently enter the header block
				break;
			case 'lrts'://listtypeSTREAMHEADER:
				if (!_parseStreamHeader(streamlist, dwLength))
					fAcceptIndexOnly = FALSE;
				else
					m_hyperindexed = TRUE;

				++m_streams;
				dwLength = 0;
				break;
			}

			break;

		case '1xdi'://ckidAVINEWINDEX:	// idx1
			if (!m_hyperindexed)
			{
				index_found = _parseIndexBlock(streamlist, dwLength / 16, i64ChunkMoviPos);
				dwLength &= 15;
			}
			break;
		case 'hvma':
		case 'hiva'://ckidAVIMAINHDR:
			if (dwLength >= sizeof(MainAVIHeader))
			{
				_readFile(&m_mainheader, sizeof(MainAVIHeader));
				dwLength = dwLength - sizeof(MainAVIHeader);
			}
			break;
		}
		if (dwLength)
		{
			if (!_skipFile(dwLength + (dwLength & 1)))
				break;
		}
		if (size_invalid && fccType == '1xdi'/*ckidAVINEWINDEX*/)
			break;
	}

	if (i64ChunkMoviPos == 0) return FALSE;

terminate_scan:
	if (!index_found && !m_hyperindexed) return FALSE;

	MXPOSITION pos = streamlist.GetHeadPosition();
	while (pos)
	{
		MXPOSITION prevpos = pos;
		pasn = streamlist.GetNext(pos);
		if (!pasn->index.makeIndex2())
			return FALSE;
		pasn->frames = pasn->index.indexLen();
		if (pasn->fccType == MxStreamType_audio&&pasn->ainfo.blockalign)
		{
			switch (pasn->ainfo.fourcc)
			{
			case 0x55:
				pasn->ainfo.fourcc = MxFormatType_mp3;
				break;
			}
			if (pasn->ainfo.framesize)
			{
				pasn->length = pasn->frames;
				pasn->ainfo.samples = pasn->frames*pasn->ainfo.samplesperframe;
				pasn->ainfo.framesize = 0;
			}
			else
			{
				pasn->length = (LONG)pasn->bytes / pasn->ainfo.blockalign;
				pasn->ainfo.framesize = pasn->ainfo.blockalign;

				//DV录制的AVI视频中可能出现8bitPCM音频数据，不能仅仅通过字节判断数据类型
				if ((pasn->ainfo.blockalign == 1) && (pasn->ainfo.fourcc != MxFormatType_pcm))
				{
					pasn->ainfo.dectype = 2;
					pasn->ainfo.samples = pasn->length;
				}
				else
				{
					pasn->ainfo.samples = pasn->bytes * 8 * pasn->ainfo.freq / pasn->ainfo.bitrate;
					pasn->ainfo.framesize = pasn->ainfo.blockalign;
					pasn->ainfo.samplesperframe = pasn->ainfo.blockalign * 8 * pasn->ainfo.freq / pasn->ainfo.bitrate;
					pasn->ainfo.dectype = 1;
					if (pasn->ainfo.samplesperframe == 1)
						pasn->ainfo.dectype = 0;
				}
				if (pasn->ainfo.fourcc == 0x2000 && pasn->ainfo.blockalign != 1)
				{
					if (pasn->ainfo.bitpersample == 0)
					{
						pasn->ainfo.bitpersample = 16;
					}
					pasn->ainfo.samples = pasn->bytes * 8 * pasn->ainfo.freq / pasn->ainfo.bitrate;
					pasn->ainfo.blockalign = pasn->ainfo.channels * ((pasn->ainfo.bitpersample + 7) >> 3);
					pasn->ainfo.framesize = pasn->bytes / pasn->frames;
					pasn->ainfo.samplesperframe = pasn->ainfo.samples / pasn->frames;
					pasn->length = pasn->frames;
				}
			}
		}
		else
			pasn->length = pasn->frames;

		if (pasn->length == 0)
		{
			streamlist.RemoveAt(prevpos);
			delete pasn;
		}

		if (pasn->fccType == MxStreamType_video)
		{
			if (pasn->vinfo.fourcc == '462X')
			{
				pasn->vinfo.fourcc = MxFormatType_h264;
			}
			pasn->vinfo.frames = pasn->frames;
			if ((pasn->vinfo.fourcc == 'semm') || (pasn->vinfo.fourcc == 'SEMM')
				|| (pasn->vinfo.fourcc == '107m') || (pasn->vinfo.fourcc == '107M')
				|| (pasn->vinfo.fourcc == '307m') || (pasn->vinfo.fourcc == '307M'))
			{
				AVIIndexEntry2& avie = pasn->index.index2Ptr()[0];
				CMemArrayPtr<BYTE> tmp = new BYTE[avie.size & 0x1FFFFFFF];
				_seekFile(avie.pos + 8);
				LONG lread = _readFile(tmp, avie.size & 0x1FFFFFFF);
				_get_mpegvinfo(tmp, lread, &pasn->vinfo);
			}
			else if (pasn->vinfo.fourcc == MxFormatType_h264)
			{
				AVIIndexEntry2& avie = pasn->index.index2Ptr()[0];
				CMemArrayPtr<BYTE> tmp = new BYTE[avie.size & 0x1FFFFFFF];
				_seekFile(avie.pos + 8);
				LONG lread = _readFile(tmp, avie.size & 0x1FFFFFFF);
				int rate = pasn->vinfo.rate, scale = pasn->vinfo.scale;
				//_get_h264info(tmp,lread,&pasn->vinfo);
				vxGetVideoInfo((BYTE *)tmp, lread, &pasn->vinfo);
				pasn->vinfo.rate = rate; pasn->vinfo.scale = scale;
			}
			else if ((pasn->vinfo.fourcc == 'divx') ||
				(pasn->vinfo.fourcc == 'xvid') ||
				(pasn->vinfo.fourcc == 'XVID') ||
				(pasn->vinfo.fourcc == 'DIVX'))
			{
				pasn->vinfo.fourcc = MxFormatType_mpeg4;
				AVIIndexEntry2& avie = pasn->index.index2Ptr()[0];
				CMemArrayPtr<BYTE> tmp = new BYTE[avie.size & 0x1FFFFFFF];
				_seekFile(avie.pos + 8);
				LONG lread = _readFile(tmp, avie.size & 0x1FFFFFFF);
				int rate = pasn->vinfo.rate, scale = pasn->vinfo.scale;
				vxGetVideoInfo((BYTE *)tmp, lread, &pasn->vinfo);
				pasn->vinfo.rate = rate; pasn->vinfo.scale = scale;
			}
			/*
			else if((pasn->vinfo.fourcc=='semm')||(pasn->vinfo.fourcc=='SEMM')
			||(pasn->vinfo.fourcc=='semm')||(pasn->vinfo.fourcc=='SEMM')
			||(pasn->vinfo.fourcc=='semm')||(pasn->vinfo.fourcc=='SEMM'))
			{

			}
			*/
			else
			{
				pasn->vinfo.dectype = 2;
				if (pasn->keyonly)
					pasn->vinfo.dectype = 1;
				DWORD fcc = pasn->vinfo.fourcc;
				type2low((PBYTE)&fcc);
				switch (fcc)
				{
				case vxFormat_dvsd:
					pasn->vinfo.chromafmt = 1; //420
					pasn->vinfo.bitrate *= (1.0 * 134 / 150 * 76 / 80);
					break;
				case vxFormat_dv25:
					pasn->vinfo.chromafmt = 4; //411
					pasn->vinfo.bitrate *= (1.0 * 134 / 150 * 76 / 80);
					break;
				case vxFormat_dv50:
					pasn->vinfo.chromafmt = 2;//422
					pasn->vinfo.bitrate *= (1.0 * 134 / 150 * 76 / 80);
					break;
				case vxFormat_dvh1:
					pasn->vinfo.chromafmt = 2;//422
					pasn->vinfo.aspect = 0x90010;
					pasn->vinfo.bitrate *= (1.0 * 134 / 150 * 76 / 80);
					break;
				case 'cvdc':
					pasn->vinfo.chromafmt = 1; //420
					pasn->vinfo.bitrate *= (1.0 * 134 / 150 * 76 / 80);
					break;
				case vxFormat_AVC_I:
				case vxFormat_apcn:
				case vxFormat_apch:
				case vxFormat_DNxHD:
					pasn->vinfo.aspect = 0x90010;
					break;
				case vxFormat_bid:
					pasn->vinfo.fourcc = fcc;
					if ((pasn->vinfo.picture_width == 720) &&
						((pasn->vinfo.picture_height == 486) || (pasn->vinfo.picture_height == 480) || (pasn->vinfo.picture_height == 576)))
					{
						pasn->vinfo.aspect = 0x30004;
					}
					else
					{
						//pasn->vinfo.aspect = 0x90010;
						int aspectx = pasn->vinfo.picture_width;
						int aspecty = pasn->vinfo.picture_height;
						LONG agcd = vx_gcd(aspectx, aspecty);
						if (agcd)
						{
							pasn->vinfo.aspect = ((aspecty / agcd) << 16) | (aspectx / agcd);
						}
					}
					break;
				case vxFormat_DivX5:
					pasn->vinfo.fourcc = fcc;
					break;
				case vxFormat_mjpg:
					pasn->vinfo.fourcc = vxFormat_mjpg;
					pasn->vinfo.chromafmt = 2;
					break;
				case '4pmf':
				case '4PMF':
					pasn->vinfo.fourcc = vxFormat_MPEG4;
					pasn->vinfo.aspect = 0x90010;
					break;
				case vxFormat_ap208:
					pasn->vinfo.depth = 8;
					pasn->vinfo.chromafmt = 2;
					break;
				case vxFormat_ap210:
					pasn->vinfo.depth = 10;
					pasn->vinfo.chromafmt = 2;
					break;
				case 'xvid':
					pasn->vinfo.chromafmt = 1;
					break;
				case vxFormat_yuy2:
					pasn->vinfo.chromafmt = 2;
					break;
				case 'vuyi':
					pasn->vinfo.chromafmt = 1;
					break;
				}
			}

			if (pasn->vinfo.chromafmt < 1)
			{
				pasn->vinfo.chromafmt = 1;
			}

			if (pasn->vinfo.depth <= 0)
			{
				pasn->vinfo.depth = 8;
			}

			if (pasn->vinfo.bitrate <= 0)
			{
				__int64 vStreamLength = 0;
				__int64 vbitrate = 0;

				for (int i = 0; i < pasn->frames; ++i)
				{
					AVIIndexEntry2 &avie = pasn->index.index2Ptr()[i];
					vStreamLength += avie.size & 0x1FFFFFFF;
				}
				if (vStreamLength > 0)
				{
					vbitrate = vStreamLength * 8 / (pasn->frames * pasn->vinfo.scale * 1.0f / pasn->vinfo.rate);
				}
				if (vbitrate > pasn->vinfo.bitrate)
				{
					pasn->vinfo.bitrate = vbitrate;
				}
			}
			if (!pasn->vinfo.aspect && pasn->vinfo.picture_height && pasn->vinfo.picture_width)
			{
				LONG agcd = vx_gcd(pasn->vinfo.picture_width, pasn->vinfo.picture_height);
				pasn->vinfo.aspect = ((pasn->vinfo.picture_height / agcd) << 16) | (pasn->vinfo.picture_width / agcd);
			}
		}
	}

	if (m_biavs && (pasn = streamlist.GetHead()))
	{
		pasn->fccType = 'sdiv';//streamtypeVIDEO;
		DVINFO* info = (DVINFO*)pasn->vinfo.pExtraData;
		pasn->vinfo.aspect = ((info->dwDVVAuxCtl & 0x300) == 0x200) ? 0x00090010 : 0x00030004;
		if (pasn->vinfo.picture_width == 0)
		{
			pasn->vinfo.picture_width = pasn->vinfo.display_width = 720;
			pasn->vinfo.picture_height = pasn->vinfo.display_height = ((pasn->vinfo.rate / pasn->vinfo.scale) == 25) ? 576 : 480;
		}


		int freq = 48000;
		if ((info->dwDVAAuxSrc & 0x38000000) == 0x10000000) freq = 32000;

		AVIIndexEntry2* index = pasn->index.index2Ptr();

		CAVIStreamNode* apasn = new CAVIStreamNode(0);
		apasn->fccType = 'sdua';//streamtypeAUDIO;
		apasn->fmtfcc = vxFormat_DPCM;
		apasn->ainfo.samplesperframe = (__int64)freq*pasn->vinfo.scale / pasn->vinfo.rate;
		apasn->ainfo.freq = freq;
		apasn->ainfo.channels = 2;
		apasn->ainfo.bitpersample = 16;
		apasn->ainfo.framesize = index[0].size & 0x1FFFFFFF;
		apasn->ainfo.samples = apasn->ainfo.samplesperframe*pasn->frames;
		apasn->ainfo.dectype = 1;
		apasn->ainfo.suggestedsize = pasn->vinfo.suggestedsize;
		apasn->vinfo.bitrate = apasn->ainfo.bitrate = (__int64)apasn->ainfo.framesize * 8 * pasn->vinfo.rate / pasn->vinfo.scale;

		for (int i = 0; i<pasn->frames; i++)
			apasn->index.add('bw10', index[i].pos, index[0].size, i, TRUE, 0);
		if (!apasn->index.makeIndex2())
			return FALSE;
		apasn->frames = pasn->index.indexLen();
		apasn->length = apasn->frames;
		streamlist.AddTail(apasn);
	}
	return TRUE;
}
bool CMxAviDemuxer::_parseStreamHeader(CMxList<CAVIStreamNode *>& streams, DWORD dwLengthLeft) {
	CAVIStreamNode *pasn;
	FOURCC fccType;
	DWORD dwLength;
	AVIStreamHeader_fixed	hdr = { 0 };

	if (!(pasn = new CAVIStreamNode(0))) return FALSE;

	//	try 
	{
		while (dwLengthLeft >= 8 && _readChunkHeader(fccType, dwLength))
		{

			dwLengthLeft -= 8;

			if (dwLength > dwLengthLeft) return FALSE;

			dwLengthLeft -= (dwLength + (dwLength & 1));

			switch (fccType) {

			case 'hrts'://ckidSTREAMHEADER:
			{
				if (dwLength < sizeof(hdr))
				{
					_readFile(&hdr, dwLength);
					if (dwLength & 1) _skipFile(1);
				}
				else
				{
					_readFile(&hdr, sizeof(hdr));
					_skipFile(dwLength + (dwLength & 1) - sizeof(hdr));
				}
				dwLength = 0;
				if (m_bamv)
					hdr.fccType = streamlist.GetCount() ? vxstreamAUDIO : vxstreamVIDEO;
				pasn->fccType = hdr.fccType;
				if ((hdr.fccType == vxstreamVIDEO) || (hdr.fccType == 'svai'))
				{
					pasn->fccType = vxstreamVIDEO;
					m_biavs = (hdr.fccType == 'svai');
					pasn->vinfo.rate = hdr.dwRate;
					pasn->vinfo.scale = hdr.dwScale;
					pasn->vinfo.display_width = hdr.rcFrame.right;
					pasn->vinfo.display_height = hdr.rcFrame.bottom;
					pasn->vinfo.suggestedsize = hdr.dwSuggestedBufferSize;
					if (hdr.dwSampleSize>0)
						pasn->vinfo.bitrate = (__int64)hdr.dwSampleSize * 8 * hdr.dwRate / hdr.dwScale;
				}
				pasn->fmtfcc = hdr.fccHandler;
			}
			break;

			case 'frts'://ckidSTREAMFORMAT:
			{
				PBYTE fmt = (PBYTE)_vxmalloc(dwLength);
				_readFile(fmt, dwLength);
				if (pasn->fccType == vxstreamVIDEO)
				{
					if (m_biavs)
					{
						pasn->vinfo.picture_width = pasn->vinfo.display_width;
						pasn->vinfo.picture_height = pasn->vinfo.display_height;
						pasn->vinfo.dwExtraSize = dwLength;
						pasn->vinfo.pExtraData = (LPBYTE)_vxmalloc(dwLength);
						memcpy(pasn->vinfo.pExtraData, fmt, dwLength);
					}
					else if (m_bamv)
					{
						pasn->vinfo.picture_width = pasn->vinfo.display_width = m_mainheader.dwWidth;
						pasn->vinfo.picture_height = pasn->vinfo.display_height = m_mainheader.dwHeight;
						pasn->fmtfcc = 'VVMA';
					}
					else if (dwLength >= sizeof(BITMAPINFOHEADER))
					{
						pasn->vinfo.dectype = 1;
						switch (((BITMAPINFOHEADER *)fmt)->biCompression)
						{
						case '362h':
						case '462h':
						case '462H':
						case '462x':
						case 'mp4v':
							pasn->vinfo.dectype = 2;
							break;
						case 'v2pm':
							pasn->vinfo.dectype = 0;
							break;
						}
						if (pasn->fmtfcc == 0)
						{
							pasn->fmtfcc = ((BITMAPINFOHEADER *)fmt)->biCompression;
						}
						if (pasn->fmtfcc == '462H')
						{
							pasn->fmtfcc = '462h';
						}
						pasn->vinfo.picture_width = ((BITMAPINFOHEADER *)fmt)->biWidth;
						pasn->vinfo.picture_height = ((BITMAPINFOHEADER *)fmt)->biHeight;
						pasn->vinfo.depth = ((BITMAPINFOHEADER *)fmt)->biBitCount;

						if ((pasn->fmtfcc == ' bid' || pasn->fmtfcc == ' BID' || pasn->fmtfcc == 0x00000000)
							&& (((BITMAPINFOHEADER *)fmt)->biCompression == 0x00000000)) //RGB
						{
							if (pasn->fmtfcc == 0x00000000)
							{
								pasn->fmtfcc = 'ABGR';
							}
							if (pasn->vinfo.depth == 32)
							{

								pasn->vinfo.reserved2[0] = 'RGBA';
								pasn->vinfo.reserved2[1] = pasn->vinfo.depth;
								pasn->vinfo.depth /= 4;
							}
							else
							{
								pasn->vinfo.reserved2[0] = 'RGB ';
								pasn->vinfo.reserved2[1] = pasn->vinfo.depth;
								pasn->vinfo.depth = pasn->vinfo.depth <= 16 ? 8 : pasn->vinfo.depth / 3;
							}
						}
						else if (pasn->fmtfcc == 'ICVA')
						{
							pasn->fmtfcc = vxFormat_AVC_I;
						}
						if (pasn->vinfo.depth == 24)
						{
							pasn->vinfo.depth = 8;
						}
						pasn->vinfo.topfirst = 1;
						pasn->vinfo.chromafmt = 1;
						if (pasn->vinfo.display_width == 0) pasn->vinfo.display_width = pasn->vinfo.picture_width;
						if (pasn->vinfo.display_height == 0) pasn->vinfo.display_height = pasn->vinfo.picture_height;
						if (dwLength>sizeof(BITMAPINFOHEADER))
						{
							if ((pasn->fmtfcc == 'semm') || (pasn->fmtfcc == 'SEMM')
								|| (pasn->fmtfcc == '107m') || (pasn->fmtfcc == '107M')
								|| (pasn->fmtfcc == '307m') || (pasn->fmtfcc == '307M'))
							{
								LPMPEG2AVI_FORMAT mpeg2fmt = (LPMPEG2AVI_FORMAT)fmt;
								if (mpeg2fmt->mt.bTemporalCompression) pasn->vinfo.dectype = 0;
								pasn->vinfo.profile = mpeg2fmt->info.dwProfile;
								pasn->vinfo.level = mpeg2fmt->info.dwLevel;
								pasn->vinfo.chromafmt = (pasn->vinfo.profile == 6) ? 2 : 1;
								pasn->vinfo.aspect = ((pasn->vinfo.display_width == 1440) || (pasn->vinfo.display_width == 1920)) ? 0x00090010 : 0x00030004;
								if ((pasn->vinfo.display_height == 608) || (pasn->vinfo.display_height == 512))
								{
									pasn->vinfo.display_yoffset = 32;
									pasn->vinfo.display_height -= 32;
								}
							}
							if (pasn->fmtfcc != vxFormat_dvh1)
							{
								int size = pasn->vinfo.dwExtraSize = dwLength - sizeof(BITMAPINFOHEADER);
								pasn->vinfo.pExtraData = (LPBYTE)_vxmalloc(size);
								memcpy(pasn->vinfo.pExtraData, (PBYTE)fmt + sizeof(BITMAPINFOHEADER), size);
							}
						}
					}
				}
				else if (pasn->fccType == vxstreamAUDIO)
				{
					WAVEFORMATEX* wfx = (WAVEFORMATEX*)fmt;
					if (wfx->wFormatTag == 0x55)//MP3
					{
						MPEGLAYER3WAVEFORMAT* mp3 = (MPEGLAYER3WAVEFORMAT*)fmt;
						pasn->ainfo.fourcc = wfx->wFormatTag;
						pasn->ainfo.channels = wfx->nChannels;
						pasn->ainfo.freq = wfx->nSamplesPerSec;
						pasn->ainfo.bitpersample = wfx->wBitsPerSample;
						pasn->ainfo.blockalign = wfx->nBlockAlign;
						pasn->ainfo.framesize = mp3->nBlockSize;
						if (pasn->ainfo.blockalign == 1)
						{
							pasn->ainfo.samplesperframe = 1.0 * wfx->nSamplesPerSec / wfx->nAvgBytesPerSec * mp3->nBlockSize;
						}
						else
						{
							pasn->ainfo.samplesperframe = 1152;//wfx->nBlockAlign*mp3->nFramesPerBlock;
						}
						pasn->ainfo.bitrate = wfx->nAvgBytesPerSec * 8;
						if (pasn->ainfo.bitpersample == 0) pasn->ainfo.bitpersample = 16;
					}
					else if (wfx->wFormatTag == 0x706d)
					{
						pasn->ainfo.fourcc = vxFormat_AAC;
						pasn->ainfo.channels = wfx->nChannels;
						pasn->ainfo.freq = wfx->nSamplesPerSec;
						pasn->ainfo.bitpersample = wfx->wBitsPerSample;
						pasn->ainfo.blockalign = wfx->nBlockAlign;
						pasn->ainfo.samplesperframe = hdr.dwScale;
						pasn->ainfo.framesize = wfx->nBlockAlign;
						pasn->ainfo.bitrate = wfx->nAvgBytesPerSec * 8;
						if (pasn->ainfo.bitpersample == 0) pasn->ainfo.bitpersample = 16;
					}
					else
					{
						pasn->ainfo.fourcc = wfx->wFormatTag;
						pasn->ainfo.channels = wfx->nChannels;
						pasn->ainfo.freq = wfx->nSamplesPerSec;
						pasn->ainfo.bitpersample = wfx->wBitsPerSample;
						pasn->ainfo.blockalign = wfx->nBlockAlign;
						pasn->ainfo.bitrate = wfx->nAvgBytesPerSec * 8;
						if ((dwLength>sizeof(WAVEFORMATEX)) && (wfx->cbSize>0))
						{
							pasn->ainfo.dwExtraSize = wfx->cbSize;
							pasn->ainfo.pExtraData = (LPBYTE)_vxmalloc(wfx->cbSize);
							memcpy(pasn->ainfo.pExtraData, wfx + 1, wfx->cbSize);
						}
					}
					pasn->ainfo.dectype = 1;
					if (pasn->ainfo.fourcc == 1) pasn->ainfo.fourcc = vxFormat_PCM;

				}
				else if (pasn->fccType == 'svai')
				{
					m_biavs = TRUE;
				}

				_vxfree(fmt);
				if (dwLength & 1)
					_skipFile(1);
				dwLength = 0;
			}
			break;

			case 'xdni':			// OpenDML extended index
				_parseExtendedIndexBlock(streamlist, pasn, -1, dwLength);
				m_hyperindexed = TRUE;
				break;
			case 'KNUJ'://ckidAVIPADDING:	// JUNK
				break;
			case 'TSIL':
				_seekFile(_posFile() - 8);
				dwLengthLeft = 0;
				dwLength = 0;
				break;
			case 'prpv':
				if (dwLength < 24)
				{
					break;
				}
				PBYTE PRPV = (PBYTE)_vxmalloc(dwLength);
				int ret = _readFile(PRPV, dwLength);
				WORD FrameAspectRatio_H = 0, FrameAspectRatio_W = 0;
				FrameAspectRatio_H = (PRPV[21] << 8) | PRPV[20];
				FrameAspectRatio_W = (PRPV[23] << 8) | PRPV[22];
				if (pasn->fccType == vxstreamVIDEO && FrameAspectRatio_W && FrameAspectRatio_H)
				{
					pasn->vinfo.aspect = (FrameAspectRatio_H << 16) | FrameAspectRatio_W;
					pasn->vinfo.reserved2[3] = 1;
				}
				_vxfree(PRPV);
				if (dwLength & 1)
					_skipFile(1);
				dwLength = 0;
				break;
			}

			if (dwLength) {
				if (!_skipFile(dwLength + (dwLength & 1)))
					break;
			}
		}

		if (dwLengthLeft)
			_skipFile(dwLengthLeft);
		streamlist.AddTail(pasn);
	}
	/*
	catch(...)
	{
	delete pasn;
	}
	*/
	return m_hyperindexed;
}
bool CMxAviDemuxer::_parseIndexBlock(CMxList<CAVIStreamNode *>& streams, int count, __int64 movi_offset) {
	AVIIndexEntry avie[32];
	CAVIStreamNode *pasn;//, *pasn_next;
	bool absolute_addr = TRUE;

	// Some AVI files have relative addresses in their AVI index chunks, and some
	// relative.  They're supposed to be relative to the 'movi' chunk; all versions
	// of VirtualDub using fast write routines prior to build 4936 generate absolute
	// addresses (oops). AVIFile and ActiveMovie are both ambivalent.  I guess we'd
	// better be as well.

	while (count > 0) {
		int tc = count;
		int i;

		if (tc>32) tc = 32;
		count -= tc;

		if (tc * sizeof(AVIIndexEntry) != _readFile(avie, tc * sizeof(AVIIndexEntry)))
		{
			MXPOSITION pos = streamlist.GetHeadPosition();
			while (pos)
			{
				pasn = streamlist.GetNext(pos);
				pasn->index.clear();
				pasn->bytes = 0;
			}
			return FALSE;
		}

		for (i = 0; i<tc; i++)
		{
			int stream = StreamFromFOURCC(avie[i].ckid);

			if (absolute_addr && avie[i].dwChunkOffset<movi_offset)
				absolute_addr = FALSE;

			MXPOSITION pos = streamlist.FindIndex(stream);
			pasn = NULL;
			if (pos)
				pasn = streamlist.GetNext(pos);
			if (pasn && (avie[i].dwChunkLength > 0))
			{
				if ((pasn->ainfo.fourcc == 0x55) && (pasn->ainfo.framesize > 0) && (pasn->ainfo.blockalign == 1))
				{
					int vCurCount = avie[i].dwChunkLength / pasn->ainfo.framesize;
					if (vCurCount > 1)
					{
						for (int j = 0; j < vCurCount; j++)
						{
							bool bKey = !!(avie[i].dwFlags & AVIIF_KEYFRAME);
							/*if (absolute_addr)
							pasn->index.add(&avie[i]);
							else*/
							pasn->index.add(avie[i].ckid, (movi_offset - 4) + (__int64)avie[i].dwChunkOffset + j * pasn->ainfo.framesize, pasn->ainfo.framesize, -1, bKey);

							pasn->bytes += pasn->ainfo.framesize;

							pasn->keyonly = pasn->keyonly && bKey;
						}
					}
					else
					{

						bool bKey = !!(avie[i].dwFlags & AVIIF_KEYFRAME);
						if (absolute_addr)
						{
							pasn->index.add(&avie[i]);
						}
						else
						{
							pasn->index.add(avie[i].ckid, (movi_offset - 4) + (__int64)avie[i].dwChunkOffset, avie[i].dwChunkLength, -1, bKey);
						}
						pasn->bytes += avie[i].dwChunkLength;
						pasn->keyonly = pasn->keyonly && bKey;
					}
				}
				else
				{
					bool bKey = !!(avie[i].dwFlags & AVIIF_KEYFRAME);
					if (absolute_addr)
						pasn->index.add(&avie[i]);
					else
						pasn->index.add(avie[i].ckid, (movi_offset - 4) + (__int64)avie[i].dwChunkOffset, avie[i].dwChunkLength, -1, bKey);

					pasn->bytes += avie[i].dwChunkLength;

					pasn->keyonly = pasn->keyonly&&bKey;
				}
			}
		}
	}

	return TRUE;
}
void CMxAviDemuxer::_parseExtendedIndexBlock(CMxList<CAVIStreamNode *>& streamlist, CAVIStreamNode *pasn, __int64 fpos, DWORD dwLength) {
	union {
		AVISUPERINDEX idxsuper;
		AVISTDINDEX idxstd;
	};
	union {
		struct	_avisuperindex_entry		superent[64];
		DWORD	dwHeap[256];
	};


	int entries, tp;
	int i;
	__int64 i64FPSave = _posFile();
	if (fpos >= 0)
	{
		if (!_seekFile(fpos)) return;
	}
	memset(&idxsuper, 0, sizeof(AVISUPERINDEX));
	idxsuper.cb = sizeof(AVISUPERINDEX);
	_readFile((PBYTE)&idxsuper + 8, sizeof(AVISUPERINDEX) - 8);

	switch (idxsuper.bIndexType)
	{
	case AVI_INDEX_OF_INDEXES:
		// sanity check

		if (idxsuper.wLongsPerEntry != 4)
			assert(FALSE);

		entries = idxsuper.nEntriesInUse;

		while (entries > 0)
		{
			tp = sizeof(superent) / sizeof(superent[0]);
			if (tp > entries) tp = entries;

			_readFile(superent, tp * sizeof(superent[0]));

			for (i = 0; i < tp; i++)
				_parseExtendedIndexBlock(streamlist, pasn, superent[i].qwOffset + 8, superent[i].dwSize - 8);

			entries -= tp;
		}

		break;

	case AVI_INDEX_OF_CHUNKS:
		entries = idxstd.nEntriesInUse;
		// In theory, if bIndexSubType==AVI_INDEX_2FIELD it's supposed to have
		// wLongsPerEntry=3, and bIndexSubType==0 gives wLongsPerEntry=2.
		// Matrox's MPEG-2 stuff generates bIndexSubType=16 and wLongsPerEntry=6.
		// *sigh*
		//
		// For wLongsPerEntry==2 and ==3, dwOffset is at 0 and dwLength at 1;
		// for wLongsPerEntry==6, dwOffset is at 2 and all are keyframes.

		{
			__int64 llBloackOffset = idxstd.qwBaseOffset;//(idxstd.dwHBaseOffset<<32)|idxstd.dwLBaseOffset;
			if (idxstd.wLongsPerEntry != 2 && idxstd.wLongsPerEntry != 3 && idxstd.wLongsPerEntry != 6)
				assert(FALSE);

			while (entries > 0) {
				tp = (sizeof(dwHeap) / sizeof(dwHeap[0])) / idxstd.wLongsPerEntry;
				if (tp > entries) tp = entries;

				_readFile(dwHeap, tp*idxstd.wLongsPerEntry * sizeof(DWORD));

				if (idxstd.wLongsPerEntry == 6)
					for (i = 0; i < tp; i++)
					{
						__int64 llOffset = dwHeap[i*idxstd.wLongsPerEntry + 1];
						llOffset <<= 32;
						llOffset += dwHeap[i*idxstd.wLongsPerEntry + 0];

						DWORD dwSize = dwHeap[i*idxstd.wLongsPerEntry + 2];
						LONG lDisplayNo = dwHeap[i*idxstd.wLongsPerEntry + 3];
						bool keyframe = !(dwSize & 0x80000000);
						pasn->index.add(idxstd.dwChunkId, llBloackOffset + llOffset - 8, dwSize, lDisplayNo, keyframe);
						pasn->bytes += dwSize & 0x1FFFFFFF;
						pasn->keyonly = pasn->keyonly&&keyframe;
					}
				else
					for (i = 0; i < tp; i++)
					{
						DWORD dwOffset = dwHeap[i*idxstd.wLongsPerEntry + 0];
						DWORD dwSize = dwHeap[i*idxstd.wLongsPerEntry + 1];
						bool keyframe = !(dwSize & 0x80000000);
						pasn->index.add(idxstd.dwChunkId, (llBloackOffset + dwOffset) - 8, dwSize & 0x7FFFFFFF, -1, !(dwSize & 0x80000000));
						pasn->bytes += dwSize & 0x1FFFFFFF;
						pasn->keyonly = pasn->keyonly&&keyframe;
					}

				entries -= tp;
			}
		}

		break;

	default:
		assert(FALSE);
	}

	_seekFile(i64FPSave);
}