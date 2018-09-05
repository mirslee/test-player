

#ifndef __CMXDEMUXER_H__
#define __CMXDEMUXER_H__

#include "MxCodec.h"
#include "MxSynchronize.h"
#include "MxPointer.h"
#include "MxCodec.h"
#include "CMxList.h"
#include "CMxQueue.h"
#include "CMxStreamReader.h"
#include "MxAvi.h"

class CMxDemuxer : public MxDemuxer, public CMxObject {
    MX_OBJECT
    friend class CMxStreamReader;
public:
    CMxDemuxer(MxSource* source, uint32 type);
    virtual ~CMxDemuxer();
    
public:
    virtual bool init();
    virtual void unInit();
    virtual long readData(ulong fccType, int stream, BYTE*buffer, int64 position, long len, ulong dwReserve, bool bSeek, bool bFastRead, int nIoID);
    
public:
    MxMuxerInfo* getFormat() { return &m_muxinfo; }
    long getSource(MxSource** src) { *src = m_pSource; return 1; }
    long getStreams() { return m_streams; }
    long getStream(ulong fccType, long id, MxStreamReader** stream);
    void reset() {}
    bool canRefresh() { return false; }
    bool refresh() { return false; }
    
protected:
    void onDelete() { unInit(); }
    enum { STREAM_SIZE = 1048576 };
    enum { STREAM_RT_SIZE = 65536 };
    enum { STREAM_BLOCK_SIZE = 4096 };
    
    CMxList<CAVIStreamNode*> m_listStreams;
    virtual bool _parseFile(CMxList<CAVIStreamNode*>& streams) = 0;
    
protected:
    CMxMutex m_csRead;
    CMxObjectPointer<MxSource> m_pSource;
    MxMuxerInfo m_muxinfo;
    int            m_streams;
    
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
    int64 _seekFileEx(int64 i64NewPos);
    bool _skipFile(int64 bytes);
    int64 _posFile();
    int64 _sizeFile();
};

namespace VXAUXTOOLS_CPP_NAMESPACE {
	typedef int64 QUADWORD;

}


#endif /* __CMXDEMUXER_H__ */
