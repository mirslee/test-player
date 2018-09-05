
#include "stdafx.h"
#include "CMxStreamReader.h"

CMxStreamReader::CMxStreamReader(CMxDemuxer* demux, CAVIStreamNode* node, int idx)
: m_demux(demux)
, m_psnData(node)
, m_strmid(idx)
, m_length(node->length)
, m_frames(node->frames)
{
    demux->addRef();
}

bool CMxStreamReader::isKeyFrame(long lFrame)
{
    if (m_sampsize)    return TRUE;
    if (lFrame < 0 || lFrame >= m_length)
        return FALSE;
    return !(m_psnData->index.index2Ptr()[lFrame].size & 0xE0000000);
}

long CMxStreamReader::prevKeyFrame(long lFrame)
{
    if (m_sampsize)    return lFrame > 0 ? lFrame - 1 : -1;
    if (lFrame < 0)    return -1;
    
    if (lFrame > m_length) lFrame = (long)m_length;
    
    while (--lFrame >= 0)
    {
        if (!(m_psnData->index.index2Ptr()[lFrame].size & 0xE0000000))
            return lFrame;
    }
    return -1;
}

long CMxStreamReader::nextKeyFrame(long lFrame)
{
    if (m_sampsize)    return lFrame < m_length ? lFrame + 1 : -1;
    
    if (lFrame < 0) return 0;
    if (lFrame >= m_length)    return -1;
    
    while (++lFrame < m_length)
        if (!(m_psnData->index.index2Ptr()[lFrame].size & 0xE0000000))
            return lFrame;
    return -1;
}

long CMxStreamReader::nearestKeyFrame(long lFrame)
{
    if (m_sampsize)    return lFrame;
    
    if (isKeyFrame(lFrame))
        return lFrame;
    
    long lprev = prevKeyFrame(lFrame);
    
    if (lprev < 0)
        return 0;
    else
        return lprev;
}

uint CMxStreamReader::getFrameType(long lFrame)
{
    if (m_sampsize)    return 1;
    if (lFrame < 0 || lFrame >= m_length)
        return false;
    static uint32 frametype[] = { MxFrameType_I,MxFrameType_B,MxFrameType_P };
    int idx = ((uint32)(m_psnData->index.index2Ptr()[lFrame].size) >> 29);
    return frametype[idx];
}


long CMxStreamReader::read2(int64 start, long lSamples, uint8* lpBuffer, long cbBuffer, long *plBytes, long *plSamples, uint32* ftype, int bSeek, bool bFastRead, int nIoID, VXFEXTINFO* extinfo)
{
    long lActual;
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
        int64 byte_off = (__int64)start * m_sampsize;
        int64 bytecnt;
        int64 actual_bytes = 0;
        int64 block_pos;
        
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

long  CMxStreamReader::read(int64 start, long lSamples, uint8* lpBuffer, long cbBuffer, long *plBytes, long *plSamples, uint32* ftype, int bSeek, bool bFastRead, int nIoID)
{
    VXFEXTINFO extinfo = { 0 };
    return read2(start, lSamples, lpBuffer, cbBuffer, plBytes, plSamples, ftype, bSeek, bFastRead, nIoID, &extinfo);
}

LONG CMxStreamReader::queryInterfaceDelgate(long iid, void** ppv)
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

CMxVideoStreamReader::CMxVideoStreamReader(CMxDemuxer* demux, CAVIStreamNode* node, int idx)
: CMxStreamReader(demux, node, idx)
{
    m_sampsize = 0;
}

LONG CMxVideoStreamReader::NonDelegatingQueryInterface(LONG iid, void** ppv)
{
    if (iid == LIID_IVxVideoReadStream)
    {
        //return GetVxInterface((MxVideoStreamReader*)this, ppv);
        *ppv = (MxVideoStreamReader*)this;
        return 0;
    }
    else
    {
        return CMxStreamReader::queryInterface(iid, ppv);
    }
}

CMxAudioStreamReader::CMxAudioStreamReader(CMxStreamReader* demux, CAVIStreamNode* node, int idx)
: CMxStreamReader(demux, node, idx)
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
        return CMxStreamReader::NonDelegatingQueryInterface(iid, ppv);
    }
}
