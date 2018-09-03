
#ifndef __MXCODEC_H__
#define __MXCODEC_H__

#include "MxObject.h"
#include "MxTypes.h"

#define MX_MAXPATH	512


typedef struct
{
	DWORD			fmt;
	VXBOOL			topfirst;
	int				planes;		//平面数
	PBYTE			buf[VX_MAX_PLANES];		//视频分量数据
	int				pitch[VX_MAX_PLANES];	//每行字节大小，每行一般为 64 字节对齐的
	int				fields;				//该帧持续的场数目
	VXBOOL			getpic;
	vxuintptr		reserved;			//保留数据，可存储一些附加信息。
}DECOUT_IMAGE;

#define MAX_AUD_CHANNELS	64
typedef struct
{
	int samples;
	int channels;
	float* buf[MAX_AUD_CHANNELS];
	DWORD reserved[2];		//保留数据，可存储一些附加信息。
}DECOUT_SAMPLES;


typedef struct
{
	int			x, y;					///< top left corner  of pict, undefined when pict is not set
	int			w, h;					///< width,height of pict, undefined when pict is not set
	int			nb_colors;				///< number of colors in pict, undefined when pict is not set

	int			planes;					//平面数
	PBYTE		buf[VX_MAX_PLANES];		//视频分量数据
	int			pitch[VX_MAX_PLANES];	//每行字节大小，每行一般为 64 字节对齐的
} SUBTITLERECT;

typedef struct
{
	DWORD			fmt;
	DWORD			start_display_time; //relative to packet pts, in ms 
	DWORD			end_display_time;	// relative to packet pts, in ms
	int				num_rects;
	SUBTITLERECT	rects[8];
	DWORD			reserved;			//保留数据，可存储一些附加信息。
}DECOUT_SUBTITLE;

struct MxPath
{
	uint reffile;				
	char szPath[MX_MAXPATH];		
	char szExtraPath[MX_MAXPATH];
};

struct MediaFormat 
{
};

struct FASTRDPARAM {
	void* fastio;
	void* srcp;
	__int64 pos;
	LONG requestbytes;
	void* buffer;
#ifdef _WIN32
	LONG reads;
#elif __APPLE__
	ByteCount reads;
#else
	int reads;
#endif
	int idxwait;
	BYTE usrdata[1024];
};
typedef void(__cdecl *asynccallback)(FASTRDPARAM*);
typedef LONG(__cdecl *fastioread)(FASTRDPARAM*, asynccallback acb);
enum fastioreadtype { fastio_sequential, fastio_random };
struct MxFastIORead : public MxObject
{
	virtual int   __stdcall GetId() = 0;
virtual bool  __stdcall InitFile(void* srcp, MxFastIO* vxdemul, mxuvoidptr fid,uint sectorsize, fastioread ioread,bool asyncrd = false) = 0;
virtual void  __stdcall UninitFile(bool bRemove = true) = 0;
virtual mxuvoidptr __stdcall GetFileId() = 0;
virtual long  __stdcall Read(int stream, __int64 pos, BYTE* buf, long lBytes, fastioreadtype mode = fastio_random) = 0;
virtual long  __stdcall DirectRead(__int64 pos, BYTE* buf, long lBytes) = 0;
virtual int   __stdcall LockCached(int stream, __int64 pos) = 0;
virtual const BYTE* __stdcall GetCache(int idx, long& size) = 0;
virtual void   __stdcall UnlockCached(int lockidx) = 0;

virtual long  __stdcall GetBlockSize() = 0;
};

struct MxFastIO : public MxObject
{
	virtual void addFastIO(MxFastIORead*) = 0;
	virtual void removeFastIO(int nFastIoID,void* srcp) = 0;
	virtual bool isFastIO(int nFastIoID) = 0;
};

enum storagetype { st_harddisk, st_removable, st_cdrom, st_netshare, st_ftp, st_http };
struct MxSource: public MxFastIO
{
	virtual long getType() = 0;
	virtual long getExtra(MxSource**extra) = 0;
	virtual long getExtra(const char*privatefile, MxSource**extra) = 0;
	virtual void getFileName(MxPath* mxPath) = 0;
	virtual mxuvoidptr getFileId() = 0;
	virtual storagetype getStorageType() = 0;

	virtual int64 getPosition() = 0;
	virtual int64 getSize() = 0;
	virtual int64 seek(int64 pos) = 0;
	virtual long read(BYTE* buf, long size, bool bSeek = false) = 0;
	virtual long fastRead(int64 pos, BYTE* buf, long size, int stream, bool bSeek, int nIoID) = 0;
	virtual void infoEnd() = 0;
	virtual void refresh() = 0;
};

struct MxDemuxer: public MxObject {
	virtual MediaFormat* getFormat() = 0;
	virtual long getSource() = 0;
	virtual long getStreams() = 0;
	virtual long getStream(uint fccType, long id, MxStreamReader**) = 0;
	virtual void reset(float sec) = 0;
	virtual bool canRefresh() = 0;
	virtual bool refresh() = 0;
};
struct MxMuxer: public MxObject {
    
};
struct MxStreamReader: public MxObject {
	virtual mxuvoidptr getFileId() = 0;
	virtual long getDemuxer(MxDemuxer** demux) = 0;
	virtual long getSource() = 0;
	virtual uint getStreamType() = 0;
	virtual void addFastIO(MxFastIORead*) = 0;
	virtual void clear() = 0;

	virtual long nearestKeyFrame(long llFrame) = 0;
	virtual long preKeyFrame(long llFrame) = 0;
	virtual long nextKeyFrame(long llFrame) = 0;
	virtual uint getFrameType(long llFrame) = 0;
	virtual long read(int64 start, long lSamples, BYTE* lpBuffer, long cbBuffer, long *plBytes, long *plSamples, uint* ftype, int bSeek = 0, bool bFastRead = false, int nIoID = 0) = 0;//音频解码时数据为不固定帧样时ftype返回的是有效样本的偏移,mode(0==>play,1==>seek,2==>fastseek,3==>fastplay)

};
struct MxVideoInfo 
{
};
struct MxAudioInfo
{
};
struct MxSubtitleInfo
{
};
struct MxVideoStreamReader: public MxObject 
{
	virtual MxVideoInfo* getFormat() = 0;
};
struct MxAudioStreamReader : public MxObject
{
	virtual MxAudioInfo* getFormat() = 0;
};
struct MxSubtitleStreamReader : public MxObject
{
	virtual MxSubtitleInfo* getFormat() = 0;
};
struct MxStreamWriter: public MxObject {
    
};

typedef void(*DECOUT)(void* param, __int64 frame, DECOUT_IMAGE* image);
struct MxVideoDecoder: public MxObject {
	virtual void reset() = 0;
	virtual bool supportAsync(DECOUT decout, void*p) = 0;
	virtual long decodeFrame(BYTE* inbuf, long insize, DECOUT_IMAGE* img, uint flag) = 0;//flag&0x80000000-->直接得到当前解码图 flag&0x8000-->只是等到图像的信息 flag&0x1-->keyframe
	virtual long decodeFrame(BYTE* inbuf, long insize, long frame) = 0;//异步解码，只对帧内压缩的码流有效
};
struct MxVideoEncoder: public MxObject {
    
};
struct MxAudioDecoder: public MxObject {
	virtual void reset() = 0;
	virtual long decodeFrame(BYTE* inbuf, long insize, DECOUT_SAMPLES* samples) = 0;
};
struct MxAudioEncoder: public MxObject {
    
};


#endif //__MXCODEC_H__
