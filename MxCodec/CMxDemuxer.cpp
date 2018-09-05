
#include "stdafx.h"
#include "CMxDemuxer.h"
#include "MxMemory.h"


	CMxDemuxer::CMxDemuxer(MxSource* source, uint32 type)
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

	bool CMxDemuxer::init()
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
					if (node->fccType == MxStreamType_video)
					{
						bitrate += node->vinfo.bitrate;
						if (node->vinfo.dwKeyframeSpace == 0 && node->vinfo.dectype != 1)
						{   //有索引的时候，根据帧类型获取GOPsize及B帧个数
							static uint32 frametype[] = { MxFrameType_I,MxFrameType_B,MxFrameType_P };
							int length = node->vinfo.frames;
							int frmIdx = 0;
							for (; frmIdx < length; frmIdx++)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] == MxFrameType_I)
									break;
							}
							int frstI = frmIdx++;
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] != MxFrameType_P)
									break;
								frmIdx++;
							}
							int bFrames = 0;
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] != MxFrameType_B)
									break;
								else
									bFrames++;
								frmIdx++;
							}
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] == MxFrameType_I)
									break;
								frmIdx++;
							}
							int scndI = frmIdx++;
							while (frmIdx < length)
							{
								if (frametype[((DWORD)(node->index.index2Ptr()[frmIdx].size) >> 29)] == MxFrameType_I)
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
			unInit();
			bFinally = TRUE;
		}
#endif
		m_pSource->InfoEnd();
		return !bFinally;
	}

	void CMxDemuxer::unInit()
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


	long CMxDemuxer::getStream(uint32 fccType, long lParam, MxStreamReader** stream)
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
			if (pasn->fccType == MxStreamType_audio&&pasn->ainfo.blockalign == 0)
			{
				pasn->ainfo.blockalign = pasn->ainfo.channels*((pasn->ainfo.bitpersample + 7) >> 3);
				if (pasn->ainfo.fourcc == MxFormatType_pcm)
				{
					pasn->length = (LONG)pasn->bytes / pasn->ainfo.blockalign;
				}
			}
			CMxStreamReader* gstream = nullptr;
			if (pasn->fccType == MxStreamType_video)
				gstream = new CMxVideoStreamReader(this, pasn, streamno);
			else if (pasn->fccType == MxStreamType_audio)
				gstream = new CMxAudioStreamReader(this, pasn, streamno);
			if (gstream)
			{
				//return GetVxInterface(static_cast<IVxReadStream*>(gstream), (void**)stream);
				*stream = static_cast<MxStreamReader*>(gstream);
				return 0;
			}	
			else
				return -1;
		}

		return -1;
	}

	long CMxDemuxer::readData(uint32 dwType, int stream, BYTE* buffer, int64 position, LONG len, DWORD dwReserve, bool bSeek, bool bFastRead, int nIoID)
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

	long CMxDemuxer::_readFile(void *data, long len, bool bSeek)
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

	
namespace VXAUXTOOLS_CPP_NAMESPACE {
}
