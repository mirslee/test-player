

#ifndef __CMXDEMUXER_H__
#define __CMXDEMUXER_H__

#include "MxCodec.h"
#include "MxSynchronize.h"
#include "MxPointer.h"
#include "MxCodec.h"
#include "CMxList.h"
#include "CMxQueue.h"

#define vxstreamVIDEO		'sdiv'//mmioFOURCC('v', 'i', 'd', 's')
#define vxstreamAUDIO		'sdua'//mmioFOURCC('a', 'u', 'd', 's')
#define vxstreamSUBPIC		'cips'//mmioFOURCC('s', 'p', 'i', 'c'),	//字幕格式
#define vxstreamDATA		'atad'//mmioFOURCC('d', 'a', 't', 'a')

//I,B,P帧,与MPEG标准一致
#define vxFrameIType	1	
#define vxFramePType	2
#define vxFrameBType	3

namespace VXAUXTOOLS_CPP_NAMESPACE {
	typedef int64 QUADWORD;

#define AVI_INDEX_OF_INDEXES	0x00
#define AVI_INDEX_OF_CHUNKS		0x01
#define AVI_INDEX_IS_DATA		0x80
#define AVI_INDEX_2FIELD		0x01

	struct _avisuperindex_entry {
		QUADWORD qwOffset;	// absolute file offset, offset 0 is
		DWORD dwSize;		// size of index chunk at this offset
		DWORD dwDurationl;	// time span in stream ticks
	};

	struct _avistdindex_entry
	{
		DWORD dwOffset;		// qwBaseOffset + this is absolute file offset
		DWORD dwSize;		// bit 31 is set if this is NOT a keyframe
	};

	struct _avifieldindex_entry
	{
		DWORD dwOffset;
		DWORD dwSize;
		DWORD dwOffsetField2;
	};

#pragma pack(push,2)

	typedef struct _avistdindex_chunk
	{
		DWORD fcc;
		DWORD cb;
		WORD wLongsPerEntry;			// must be sizeof(aIndex[0])/sizeof(DWORD)
		BYTE bIndexSubType;				// must be 0
		BYTE bIndexType;				// must be AVI_INDEX_OF_CHUNKS
		DWORD nEntriesInUse;
		DWORD dwChunkId;
		QUADWORD qwBaseOffset;			// all dwOffsets in aIndex array are
		DWORD dwReserved3;				// must be 0
		_avistdindex_entry aIndex[];
	} AVISTDINDEX, *PAVISTDINDEX;

	typedef struct _avisuperindex_chunk {
		DWORD fcc;
		DWORD cb;						// size of this structure
		WORD wLongsPerEntry;			// must be 4 (size of each entry in aIndex array)
		BYTE bIndexSubType;				// must be 0 or AVI_INDEX_2FIELD
		BYTE bIndexType;				// must be AVI_INDEX_OF_INDEXES
		DWORD nEntriesInUse;			// number of entries in aIndex array that are used
		DWORD dwChunkId;
		DWORD dwReserved[3];			// must be 0
		struct _avisuperindex_entry aIndex[];
	} AVISUPERINDEX, *PAVISUPERINDEX;

	typedef struct
	{
		DWORD fcc;
		DWORD cb;
		WORD wLongsPerEntry;
		BYTE bIndexSubType;
		BYTE bIndexType;
		DWORD nEntriesInUse;
		DWORD dwChunkId;
		QUADWORD qwBaseOffset;
		DWORD dwReserved3;
		struct _avifieldindex_entry aIndex[];
	} AVIFIELDINDEX, *PAVIFIELDINDEX;

#pragma pack(pop)

	struct AVIIndexEntry {
		DWORD ckid;
		DWORD dwFlags;
		DWORD dwChunkOffset;
		DWORD dwChunkLength;
	};

	struct AVIIndexEntry2
	{
		int64 pos;
		union
		{
			DWORD ckid;
			int fileno;
		};
		long size;
		long displayno;
		DWORD reserve;
	};

	struct AVIIndexEntry3 {
		DWORD dwOffset;
		DWORD dwSizeKeyframe;
	};

	struct AVIIdexEntry4 {
		int64 pos;
		DWORD size;
		DWORD frameindex;
		DWORD realocation;
		DWORD reserve;
	};

#ifndef AVIIF_KEYFRAME
#define AVIIF_KEYFRAME  0x00000010L // this frame is a key frame.
#endif

	class CAVIIndexChainNode {
	public:
		CAVIIndexChainNode() { num_ents = 0; next = nullptr; }
		virtual ~CAVIIndexChainNode() {}

	public:
		enum { ENTS = 2048 };
		CAVIIndexChainNode* next;
		AVIIndexEntry2 ient[ENTS];
		int num_ents;

