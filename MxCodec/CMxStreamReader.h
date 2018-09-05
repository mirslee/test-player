

#ifndef __CMXSTREAMREADER_H__
#define __CMXSTREAMREADER_H__

#include "CMxDemuxer.h"
#include "MxAvi.h"
#include "MxMediaDefine.h"

class CMxStreamReader : public CMxObject, public MxStreamReader, public MxStreamReader2
{
    MX_OBJECT
public:
    CMxStreamReader(CMxDemuxer* demux, CAVIStreamNode*, int idx);
    virtual ~CMxStreamReader() { m_demux->unRef(); }
public:
    CAVIStreamNode *m_psnData;
protected:
    CMxDemuxer* m_demux;
    int64 m_length;
    int64 m_frames;
    long m_sampsize;
    int m_strmid;
    
    int64        m_i64CachedPosition;
    AVIIndexEntry2    *m_pCachedEntry;
    
public:
    mxuvoidptr getFileId() { return m_demux->m_pSource->GetFileId(); };
    long getDemuxer(MxDemuxer** demux) { /*return GetVxInterface(m_demux, (void**)demux);*/return 1; }
    long getSource(MxSource** source) { return m_demux->GetSource(source); }
    void addFastIO(MxFastIORead* fastio) { m_demux->m_pSource->AddFastIO(fastio); }
    void clear() {}
    
    long  prevKeyFrame(long lFrame);
    long  nextKeyFrame(long lFrame);
    long  nearestKeyFrame(long lFrame);
    uint  getFrameType(long lFrame);
    long  read(int64 start, long lSamples, uint8* lpBuffer, long cbBuffer, long *plBytes, long *plSamples, uint32* ftype, int bSeek, bool bFastRead = FALSE, int nIoID = 0);
    
    long  read2(int64 start, long lSamples, uint8* lpBuffer, long cbBuffer, long *plBytes, long *plSamples, uint32* ftype, int bSeek = 0, bool bFastRead = FALSE, int nIoID = 0, VXFEXTINFO* extinfo = NULL);
    
    long queryInterfaceDelgate(long iid, void**);
protected:
    bool isKeyFrame(LONG lFrame);
    
};

class CMxVideoStreamReader : public CMxStreamReader, public MxVideoStreamReader
{
public:
    CMxVideoStreamReader(CMxDemuxer* demux, CAVIStreamNode*, int idx);
public:
    uint GetStreamType() { return MxStreamType_video; }
    MxVideoInfo* GetFormat() { m_length = m_psnData->length; m_frames = m_psnData->frames; return &m_psnData->vinfo; }
    long NonDelegatingQueryInterface(LONG iid, void**);
};

class CMxAudioStreamReader : public CMxStreamReader, public MxAudioStreamReader
{
public:
    CMxAudioStreamReader(CMxStreamReader* demux, CAVIStreamNode*, int idx);
public:
    uint GetStreamType() { return MxStreamType_audio; }
    MxAudioInfo* GetFormat() { m_length = m_psnData->length; m_frames = m_psnData->frames; return &m_psnData->ainfo; }
    LONG NonDelegatingQueryInterface(LONG iid, void**);
};

#endif /* __CMXSTREAMREADER_H__ */
