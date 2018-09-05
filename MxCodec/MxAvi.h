
#ifndef __MXAVI_H__
#define __MXAVI_H__

#include "MxTypes.h"
#include "MxMemory.h"
#include "MxMediaDefine.h"

#define AVI_INDEX_OF_INDEXES    0x00
#define AVI_INDEX_OF_CHUNKS        0x01
#define AVI_INDEX_IS_DATA        0x80
#define AVI_INDEX_2FIELD        0x01

typedef int64 QUADWORD;

struct _avisuperindex_entry {
    QUADWORD qwOffset;    // absolute file offset, offset 0 is
    DWORD dwSize;        // size of index chunk at this offset
    DWORD dwDurationl;    // time span in stream ticks
};

struct _avistdindex_entry
{
    DWORD dwOffset;        // qwBaseOffset + this is absolute file offset
    DWORD dwSize;        // bit 31 is set if this is NOT a keyframe
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
    WORD wLongsPerEntry;            // must be sizeof(aIndex[0])/sizeof(DWORD)
    BYTE bIndexSubType;                // must be 0
    BYTE bIndexType;                // must be AVI_INDEX_OF_CHUNKS
    DWORD nEntriesInUse;
    DWORD dwChunkId;
    QUADWORD qwBaseOffset;            // all dwOffsets in aIndex array are
    DWORD dwReserved3;                // must be 0
    _avistdindex_entry aIndex[];
} AVISTDINDEX, *PAVISTDINDEX;

typedef struct _avisuperindex_chunk {
    DWORD fcc;
    DWORD cb;                        // size of this structure
    WORD wLongsPerEntry;            // must be 4 (size of each entry in aIndex array)
    BYTE bIndexSubType;                // must be 0 or AVI_INDEX_2FIELD
    BYTE bIndexType;                // must be AVI_INDEX_OF_INDEXES
    DWORD nEntriesInUse;            // number of entries in aIndex array that are used
    DWORD dwChunkId;
    DWORD dwReserved[3];            // must be 0
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
    CAVIIndexChain(){
        head = tail = NULL;
        total_ents = 0;
    }
    virtual ~CAVIIndexChain(){
        delete_chain();
    }
    
public:
    int total_ents;
    CAVIIndexChainNode *head, *tail;
    
protected:
    void delete_chain(){
        CAVIIndexChainNode *aicn = head, *aicn2;
        while (aicn)
        {
            aicn2 = aicn->next;
            delete aicn;
            aicn = aicn2;
        }
        head = tail = NULL;
    }
public:
    bool add(AVIIndexEntry *avie){
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
    bool add(AVIIndexEntry2 *avie2){
        return add(avie2->ckid, avie2->pos, avie2->size & 0x3FFFFFFF, avie2->displayno, !!(avie2->size & 0x80000000), avie2->reserve);
    }
    bool add(DWORD ckid, int64 pos, long len, long displayno, bool is_keyframe, DWORD reserve = 0){
        if (displayno == -1)
            displayno = total_ents;
        if (!tail || !tail->add(ckid, pos, len, displayno, is_keyframe, reserve))
        {
            CAVIIndexChainNode *aicn = new CAVIIndexChainNode();
            
            if (tail) tail->next = aicn; else head = aicn;
            tail = aicn;
            
            if (tail->add(ckid, pos, len, displayno, is_keyframe, reserve))
            {
                ++total_ents;
                return TRUE;
            }
            
            return FALSE;
        }
        
        ++total_ents;
        
        return TRUE;
    }
    void put(AVIIndexEntry *avietbl){
        CAVIIndexChainNode *aicn = head;
        
        while (aicn)
        {
            aicn->put(avietbl);
            aicn = aicn->next;
        }
        
        delete_chain();
    }
    void put(AVIIndexEntry2 *avie2tbl){
        CAVIIndexChainNode *aicn = head;
        
        while (aicn)
        {
            aicn->put(avie2tbl);
            aicn = aicn->next;
        }
        
        delete_chain();
    }
    void put(AVIIndexEntry3 *avie3tbl, int64 offset){
        CAVIIndexChainNode *aicn = head;
        
        while (aicn)
        {
            aicn->put(avie3tbl, offset);
            aicn = aicn->next;
        }
        
        delete_chain();
    }
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
    CAVIIndex(){
        index = NULL;
        index2 = NULL;
        index3 = NULL;
        oldindex2s.SetMaxSize(8);
    }
    virtual ~CAVIIndex(){
        delete[] index;
        delete[] index2;
        delete[] index3;
        AVIIndexEntry2* oldinex2 = nullptr;
        while (oldindex2s.Pop(&oldinex2, false))
            delete[] oldinex2;
    }
    
    bool makeIndex(){
        AVIIndexEntry* newindex = new AVIIndexEntry[total_ents];
        put(newindex);
        AVIIndexEntry* oldindex = (AVIIndexEntry*)InterlockedExchangePointer((void**)&index, newindex);
        if (oldindex) delete oldindex;
        index_len = total_ents;
        return true;
    }
    bool makeIndex2(){
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
    bool makeIndex3(int64 offset){
        AVIIndexEntry3* newindex3 = new AVIIndexEntry3[total_ents];
        put(newindex3, offset);
        AVIIndexEntry3* oldindex3 = (AVIIndexEntry3*)InterlockedExchangePointer((void**)&index3, newindex3);
        if (oldindex3) delete oldindex3;
        index_len = total_ents;
        return true;
    }
    void clear(){
        delete_chain();
        delete[] index;
        delete[] index2;
        delete[] index3;
        index = nullptr;
        index2 = nullptr;
        index3 = nullptr;
        total_ents = 0;
    }
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
    CAVIStreamNode(DWORD fccType)
    {
        this->fccType = fccType;
        bytes = 0;
        strmid = 0;
        starttime = 0;
        priv_data = NULL;
        keyonly = TRUE;
        memset(&vinfo, 0, sizeof(MxVideoInfo));
    }
    virtual ~CAVIStreamNode(){
        if (fccType == MxStreamType_video)
        {
            if (vinfo.pExtraData) mx_free(vinfo.pExtraData);
            if (vinfo.metadata) mx_free(vinfo.metadata);
        }
        else if (fccType == MxStreamType_audio)
        {
            if (ainfo.pExtraData) mx_free(ainfo.pExtraData);
            if (ainfo.metadata) mx_free(ainfo.metadata);
        }
    }
    
public:
    ulong fccType;
    union
    {
        ulong fmtfcc;
        MxVideoInfo vinfo;
        MxAudioInfo ainfo;
    };
    CAVIIndex                index;
    int64                    bytes;
    int64                    length;
    int64                    frames;
    int                        strmid;
    int64                    starttime;
    void*                    priv_data;
    bool                    keyonly;
};

#endif /* __MXAVI_H__ */
