
#include "stdafx.h"
#include "CMxDemuxer.h"
#include "MxMemory.h"

namespace VXAUXTOOLS_CPP_NAMESPACE {
	CAVIIndexChain::CAVIIndexChain()
	{
		head = tail = NULL;
		total_ents = 0;
	}

	void CAVIIndexChain::delete_chain()
	{
		CAVIIndexChainNode *aicn = head, *aicn2;
		while (aicn)
		{
			aicn2 = aicn->next;
			delete aicn;
			aicn = aicn2;
		}
		head = tail = NULL;
	}

	CAVIIndexChain::~CAVIIndexChain()
	{
		delete_chain();
	}

	bool CAVIIndexChain::add(AVIIndexEntry *avie)
	{
		if (!tail || !tail->add(avie->ckid, avie->dwChunkOffset, avie->dwChunkLength, total_ents, !!(avie->dwFlags & AVIIF_KEYFRAME)))
		{
			CAVIIndexChainNode *aicn = new CAVIIndexChainNode();
			if (tail) tail->next = aicn; else head = aicn;
			tail = aicn;
			if (tail->add(avie->ckid, avie->dwChunkOffset, avie->dwChunkLength, total_ents, !!(avie->dwFlags & AVIIF_KEYFRAME)))
			{
				++total_ents;
				return true;
			}

			return false;
		}

		++total_ents;

		return true;
	}

	bool CAVIIndexChain::add(AVIIndexEntry2 *avie2)
	{
		return add(avie2->ckid, avie2->pos, avie2->size & 0x3FFFFFFF, avie2->displayno, !!(avie2->size & 0x80000000), avie2->reserve);
	}

	bool CAVIIndexChain::add(DWORD ckid, __int64 pos, LONG size, LONG displayno, bool is_keyframe, DWORD reserve)
	{
		if (displayno == -1)
			displayno = total_ents;
		if (!tail || !tail->add(ckid, pos, size, displayno, is_keyframe, reserve))
		{
			CAVIIndexChainNode *aicn = new CAVIIndexChainNode();

			if (tail) tail->next = aicn; else head = aicn;
			tail = aicn;

			if (tail->add(ckid, pos, size, displayno, is_keyframe, reserve))
			{
				++total_ents;
				return TRUE;
			}

			return FALSE;
		}

		++total_ents;

		return TRUE;
	}

	void CAVIIndexChain::put(AVIIndexEntry *avietbl)
	{
		CAVIIndexChainNode *aicn = head;

		while (aicn)
		{
			aicn->put(avietbl);
			aicn = aicn->next;
		}

		delete_chain();
	}

	void CAVIIndexChain::put(AVIIndexEntry2 *avie2tbl)
	{
		CAVIIndexChainNode *aicn = head;

		while (aicn)
		{
			aicn->put(avie2tbl);
			aicn = aicn->next;
		}

		delete_chain();
	}

	void CAVIIndexChain::put(AVIIndexEntry3 *avie3tbl, __int64 offset)
	{
		CAVIIndexChainNode *aicn = head;

		while (aicn)
		{
			aicn->put(avie3tbl, offset);
			aicn = aicn->next;
		}

		delete_chain();
	}

	CAVIIndex::CAVIIndex()
	{
		index = NULL;
		index2 = NULL;
		index3 = NULL;
		oldindex2s.SetMaxSize(8);
	}

	CAVIIndex::~CAVIIndex()
	{
		delete[] index;
		delete[] index2;
		delete[] index3;
		AVIIndexEntry2* oldinex2 = nullptr;
		while (oldindex2s.Pop(&oldinex2, false))
			delete[] oldinex2;
	}

	bool CAVIIndex::makeIndex()
	{
		AVIIndexEntry* newindex = new AVIIndexEntry[total_ents];
		put(newindex);
		AVIIndexEntry* oldindex = (AVIIndexEntry*)InterlockedExchangePointer((void**)&index, newindex);
		if (oldindex) delete oldindex;
		index_len = total_ents;
		return true;
	}