	public:
		bool add(DWORD ckid, int64 pos, long size, long displayno, bool is_keyframe, DWORD reserve = 0) {
			if (num_ents < ENTS) {
				ient[num_ents].ckid = ckid;
				ient[num_ents].pos = pos;
				ient[num_ents].size = (is_keyframe || (size & 0xC0000000)) ? size : 0x40000000 + size;
				ient[num_ents].displayno = displayno;
				ient[num_ents].reserve = reserve;
				++num_ents;
				return true;
			}
		}
		void remove(int begin, int count) {
			num_ents -= count;
			memmove(ient + begin, ient + begin + count, num_ents * sizeof(AVIIndexEntry2));
		}
		void put(AVIIndexEntry *&avieptr) {
			for (int i = 0; i < num_ents; i++)
			{
				avieptr->ckid = ient[i].ckid;
				avieptr->dwFlags = ient[i].size & 0xC0000000 ? 0 : AVIIF_KEYFRAME;
				avieptr->dwChunkOffset = (DWORD)ient[i].pos;
				avieptr->dwChunkLength = ient[i].size & 0x3FFFFFFF;
				++avieptr;
			}
		}
		void put(AVIIndexEntry2 *& avie2ptr) {
			for (int i = 0; i < num_ents; i++)
			{
				*avie2ptr++ = ient[i];
			}
		}
		void put(AVIIndexEntry3 *&avie3ptr, int64 offset) {
			for (int i = 0; i < num_ents; i++) {
				avie3ptr->dwSizeKeyframe = ient[i].size;
				avie3ptr->dwOffset = (DWORD)(ient[i].pos - offset);
				++avie3ptr;
			}
		}
	};

	class CAVIIndexChain {
	public:
		CAVIIndexChain();
		virtual ~CAVIIndexChain();

	public:
		int total_ents;
		CAVIIndexChainNode *head, *tail;

	protected:
		void delete_chain();
	public:
		bool add(AVIIndexEntry *avie);
		bool add(AVIIndexEntry2 *avie2);
		bool add(DWORD ckid, int64 pos, long len, long displayno, bool is_keyframe, DWORD reserve = 0);
		void put(AVIIndexEntry *avietbl);
		void put(AVIIndexEntry2 *avie2tbl);
		void put(AVIIndexEntry3 *avie3tbl, int64 offset);
	};

	class CAVIIndex : public CAVIIndexChain {
	protected:
		AVIIndexEntry *index;
		AVIIndexEntry2 *index2;
		AVIIndexEntry3 *index3;
		int index_len;
		CMxQueue<AVIIndexEntry2*> oldindex2s;

		AVIIndexEntry *allocateIndex(int total_entries) {
			return index = new AVIIndexEntry[index_len = total_entries];
		}
		AVIIndexEntry2 *allocateIndex2(int total_entries)
		{
			return index2 = new AVIIndexEntry2[index_len = total_entries];
		}
		AVIIndexEntry3 *allocateIndex3(int total_entries)
		{
			return index3 = new AVIIndexEntry3[index_len = total_entries];
		}

	public:
		CAVIIndex();
		virtual ~CAVIIndex();

		bool makeIndex();
		bool makeIndex2();
		bool makeIndex3(int64 offset);
		void clear();
		AVIIndexEntry *indexPtr() {
			return index;
		}
		AVIIndexEntry2 *index2Ptr() {
			return index2;
		}
		AVIIndexEntry3 *index3Ptr() {
			return index3;
		}
		AVIIndexEntry2 *takeIndex2() {
			AVIIndexEntry2 *idx = index2;
			index2 = nullptr;
			return idx;
		}
		int size() {
			return total_ents;
		}
		int indexLen() {
			return index_len;
		}
	};

	class CAVIStreamNode
	{
	public:
		CAVIStreamNode(DWORD fccType);
		virtual ~CAVIStreamNode();

	public:
		ulong fccType;
		union
		{
			ulong fmtfcc;
			MxVideoInfo vinfo;
			MxAudioInfo ainfo;
		};
		CAVIIndex				index;
		int64					bytes;
		int64					length;
		int64					frames;
		int						strmid;
		int64					starttime;
		void*					priv_data;
		bool					keyonly;
	};


	class CMxDemuxer : public MxDemuxer, public CMxObject {
		MX_OBJECT
		friend class CMxStream;
	public:
		CMxDemuxer(MxSource* source, ulong type);
		virtual ~CMxDemuxer();

	public:
		virtual bool Initialize();
		virtual void Uninitialize();
		virtual long ReadData(ulong fccType, int stream, BYTE*buffer, int64 position, long len, ulong dwReserve, bool bSeek, bool bFastRead, int nIoID);

