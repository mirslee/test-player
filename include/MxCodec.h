
#ifndef __MXCODEC_H__
#define __MXCODEC_H__

#include "MxObject.h"
#include "MxTypes.h"
#include "MxMediaDefine.h"

struct MxFastIO;
struct MxFastIORead;
struct MxSource;

struct MxDemuxer;
struct MxStreamReader;
struct MxStreamReader2;
struct MxVideoStreamReader;
struct MxAudioStreamReader;
struct MxSubtitleStreamReader;
struct MxVideoDecoder;
struct MxAudioDecoder;

struct MxMuxer;
struct MxStreamWriter;
struct MxVideoEncoder;
struct MxAudioEncoder;

typedef struct
{
	uint32			fmt;
	bool			topfirst;
	int				planes;		//ƽ����
	uint8*			buf[MX_MAXPLANES];		//��Ƶ��������
	int				pitch[MX_MAXPLANES];	//ÿ���ֽڴ�С��ÿ��һ��Ϊ 64 �ֽڶ����
	int				fields;				//��֡�����ĳ���Ŀ
	bool			getpic;
	mxuvoidptr		reserved;			//�������ݣ��ɴ洢һЩ������Ϣ��
}DECOUT_IMAGE;

typedef struct
{
	int samples;
	int channels;
	float* buf[MX_MAXAUDIOCHANNELS];
	uint32 reserved[2];		//�������ݣ��ɴ洢һЩ������Ϣ��
}DECOUT_SAMPLES;

typedef struct
{
	int			x, y;					///< top left corner  of pict, undefined when pict is not set
	int			w, h;					///< width,height of pict, undefined when pict is not set
	int			nb_colors;				///< number of colors in pict, undefined when pict is not set

	int			planes;					//ƽ����
	BYTE*		buf[MX_MAXPLANES];		//��Ƶ��������
	int			pitch[MX_MAXPLANES];	//ÿ���ֽڴ�С��ÿ��һ��Ϊ 64 �ֽڶ����
} SUBTITLERECT;

typedef struct
{
	uint			fmt;
	uint			start_display_time; //relative to packet pts, in ms
	uint			end_display_time;	// relative to packet pts, in ms
	int				num_rects;
	SUBTITLERECT	rects[8];
	uint			reserved;			//�������ݣ��ɴ洢һЩ������Ϣ��
}DECOUT_SUBTITLE;

struct FASTRDPARAM {
	void* fastio;
	void* srcp;
	int64 pos;
	long requestbytes;
	void* buffer;
#ifdef _WIN32
	long reads;
#elif __APPLE__
	/*ByteCount*/ uint64 reads;
#else
	int reads;
#endif
	int idxwait;
	uint8 usrdata[1024];
};
typedef void (__cdecl *asynccallback)(FASTRDPARAM*);
typedef long (__cdecl *fastioread)(FASTRDPARAM*, asynccallback acb);
enum fastioreadtype { fastio_sequential, fastio_random };
struct MxFastIO;
struct MxFastIORead : public MxObject
{
	virtual int   GetId() = 0;
virtual bool  InitFile(void* srcp, MxFastIO* vxdemul, mxuvoidptr fid,uint sectorsize, fastioread ioread,bool asyncrd = false) = 0;
virtual void   UninitFile(bool bRemove = true) = 0;
virtual mxuvoidptr  GetFileId() = 0;
virtual long   Read(int stream, __int64 pos, BYTE* buf, long lBytes, fastioreadtype mode = fastio_random) = 0;
virtual long   DirectRead(__int64 pos, BYTE* buf, long lBytes) = 0;
virtual int    LockCached(int stream, __int64 pos) = 0;
virtual const BYTE*  GetCache(int idx, long& size) = 0;
virtual void    UnlockCached(int lockidx) = 0;

virtual long   GetBlockSize() = 0;
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
	virtual uint32 getType() = 0;
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

struct MxStreamReader;
struct MxDemuxer: public MxObject {
	virtual MxMuxerInfo* getFormat() = 0;
	virtual long GetSource(MxSource**) = 0;
	virtual long GetStreams() = 0;
	virtual long GetStream(uint fccType, long id, MxStreamReader**) = 0;
	virtual void Reset(float sec) = 0;
	virtual bool CanRefresh() = 0;
	virtual bool Refresh() = 0;
};

struct MxMuxer: public MxObject {
    
};

struct MxStreamReader: public MxObject {
	virtual mxuvoidptr getFileId() = 0;
	virtual long getDemuxer(MxDemuxer** demux) = 0;
	virtual long getSource(MxSource** source) = 0;
	virtual uint getStreamType() = 0;
	virtual void addFastIO(MxFastIORead*) = 0;
	virtual void clear() = 0;

	virtual long nearestKeyFrame(long llFrame) = 0;
	virtual long prevKeyFrame(long llFrame) = 0;
	virtual long nextKeyFrame(long llFrame) = 0;
	virtual uint getFrameType(long llFrame) = 0;
	virtual long read(int64 start, long lSamples, uint8* lpBuffer, long cbBuffer, long *plBytes, long *plSamples, uint* ftype, int bSeek = 0, bool bFastRead = false, int nIoID = 0) = 0;//��Ƶ����ʱ����Ϊ���̶�֡��ʱftype���ص�����Ч������ƫ��,mode(0==>play,1==>seek,2==>fastseek,3==>fastplay)

};

struct MxStreamReader2 : public MxObject
{
	//��Ƶ����ʱ����Ϊ���̶�֡��ʱftype���ص�����Ч������ƫ��,mode(0==>play,1==>seek,2==>fastseek,3==>fastplay)]
	//extinfo==NULL����Ҫ��ȡ������Ϣ��extinfo�ǿ����������������VXFEXTINFO��infotype������Ҫ����ϢFEEXTINFO_*�ĺϼ�����VXFEXTINFO��infotype��������ļ�����Ϣ����FEEXTINFO_*�ĺϼ���
	virtual long   Read2(int64 start, long lSamples, uint8* lpBuffer, long cbBuffer,long *plBytes, long *plSamples,uint32* ftype,int bSeek = 0,bool bFastRead = false,int nIoID = 0,VXFEXTINFO* extinfo = nullptr) = 0;
};

struct MxSubtitleInfo
{
};

struct MxVideoStreamReader: public MxObject 
{
	virtual MxVideoInfo* GetFormat() = 0;
};

struct MxAudioStreamReader : public MxObject
{
	virtual MxAudioInfo* GetFormat() = 0;
};

struct MxSubtitleStreamReader : public MxObject
{
	virtual MxSubtitleInfo* GetFormat() = 0;
};

struct MxStreamWriter: public MxObject {
    
};

typedef void(*DECOUT)(void* param, __int64 frame, DECOUT_IMAGE* image);
struct MxVideoDecoder: public MxObject {
	virtual void reset() = 0;
	virtual bool supportAsync(DECOUT decout, void*p) = 0;
	virtual long decodeFrame(BYTE* inbuf, long insize, DECOUT_IMAGE* img, uint flag) = 0;//flag&0x80000000-->ֱ�ӵõ���ǰ����ͼ flag&0x8000-->ֻ�ǵȵ�ͼ�����Ϣ flag&0x1-->keyframe
	virtual long decodeFrame(BYTE* inbuf, long insize, long frame) = 0;//�첽���룬ֻ��֡��ѹ����������Ч
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