	bool CAVIIndex::makeIndex2()
	{
		AVIIndexEntry2* newindex2 = new AVIIndexEntry2[total_ents];
		put(newindex2);
		AVIIndexEntry2* oldindex2 = (AVIIndexEntry2*)InterlockedExchangePointer((void**)&index2, newindex2);
		index_len = total_ents;
		if (oldindex2)
		{
			if (oldindex2s.IsFull())
			{
				AVIIndexEntry2* delentry = nullptr;
				oldindex2s.Pop(&delentry);
				delete[] delentry;
			}
			oldindex2s.Push(oldindex2);
		}
		return true;
	}

	bool CAVIIndex::makeIndex3(int64 offset)
	{
		AVIIndexEntry3* newindex3 = new AVIIndexEntry3[total_ents];
		put(newindex3, offset);
		AVIIndexEntry3* oldindex3 = (AVIIndexEntry3*)InterlockedExchangePointer((void**)&index3, newindex3);
		if (oldindex3) delete oldindex3;
		index_len = total_ents;
		return true;
	}


	void CAVIIndex::clear()
	{
		delete_chain();
		delete[] index;
		delete[] index2;
		delete[] index3;
		index = nullptr;
		index2 = nullptr;
		index3 = nullptr;
		total_ents = 0;
	}

	CAVIStreamNode::CAVIStreamNode(DWORD fcc)
		: fccType(fcc)
	{
		bytes = 0;
		strmid = 0;
		starttime = 0;
		priv_data = NULL;
		keyonly = TRUE;
		memset(&vinfo, 0, sizeof(MxVideoInfo));
	}

	CAVIStreamNode::~CAVIStreamNode()
	{
		if (fccType == vxstreamVIDEO)
		{
			if (vinfo.pExtraData) mx_free(vinfo.pExtraData);
			if (vinfo.metadata) mx_free(vinfo.metadata);
		}
		else if (fccType == vxstreamAUDIO)
		{
			if (ainfo.pExtraData) mx_free(ainfo.pExtraData);
			if (ainfo.metadata) mx_free(ainfo.metadata);
		}
	}

	CMxDemuxer::CMxDemuxer(MxSource* source, DWORD type)
	: m_pSource(source)
	{
		mxMutexInit(&m_csRead);
		memset(&m_muxinfo, 0, sizeof(m_muxinfo));
		m_muxinfo.fourcc = type;
	}

	CMxDemuxer::~CMxDemuxer(void)
	{
		if (m_muxinfo.pExtraData) mx_free(m_muxinfo.pExtraData);
		if (m_muxinfo.metadata) mx_free(m_muxinfo.metadata);
		mxMutexDestroy(&m_csRead);
	}

	bool CMxDemuxer::Initialize()
	{
		m_pSource->Seek(0);
		bool bFinally = FALSE;
#ifndef _DEBUG
		try
		{
#endif
			m_streams = 0;
			if (!_parseFile(m_listStreams)) return FALSE;
			m_streams = m_listStreams.GetSize();
			if (!m_muxinfo.bitrate)
			{
				LONG bitrate = 0;
				MXPOSITION pos = m_listStreams.GetHeadPosition();
				while (pos)
				{
					CAVIStreamNode* node = m_listStreams.GetNext(pos);
					if (node->fccType == vxstreamVIDEO)
					{
						bitrate += node->vinfo.bitrate;
						if (node->vinfo.dwKeyframeSpace == 0 && node->vinfo.dectype != 1)
						{   //有索引的时候，根据帧类型获取GOPsize及B帧个数
							static DWORD frametype[] = { vxFrameIType,vxFrameBType,vxFramePType };
							int length = node->vinfo.frames;
							int frmIdx = 0;
							for (; frmIdx < length; frmIdx++)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] == vxFrameIType)
									break;
							}
							int frstI = frmIdx++;
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] != vxFramePType)
									break;
								frmIdx++;
							}
							int bFrames = 0;
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] != vxFrameBType)
									break;
								else
									bFrames++;
								frmIdx++;
							}
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] == vxFrameIType)
									break;
								frmIdx++;
							}
							int scndI = frmIdx++;
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] == vxFrameIType)
									break;
								frmIdx++;
							}
							int thrdI = frmIdx;
							int gopsize = max(scndI - frstI, thrdI - scndI);
							node->vinfo.dwKeyframeSpace = bFrames << 8 | gopsize;
						}
					}
					else
						bitrate += node->ainfo.bitrate;
				}
				m_muxinfo.bitrate = bitrate;
			}