	public:
		MxMuxerInfo* GetFormat() { return &m_muxinfo; }
		long GetSource(MxSource** src) { *src = m_pSource; return 1; }
		long GetStreams() { return m_streams; }
		long GetStream(ulong fccType, long id, MxStreamReader** stream);
		void Reset() {}
		bool CanRefresh() { return false; }
		bool Refresh() { return false; }

	protected:
		void OnDelete() { Uninitialize(); }
		enum { STREAM_SIZE = 1048576 };
		enum { STREAM_RT_SIZE = 65536 };
		enum { STREAM_BLOCK_SIZE = 4096 };

		CMxList<CAVIStreamNode*> m_listStreams;
		virtual bool _parseFile(CMxList<CAVIStreamNode*>& streams) = 0;

	protected:
		CMxMutex m_csRead;
		CMxObjectPointer<MxSource> m_pSource;
		MxMuxerInfo m_muxinfo;
		int			m_streams;

	protected:
#define  vxio_read _readFile
		static int File_Read(void* opaque, BYTE* buf, int buf_size) {
			return ((CMxDemuxer*)opaque)->vxio_read(buf, buf_size, false);
		}
		static int64 File_Seek(void*opaque, int64 offset, int whence) {
			return ((CMxDemuxer*)opaque)->vxio_seek(offset, whence);
		}
		int64 vxio_seek(int64 offset, int whence);

		long _readFile(void* data, long len, bool bSeek = false);
		bool _readChunkHeader(DWORD& pfcc, DWORD &pdwlen);
		bool _seekFile(int64 i64NewPos);
		int64 seekFileEx(int64 i64NewPos);
		bool skipFile(int64 bytes);
		int64 _posFile();
		int64 _sizeFile();
	};

	class CMxStream : public CMxObject, public MxStreamReader, public MxStreamReader2
	{
		MX_OBJECT
	public:
		CMxStream(CMxDemuxer* demux, CAVIStreamNode*, int idx);
		virtual ~CMxStream() { m_demux->unRef(); }
	public:
		CAVIStreamNode *m_psnData;
	protected:
		CMxDemuxer* m_demux;
		__int64 m_length;
		__int64 m_frames;
		LONG m_sampsize;
		int m_strmid;

		__int64		m_i64CachedPosition;
		AVIIndexEntry2	*m_pCachedEntry;

	public:
		mxuvoidptr __stdcall GetFileId() { return m_demux->m_pSource->GetFileId(); };
		long GetDemuxer(MxDemuxer** demux) { /*return GetVxInterface(m_demux, (void**)demux);*/return 1; }
		long GetSource(MxSource** source) { return m_demux->GetSource(source); }
		void __stdcall AddFastIO(MxFastIORead* fastio) { m_demux->m_pSource->AddFastIO(fastio); }
		void __stdcall Clear() {}

		LONG  __stdcall PrevKeyFrame(LONG lFrame);
		LONG  __stdcall NextKeyFrame(LONG lFrame);
		LONG  __stdcall NearestKeyFrame(LONG lFrame);
		uint __stdcall GetFrameType(LONG lFrame);
		LONG  __stdcall Read(__int64 start, LONG lSamples, PBYTE lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples, DWORD* ftype, int bSeek, bool bFastRead = FALSE, int nIoID = 0);

		LONG  __stdcall Read2(__int64 start, LONG lSamples, PBYTE lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples, DWORD* ftype, int bSeek = 0, bool bFastRead = FALSE, int nIoID = 0, VXFEXTINFO* extinfo = NULL);

		LONG __stdcall NonDelegatingQueryInterface(LONG iid, void**);
	protected:
		bool IsKeyFrame(LONG lFrame);

	};


	class CMxVideoStream : public CMxStream, public MxVideoStreamReader
	{
	public:
		CMxVideoStream(CMxDemuxer* demux, CAVIStreamNode*, int idx);
	public:
		uint GetStreamType() { return vxstreamVIDEO; }
		MxVideoInfo* GetFormat() { m_length = m_psnData->length; m_frames = m_psnData->frames; return &m_psnData->vinfo; }
		LONG NonDelegatingQueryInterface(LONG iid, void**);
	};

	class CMxAudioStream : public CMxStream, public MxAudioStreamReader
	{
	public:
		CMxAudioStream(CMxDemuxer* demux, CAVIStreamNode*, int idx);
	public:
		uint __stdcall GetStreamType() { return vxstreamAUDIO; }
		MxAudioInfo* __stdcall GetFormat() { m_length = m_psnData->length; m_frames = m_psnData->frames; return &m_psnData->ainfo; }
		LONG __stdcall NonDelegatingQueryInterface(LONG iid, void**);
	};

}


#endif /* __CMXDEMUXER_H__ */