#ifndef _DEBUG
		}
		catch (...)
		{
			Uninitialize();
			bFinally = TRUE;
		}
#endif
		m_pSource->InfoEnd();
		return !bFinally;
	}

	void CMxDemuxer::Uninitialize()
	{
		CAVIStreamNode *pasn;
		MXPOSITION pos = m_listStreams.GetHeadPosition();
		while (pos)
		{
			pasn = m_listStreams.GetNext(pos);
			delete pasn;
		}
		m_listStreams.RemoveAll();

		//_vxRefObject(m_pSource->GetFileId(), NULL);
	}


	LONG CMxDemuxer::GetStream(DWORD fccType, LONG lParam, MxStreamReader** stream)
	{
		if (lParam < 0) return -1;
		CAVIStreamNode *pasn;
		int streamno = 0;

		bool bFind = FALSE;
		MXPOSITION pos = m_listStreams.GetHeadPosition();
		while (pos)
		{
			pasn = m_listStreams.GetNext(pos);
			if ((pasn->fccType == fccType) || (fccType == 0))
			{
				if (lParam <= 0)
				{
					bFind = TRUE;
					break;
				}
				lParam--;
			}
			streamno++;
		}

		if (bFind)
		{
			if (pasn->fccType == vxstreamAUDIO&&pasn->ainfo.blockalign == 0)
			{
				pasn->ainfo.blockalign = pasn->ainfo.channels*((pasn->ainfo.bitpersample + 7) >> 3);
				if (pasn->ainfo.fourcc == vxFormat_PCM)
				{
					pasn->length = (LONG)pasn->bytes / pasn->ainfo.blockalign;
				}
			}
			CMxStream* gstream = nullptr;
			if (pasn->fccType == vxstreamVIDEO)
				gstream = new CMxVideoStream(this, pasn, streamno);
			else if (pasn->fccType == vxstreamAUDIO)
				gstream = new CMxAudioStream(this, pasn, streamno);
			if (gstream)
			{
				//return GetVxInterface(static_cast<IVxReadStream*>(gstream), (void**)stream);
				*stream = static_cast<IVxReadStream*>(gstream);
				return 0;
			}	
			else
				return -1;
		}

		return -1;
	}

	long CMxDemuxer::ReadData(DWORD dwType, int stream, BYTE* buffer, int64 position, LONG len, DWORD dwReserve, bool bSeek, bool bFastRead, int nIoID)
	{
		if (bFastRead&&m_pSource->IsFastIO(nIoID))
		{
			len = m_pSource->FastRead(position, buffer, len, stream, bSeek, nIoID);
			if (len < 0)
			{
				char vErrStr[256] = { 0 };
				//sprintf(vErrStr, vxLoadMessageLV("source FastIO read file Error, Pos:%lld"), position);
				//VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxKeyDemultipexer::ReadData"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
			}
		}
		else
		{
			CMxMutexLocker locker(&m_csRead);
			if (!_seekFile(position))
			{
				char vErrStr[256] = { 0 };
				//sprintf(vErrStr, vxLoadMessageLV("seek source file Error, Pos:%lld"), position);
				//VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxKeyDemultipexer::ReadData"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
				return -1;
			}
			len = _readFile(buffer, len, bSeek);
			if (len < 0)
			{
				char vErrStr[256] = { 0 };
				//sprintf(vErrStr, vxLoadMessageLV("source read file Error, Pos:%lld"), position);
				//VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxKeyDemultipexer::ReadData"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
			}
		}
		return len;
	}

	int64 CMxDemuxer::vxio_seek(int64 offset, int whence)
	{
		if (whence == VXSEEK_SIZE)
			return m_pSource->GetSize();
		int64 offset1 = offset;
		if (whence == SEEK_CUR)
			offset1 = m_pSource->GetPosition() + offset;
		else if (whence == SEEK_END)
			offset1 = m_pSource->GetSize() - offset;
		return m_pSource->Seek(offset1);
	}

	LONG CMxDemuxer::_readFile(void *data, long len, int bSeek)
	{
		return m_pSource->Read((BYTE*)data, len, bSeek);
	}

	bool CMxDemuxer::_readChunkHeader(DWORD& pfcc, DWORD& pdwLen)
	{
		DWORD dw[2];
		LONG actual;
		actual = _readFile(dw, 8, false);
		if (actual != 8) return false;
		pfcc = dw[0];
		pdwLen = dw[1];
		return true;
	}

	bool CMxDemuxer::_seekFile(int64 i64NewPos)
	{
		return (m_pSource->Seek(i64NewPos) == i64NewPos);
	}

	int64 CMxDemuxer::_seekFileEx(int64 i64NewPos)
	{
		return m_pSource->Seek(i64NewPos);
	}

	bool CMxDemuxer::_skipFile(int64 bytes)
	{
		if (!_seekFile(m_pSource->GetPosition() + bytes))
			return false;
		return true;
	}

	int64 CMxDemuxer::_posFile()
	{
		return m_pSource->GetPosition();
	}

	int64 CMxDemuxer::_sizeFile()
	{
		return m_pSource->GetSize();
	}

	CMxStream::CMxStream(CMxDemuxer* demux, CAVIStreamNode* node, int idx)
	: m_demux(demux)
		, m_psnData(node)
		, m_strmid(idx)
		, m_length(node->length)
		, m_frames(node->frames)
	{
		demux->addRef();
	}

	bool CMxStream::IsKeyFrame(LONG lFrame)
	{
		if (m_sampsize)	return TRUE;
		if (lFrame < 0 || lFrame >= m_length)
			return FALSE;
		return !(m_psnData->index.index2Ptr()[lFrame].size & 0xE0000000);
	}

	LONG CMxStream::PrevKeyFrame(LONG lFrame)
	{
		if (m_sampsize)	return lFrame > 0 ? lFrame - 1 : -1;
		if (lFrame < 0)	return -1;

		if (lFrame > m_length) lFrame = (LONG)m_length;

		while (--lFrame >= 0)
		{
			if (!(m_psnData->index.index2Ptr()[lFrame].size & 0xE0000000))
				return lFrame;
		}
		return -1;
	}

	LONG CMxStream::NextKeyFrame(LONG lFrame)
	{
		if (m_sampsize)	return lFrame < m_length ? lFrame + 1 : -1;

		if (lFrame < 0) return 0;
		if (lFrame >= m_length)	return -1;

		while (++lFrame < m_length)
			if (!(m_psnData->index.index2Ptr()[lFrame].size & 0xE0000000))
				return lFrame;
		return -1;
	}

	LONG CMxStream::NearestKeyFrame(LONG lFrame)
	{
		if (m_sampsize)	return lFrame;

		if (IsKeyFrame(lFrame))
			return lFrame;

		LONG lprev = PrevKeyFrame(lFrame);

		if (lprev < 0)
			return 0;
		else
			return lprev;
	}

	uint CMxStream::GetFrameType(LONG lFrame)
	{
		if (m_sampsize)	return 1;
		if (lFrame < 0 || lFrame >= m_length)
			return false;
		static DWORD frametype[] = { vxFrameIType,vxFrameBType,vxFramePType };
		int idx = ((DWORD)(m_psnData->index.index2Ptr()[lFrame].size) >> 29);
		return frametype[idx];
	}


	LONG CMxStream::Read2(__int64 start, LONG lSamples, PBYTE lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples, DWORD* ftype, int bSeek, bool bFastRead, int nIoID, VXFEXTINFO* extinfo)
	{
		LONG lActual;
		if (lSamples <= 0 && lSamples != -1)
		{
			if (plBytes) *plBytes = 0;
			if (plSamples) *plSamples = 0;
			return 0;
		}
		if (start < 0) start = 0;
		if (start >= m_length) start = m_length - 1;
		VXFEXTINFO tmpinfo = { 0 };
		if (!extinfo) extinfo = &tmpinfo;

		if (m_sampsize)
		{
			AVIIndexEntry2 *avie2, *avie2_limit = m_psnData->index.index2Ptr() + m_frames;
			__int64 byte_off = (__int64)start * m_sampsize;
			__int64 bytecnt;
			__int64 actual_bytes = 0;
			__int64 block_pos;

			// too small to hold a sample?

			if (lpBuffer && (cbBuffer < m_sampsize))
			{
				if (plBytes) *plBytes = m_sampsize * lSamples;
				if (plSamples) *plSamples = lSamples;
				return -1;
			}

			// find the frame that has the starting sample -- try and work
			// from our last position to save time

			if (byte_off >= m_i64CachedPosition)
			{
				block_pos = m_i64CachedPosition;
				avie2 = m_pCachedEntry;
				byte_off -= block_pos;
			}
			else
			{
				block_pos = 0;
				avie2 = m_psnData->index.index2Ptr();
			}

			while ((byte_off >= (avie2->size & 0x3FFFFFFF)) && (avie2 < avie2_limit))
			{
				byte_off -= (avie2->size & 0x3FFFFFFF);
				block_pos += (avie2->size & 0x3FFFFFFF);
				++avie2;
			}

			m_pCachedEntry = avie2;
			m_i64CachedPosition = block_pos;

			// Client too lazy to specify a size?

			if (lSamples == -1)
			{
				lSamples = ((avie2->size & 0x3FFFFFFF) - (LONG)byte_off) / m_sampsize;

				if (!lSamples && avie2 + 1 < avie2_limit)
					lSamples = ((avie2[0].size & 0x3FFFFFFF) + (avie2[1].size & 0x3FFFFFFF) - (LONG)byte_off) / m_sampsize;

				if (lSamples < 0)
					lSamples = 1;
			}

			// trim down sample count

			if (lpBuffer && (lSamples > cbBuffer / m_sampsize))
				lSamples = cbBuffer / m_sampsize;
			if (start + lSamples > m_length)
				lSamples = (LONG)(m_length - start);

			bytecnt = lSamples * m_sampsize;

			// begin reading frames from this point on

			if (lpBuffer)
			{

				while (bytecnt > 0)
				{
					LONG tc = (avie2->size & 0x3FFFFFFF) - (LONG)byte_off;
					if (tc > bytecnt)
						tc = (LONG)bytecnt;

					__int64 fileoffset = avie2->pos + byte_off + 8;
					if ((lActual = m_demux->ReadData(m_psnData->fccType, m_strmid, lpBuffer, fileoffset, tc, avie2->reserve, bSeek, bFastRead, nIoID)) < 0)
						break;
					actual_bytes += lActual;
					++avie2;
					byte_off = 0;
					if ((extinfo->infotype&FEEXTINFO_OFFSET) && (extinfo->fileoffset == 0))
						extinfo->fileoffset = fileoffset;

					if (lActual < tc)
						break;

					bytecnt -= tc;
					lpBuffer = lpBuffer + tc;
				}

				if (actual_bytes < m_sampsize)
				{
					if (plBytes) *plBytes = 0;
					if (plSamples) *plSamples = 0;
					return -1;
				}

				actual_bytes -= actual_bytes % m_sampsize;

				if (plBytes) *plBytes = (LONG)actual_bytes;
				if (plSamples) *plSamples = (LONG)actual_bytes / m_sampsize;
			}
			else
			{
				if (plBytes) *plBytes = (LONG)bytecnt;
				if (plSamples) *plSamples = lSamples;
			}

		}
		else if (start < m_frames)
		{
			AVIIndexEntry2 *avie2 = &m_psnData->index.index2Ptr()[start];

			if (lpBuffer && (avie2->size & 0x1FFFFFFF) > cbBuffer)
			{
				if (plBytes) *plBytes = avie2->size & 0x1FFFFFFF;
				if (plSamples) *plSamples = 1;

				char vErrStr[256] = { 0 };
				//sprintf(vErrStr, vxLoadMessageLV("read stream Frame Error, start:%lld, size:%ld > cbBuffer:%ld"), start, avie2->size & 0x1FFFFFFF, cbBuffer);
				//VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxKeyStream::Read2"), 0, MAILSRC_HWENGINE | MAILSRC_ERROR);
				return -1;
			}

			__int64 fileoffset = avie2->pos + 8;
			LONG bytecnt = avie2->size & 0x1FFFFFFF;
			if (lpBuffer)
			{
				lActual = m_demux->ReadData(m_psnData->fccType, m_strmid, lpBuffer, fileoffset, bytecnt, avie2->reserve, bSeek, bFastRead, nIoID);
				if (lActual != bytecnt)
				{
					if (plBytes) *plBytes = 0;
					if (plSamples) *plSamples = 0;

					char vErrStr[256] = { 0 };
					//sprintf(vErrStr, vxLoadMessageLV("read stream Frame Error, start:%lld, lActual:%ld != bytecnt:%ld"), start, lActual, bytecnt);
					//VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxKeyStream::Read2"), 0, MAILSRC_HWENGINE | MAILSRC_ERROR);
					return -1;
				}
			}
			else
				lActual = bytecnt;
			if (extinfo->infotype&FEEXTINFO_OFFSET)
				extinfo->fileoffset = fileoffset;

			if (plBytes) *plBytes = lActual;
			if (plSamples) *plSamples = 1;
			if (ftype)
			{
				static DWORD frametype[] = { vxFrameIType,vxFrameBType,vxFramePType };
				int idx = (avie2->size >> 29);
				*ftype = frametype[idx];
			}
		}
		else
		{
			char vErrStr[256] = { 0 };
			//sprintf(vErrStr, vxLoadMessageLV("read stream Frame Error, start:%lld, start:%lld >= m_frames:%lld"), start, start, m_frames);
			//VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxKeyStream::Read2"), 0, MAILSRC_HWENGINE | MAILSRC_ERROR);
			return -1;
		}

		return 0;
	}

	LONG  CMxStream::Read(__int64 start, LONG lSamples, PBYTE lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples, DWORD* ftype, int bSeek, bool bFastRead, int nIoID)
	{
		VXFEXTINFO extinfo = { 0 };
		return Read2(start, lSamples, lpBuffer, cbBuffer, plBytes, plSamples, ftype, bSeek, bFastRead, nIoID, &extinfo);
	}

	LONG CMxStream::NonDelegatingQueryInterface(LONG iid, void** ppv)
	{
		if (iid == LIID_IVxReadStream)
		{
			//return GetVxInterface(static_cast<IVxReadStream*>(this), ppv);
			*ppv = static_cast<MxStreamReader*>(this);
			return 0;
		}	
		else if (iid == LIID_IVxReadStream2)
		{
			//return GetVxInterface(static_cast<IVxReadStream2*>(this), ppv);
			*ppv = static_cast<MxStreamReader*>(this);
			return 0;
		}	
		else
			return CMxObject::queryInterfaceDelgate(iid, ppv);
	}

	CMxVideoStream::CMxVideoStream(CMxDemuxer* demux, CAVIStreamNode* node, int idx)
		: CMxStream(demux, node, idx)
	{
		m_sampsize = 0;
	}

	LONG CMxVideoStream::NonDelegatingQueryInterface(LONG iid, void** ppv)
	{
		if (iid == LIID_IVxVideoReadStream)
		{
			//return GetVxInterface((MxVideoStreamReader*)this, ppv);
			*ppv = (MxVideoStreamReader*)this;
			return 0;
		}
		else
		{
			return CMxStream::NonDelegatingQueryInterface(iid, ppv);
		}
	}

	CMxAudioStream::CMxAudioStream(CMxDemuxer* demux, CAVIStreamNode* node, int idx)
		: CMxStream(demux, node, idx)
	{
		if (((node->ainfo.dectype == 0) && (node->ainfo.framesize == 0)) || (node->ainfo.dectype == 2))
			m_sampsize = node->ainfo.blockalign;
		else
			m_sampsize = node->ainfo.framesize;

		m_i64CachedPosition = 0;
		m_pCachedEntry = m_psnData->index.index2Ptr();
	}


	LONG CMxAudioStream::NonDelegatingQueryInterface(LONG iid, void** ppv)
	{
		if (iid == LIID_IVxAudioReadStream)
		{
			//return GetVxInterface((IVxAudioReadStream*)this, ppv);
			*ppv = (MxAudioStreamReader*)this;
			return 1;
		}
		else
		{
			return CMxStream::NonDelegatingQueryInterface(iid, ppv);
		}
	}
}