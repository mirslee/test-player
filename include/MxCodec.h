
#ifndef __MXCODEC_H__
#define __MXCODEC_H__

#include "MxObject.h"
#include "MxTypes.h"

enum VXOBJSUBTYPE
{
	vxSubtype_empty = 0,

	vxSourceSub_file = 'file',
	vxSourceSub_dvd = 'dvd ',
	vxSourceSub_bdr = 'bdr ',
	vxSourceSub_ftp = 'ftp ',
	vxSourceSub_http = 'http',
	vxSourceSub_smb = 'smb ',
	vxSourceSub_mms = 'mms ',
	vxSourceSub_rtp = 'rtp ',
	vxSourceSub_rtcp = 'rtcp',
	vxSourceSub_blued = 'ksdb',//win:bdsk:e:\xxxx\xxxx.mxf,osx:bdsk:/Volumes/sss/xxxx/xxxx.mxf
	vxSourceSub_RAM = 'dmar',//win:ramd:e:\xxxx\xxxx.mxf,osx:ramd:/Volumes/sss/xxxx/xxxx.mxf
	vxSourceSub_M3U8 = 'm3u8',//m3u8 file

	vxUnparkSub_AVI = 'iva.',//AVI File
	vxUnparkSub_SAVI = 'ivas',//AVI File
	vxUnparkSub_AMV = 'vma.',//AMV File
	vxUnparkSub_MPG = 'epm.',//mpeg file
	vxUnparkSub_PS = 'gpm.',//ps mpeg file
	vxUnparkSub_MPEG = 'gepm',
	vxUnparkSub_TS = ' st.',//ts mpeg file
	vxUnparkSub_MTS = 'stm.',
	vxUnparkSub_M2T = 't2m.',
	vxUnparkSub_MPV = 'vpm.',//mpeg video es file
	vxUnparkSub_MPA = 'apm.',//mpeg audio es file
	vxUnparkSub_M2P = 'p2m.',
	vxUnparkSub_MKV = 'vkm.',
	vxUnparkSub_VOB = 'bov.',
	vxUnparkSub_DVD = 'dvd ',//dvd 
	vxUnparkSub_VCD = 'vcd ',//vcd
	vxUnparkSub_SVCD = 'svcd',//super vcd
	vxUnparkSub_ASF = 'fsa.',//windows media
	vxUnparkSub_WMV = 'vmw.',//windows media
	vxUnparkSub_WMA = 'amw.',
	vxUnparkSub_3GP = 'pg3.',
	vxUnparkSub_FLV = 'vlf.',
	vxUnparkSub_F4V = 'v4f.',
	vxUnparkSub_FHV = 'vhf.',
	vxUnparkSub_SWF = 'fws.',
	vxUnparkSub_WAV = 'vaw.',//wav file
	vxUnparkSub_W64 = '46w.',//wav file
	vxUnparkSub_BWF = 'fwb.',//bwf file
	vxUnparkSub_AIFF = 'fia.',//AIFF file
	vxUnparkSub_RM = 'mr.',//real media file(rm)
	vxUnparkSub_RMVB = 'vmr.',//real media vbv file(rmvb)
	vxUnparkSub_MOV = 'vom.',//apple Quick Time file
	vxUnparkSub_M1V = 'v1m.',//mpeg1 es file
	vxUnparkSub_M2V = 'v2m.',//mpeg2 es file
	vxUnparkSub_MP2 = '2pm.',//mp2 audio file
	vxUnparkSub_MP3 = '3pm.',//mp3 audio file
	vxUnparkSub_MP4 = '4pm.',//Mpeg4 file, ISMA stream format
	vxUnparkSub_MP4V = 'v4pm',
	vxUnparkSub_M4A = 'a4m.',
	vxUnparkSub_264 = '462.',
	vxUnparkSub_H264 = '62h.',
	vxUnparkSub_MXF = 'fxm.',//Material eXchange Format file
	vxUnparkSub_GXF = 'fxg.',//General Exchange Format file
	vxUnparkSub_AC3 = '3ca.',
	vxUnparkSub_DTS = 'std.',
	vxUnparkSub_AAC = 'caa.',
	vxUnparkSub_RAWDV = 'vd.',
	vxUnparkSub_YUV = 'vuy.',
	vxUnparkSub_AMT = 'amt ',
	vxUnparkSub_DFF = 'ffd.',
	vxUnparkSub_DSF = 'fsd.',
	vxUnparkSub_SUBTITLE = 'stil',//Jetsen字幕文件
	vxUnparkSub_XMLTITLE = 'lmx.',//XML freecg字幕
	vxUnparkSub_SRT = 'trs.',//SRT字幕
	vxUnparkSub_ASS = 'ssa.',//SRT字幕
	vxUnparkSub_DPX = 'xpd.',//SRT字幕

	vxFormat_none = 'none',	//没有对应的视音频流

	vxFormat_MPEG1 = 'mp1v',
	vxFormat_MPEG1_X = 'm1vx',	// JetSen mpeg1
	vxFormat_MPEG2 = 'mp2v',
	vxFormat_MPEG2_X = 'm2vx',	// JetSen mpeg2
	vxFormat_MPEG2_I = 'mp2I',
	vxFormat_MPEG2_I_X = 'm2Ix', // JetSen mpeg2 I
	vxFormat_MIMX = 'mimx',
	vxFormat_MIMX_X = 'mixx',   // JetSen mpeg IMX
	vxFormat_MPEG4 = 'mp4v',
	vxFormat_MP42 = '24PM',
	vxFormat_MP43 = '34PM',
	vxFormat_MPEG4_RAW = 'm4ra',
	vxFormat_H263 = '362h',
	vxFormat_H264 = '462h',
	vxFormat_H264P = '462e',//1/4原始分辨率压缩
	vxFormat_X264 = '462x',
	vxFormat_X265 = '562x',
	vxFormat_XAVC = 'cvax',//xavc
	vxFormat_XAVC10 = 'tvax',//xavc10Bit
	vxFormat_XAVC8 = 'evax',//xavc8Bit
	vxFormat_XAVC_MC = 'cvam',//MC XAVC Encoder
	vxFormat_dsxhw264 = '462m',
	vxFormat_hevc = '562h',//HEVC or H265
	vxFormat_dvsd = 'dsvd',
	vxFormat_dv25 = '52vd',//dvcpro25
	vxFormat_dv50 = '05vd',//dvcpro50
	vxFormat_dvh1 = '1hvd',//dvcpro100
	vxFormat_dvh5 = 'dvh5',//apple dvcpro100
	vxFormat_dvh6 = 'dvh6',//apple dvcpro100
	vxFormat_apdv = 'apdv',//quicktime dv
	vxFormat_ap4x = 'ap4x',//Apple ProRes 4444 XQ
	vxFormat_ap4h = 'ap4h',//Apple ProRes 4444
	vxFormat_apch = 'apch',//Apple ProRes 422 HQ
	vxFormat_apcn = 'apcn',//Apple ProRes 422
	vxFormat_apcs = 'apcs',//Apple ProRes 422 LT
	vxFormat_apco = 'apco',//Apple ProRes 422 Proxy
	vxFormat_apchex = 'Jtch',//Apple ProRes 422 HQ Jetsen优化
	vxFormat_apcnex = 'Jtcn',//Apple ProRes 422 Jetsen优化
	vxFormat_DNxDV = 'AVdv',//AVID DV codec
	vxFormat_DNxHD = 'AVdn',//AVID DNxHD codec
	vxFormat_DNxHDEX = 'Jtdn',//AVID DNxHD codec Jetsen优化
	vxFormat_DNxHR = 'AVdh',//AVID DNxHR codec
	vxFormat_mjpg = 'gpjm',//motion-jpeg
	vxFormat_mjp2k = '2pjm',//motion-JPEG2000
	vxFormat_AVC_I = '462p',
	vxFormat_wmv1 = '1VMW',
	vxFormat_wmv2 = '2VMW',//Window Media Video 8
	vxFormat_wmv3 = '3VMW',//Window Media Video 9
	vxFormat_pwmv = 'vmwp',//Window Media Video 9
	vxFormat_vc1 = '1CVW',//Window Media VC-1
	vxFormat_bid = ' bid',
	vxFormat_DivX5 = '05xd',
	vxFormat_flv1 = '1vlf',
	vxFormat_vp6f = 'f6pv',
	vxFormat_yuy2 = '2yuy',
	vxFormat_uyvy = 'yvyu',
	vxFormat_ap208 = 'v208',//Apple Component Y'CbCr 8-bit 4:2:2
	vxFormat_ap210 = 'v210',//Apple Component Y'CbCr 10-bit 4:2:2
	vxFormat_rgb32 = 'xbgr',
	vxFormat_rgb24 = ' bgr',
	vxFormat_dpx = 'xpdm',
	vxFormat_AMT = ' tma',
	vxFormat_AVS = ' sva',
	vxFormat_CAVS = 'svac',
	vxFormat_SWFD = 'dfws',
	vxFormat_HQX = 'XQHC',//Canopus HQX
	vxFormat_LLC = 'CLLC',//Canopus Lossless Codec
	vxFormat_UVC = 'CVUC',//Canopus HQ/HQA

	vxFormat_PCM = ' mcp',
	vxFormat_PCMF_LE = 'elfp',
	vxFormat_PCMF_BE = 'ebfp',
	vxFormat_BPCM = 'MCPB',
	vxFormat_PCM2 = '2mcp',
	vxFormat_AES3 = '3SEA',//使用MainConcept enc_pcm
	vxFormat_MPA = ' apm',
	vxFormat_MPA2 = '2apm',
	vxFormat_MP3 = ' 3pm',
	vxFormat_AC3 = ' 3ca',
	vxFormat_EAC3 = '3cae',
	vxFormat_DTS = ' std',
	vxFormat_AAC = 'caaf',
	vxFormat_AAC2 = 'caaj',
	vxFormat_DPCM = 'mcpd',//dv数据内嵌音频
	vxFormat_wma1 = 0x0160, //Window Media Audio 9
	vxFormat_wma2 = 0x0161,
	vxFormat_wma3 = 0x0162,
	vxFormat_wmaPro = 'PAMW',
	vxFormat_pwma = 'amwp',//Window Media Audio 9
	vxFormat_amrn = 'nrma',
	vxFormat_amrw = 'wrma',
	vxFormat_TrueHD = 'dhet',
	vxFormat_VorBis = 'oVsm',
	vxFormat_s48 = '84s.',
	vxFormat_dff = ' FFD',
	vxFormat_dsf = ' FSD',

	vxFormat_pgst = 'pgst',
	vxFormat_dvdt = 'dvdt',
	vxFormat_dvbt = 'dvbt',
	vxFormat_txtt = 'txtt',
	vxFormat_bxml = '1lmx',//B类字幕

	vxSystemClock_mm = 'mmsm',	//windows 多媒体时钟
	vxSystemClock_dsx = 'mxsd',	//dsx时钟

	vxVideoLive_nod = 'adon',//没有视频预监窗口
	vxVideoLive_dx9 = '19xd',
	vxVideoLive_gl = 'lgpo',
	vxVideoLive_gl2 = '2gpo',
	vxVideoLive_dd = 'ddxd',

	vxVidInput_dsx = 'vxsd',
	vxVidInput_KS = 'sksm',
	vxVidInput_CMIO = 'oimc',

	vxAudInput_dsx = 'axsd',
	vxAudInput_KS = 'sksm',
	vxAudInput_CA = 'uatq',
	vxAudInput_WAS = 'sawm',
	vxAudInput_ASIO = 'oisa',

	vxVidOutput_over = 'revo',
	vxVidOutput_dsx = 'vxsd',
	vxVidOutput_CMIO = 'oimc',
	vxVidOutput_mt = 'rtnm',

	vxAudOutput_dsx = 'axsd',
	vxAudOutput_DS = 'sdsm',
	vxAudOutput_KS = 'sksm',
	vxAudOutput_WAS = 'sawm',
	vxAudOutput_ASIO = 'oisa',
	vxAudOutput_CA = 'uatq',
	vxAudOutput_CMIO = 'oimc',
};

#define MX_MAXPATH	512

#define VX_MAX_PLANES	6				// Number of planes in a picture

struct MxMuxerInfo{
	DWORD		fourcc;				//复合格式
	WORD		pid;				//复合标识
	BYTE		novideo;			//0-->表示包含视频,非零模板不需要压缩视频
	BYTE		noaudio;			//0-->表示包含音频,非零模板不需要压缩音频
	DWORD		bitrate;			//复合码率
	void*		pExtraData;			//附加参数数据，必须用_vxmalloc来分配
	DWORD		dwExtraSize;		//附加参数大小和类型, 高8位为类型
	DWORD		pmtid;				//TS中PMT ID
	DWORD		tsid;				//TS中transport_stream ID
	DWORD		reserved;			//保留数据，可存储一些附加信息。在MXF解复用中reserved!=0为Opatom,否则是Op1a

	void*			metadata;			//元数据信息
	int				metadatasize;		//元数据信息大小
	__int64			start_timecode;		//以帧为单位
	DWORD			reserved2[62];
};
typedef struct
{
	DWORD			fmt;
	bool			topfirst;
	int				planes;		//平面数
	BYTE*			buf[VX_MAX_PLANES];		//视频分量数据
	int				pitch[VX_MAX_PLANES];	//每行字节大小，每行一般为 64 字节对齐的
	int				fields;				//该帧持续的场数目
	bool			getpic;
	mxuvoidptr		reserved;			//保留数据，可存储一些附加信息。
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
	BYTE*		buf[VX_MAX_PLANES];		//视频分量数据
	int			pitch[VX_MAX_PLANES];	//每行字节大小，每行一般为 64 字节对齐的
} SUBTITLERECT;

typedef struct
{
	uint			fmt;
	uint			start_display_time; //relative to packet pts, in ms
	uint			end_display_time;	// relative to packet pts, in ms
	int				num_rects;
	SUBTITLERECT	rects[8];
	uint			reserved;			//保留数据，可存储一些附加信息。
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
struct MxFastIO;
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
	virtual void AddFastIO(MxFastIORead*) = 0;
	virtual void RemoveFastIO(int nFastIoID,void* srcp) = 0;
	virtual bool IsFastIO(int nFastIoID) = 0;
};

enum storagetype { st_harddisk, st_removable, st_cdrom, st_netshare, st_ftp, st_http };
struct MxSource: public MxFastIO
{
	virtual long GetType() = 0;
	virtual long GetExtra(MxSource**extra) = 0;
	virtual long GetExtra(const char*privatefile, MxSource**extra) = 0;
	virtual void GetFileName(MxPath* mxPath) = 0;
	virtual mxuvoidptr GetFileId() = 0;
	virtual storagetype GetStorageType() = 0;

	virtual int64 GetPosition() = 0;
	virtual int64 GetSize() = 0;
	virtual int64 Seek(int64 pos) = 0;
	virtual long Read(BYTE* buf, long size, bool bSeek = false) = 0;
	virtual long FastRead(int64 pos, BYTE* buf, long size, int stream, bool bSeek, int nIoID) = 0;
	virtual void InfoEnd() = 0;
	virtual void Refresh() = 0;
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
	virtual mxuvoidptr GetFileId() = 0;
	virtual long GetDemuxer(MxDemuxer** demux) = 0;
	virtual long GetSource(MxSource** source) = 0;
	virtual uint GetStreamType() = 0;
	virtual void AddFastIO(MxFastIORead*) = 0;
	virtual void Clear() = 0;

	virtual long NearestKeyFrame(long llFrame) = 0;
	virtual long PrevKeyFrame(long llFrame) = 0;
	virtual long NextKeyFrame(long llFrame) = 0;
	virtual uint GetFrameType(long llFrame) = 0;
	virtual long Read(int64 start, long lSamples, BYTE* lpBuffer, long cbBuffer, long *plBytes, long *plSamples, uint* ftype, int bSeek = 0, bool bFastRead = false, int nIoID = 0) = 0;//音频解码时数据为不固定帧样时ftype返回的是有效样本的偏移,mode(0==>play,1==>seek,2==>fastseek,3==>fastplay)

};
struct MxStreamReader2 : public MxObject
{
	//音频解码时数据为不固定帧样时ftype返回的是有效样本的偏移,mode(0==>play,1==>seek,2==>fastseek,3==>fastplay)]
	//extinfo==NULL不需要读取附加信息，extinfo非空是输入输出参数，VXFEXTINFO中infotype输入需要的信息FEEXTINFO_*的合集，在VXFEXTINFO中infotype输出根据文件中信息返回FEEXTINFO_*的合集。
	virtual LONG  __stdcall Read2(__int64 start, LONG lSamples, PBYTE lpBuffer, LONG cbBuffer,LONG *plBytes, LONG *plSamples,DWORD* ftype,int bSeek = 0,bool bFastRead = FALSE,int nIoID = 0,VXFEXTINFO* extinfo = NULL) = 0;
};
struct MxVideoInfo 
{
	DWORD			fourcc;				//视频压缩格式
	DWORD			pid;				//视频标识
	DWORD			dectype;			//解码方式 1->帧间无关，0->帧间相关(IBP)，必须在下一帧数据输给解码器才返回上一帧的数据，2->帧间相关,但是输给解码器哪一帧就能等到哪一帧(IP);3->得到一组压缩数据包含多帧
	DWORD			frames;				//视频长度
	int				picture_width;		//图像大小
	int				picture_height;		//
	int				display_width;		//显示大小
	int				display_height;
	int				display_xoffset;
	int				display_yoffset;
	short			depth;				//ｂｉｔ大小
	short			colorimetry;		//颜色空间BT.601,BT.709.... VXSURFACE_COLORIMETRY
	int				rate;			//帧速率，通过两个参数描述，=rate/scale
	int				scale;
	DWORD			bitrate;			//码率,Bits/s,最高位指示是否为变码率
	WORD			profile;			//档次, MPEG2有效
	WORD			level;				//级次, MPEG2有效
	int				chromafmt;			//色差格式 chroma_format 1=420, 2=422, 3=444, 4=411
	int				topfirst;			//0->场基-底场优先，1->场基-顶场优先，2->帧基，3->帧场自适应
	DWORD			aspect;				//显示比例 4:3==>0x00030004 16:9==>0x00090010,高4位存储AFD值(0xF0000000)，数值参照SMPTE 2016-1
	int				quality;			//压缩质量,在压缩时范围为(1-100) 0==>自动
	DWORD			suggestedsize;		//
	DWORD			dwKeyframeSpace;	//关键帧间隔,对于MPEG和H264文件为低8位为GOP长度,8-15位P帧间B帧数,H264 16-23位为参考帧个数,24-31为IDR帧间隔
	void*			pExtraData;			//附加参数数据，必须用_vxmalloc来分配
	DWORD			dwExtraSize;		//附加参数大小和类型, 高8位为类型
	DWORD			decoutfmt;			//解码后的格式［ＩＮ，ＯＵＴ］
	BYTE			vbr;
	BYTE			twopass;			//在解码时无效
	WORD			selfadapting;		//自适应参数 VXVIDEOSA_XXX
	DWORD			max_bitrate;		//
	DWORD			reserved[2];		//保留数据，可存储一些附加信息。

	void*			metadata;			//元数据信息
	int				metadatasize;		//元数据信息大小
	DWORD			muxtype;
	DWORD			reserved2[47];		//保留数据，可存储一些附加信息。
										// 0、1：标记画面旋转角度；// 2：标记是否获取ScanType,有值代表未获取
										// 3: 标记是否使用文件中的aspect
	DWORD			trimin, trimout;		//视频有效帧出入点,前包后不包；trimin==triimout无效，目前仅在movReferenc中可能有效
	DWORD			reserved3[14];		//保留数据，可存储一些附加信息。
};
struct MxAudioInfo
{
	DWORD			fourcc;				//音频压缩格式
	DWORD			pid;				//音频标识
	DWORD			dectype;			//0-->以样本定位解码（可以是连续的pcm数据，也可以是每个帧包含不固定样本数），1-->以帧定位解码（每个帧包含固定的样本数），2-->输出的是数据流
	__int64			samples;			//音频长度
	DWORD			samplesperframe;	//音频帧样本数，当dectype=1时为最大帧样本大小
	DWORD			framesize;			//基本单元大小。对于MPEG音频，为音频帧大小
	DWORD			freq;				//采样率 
	int				channels;			//通道数
	int				bitpersample;		//样本精度
	int				blockalign;			//
	DWORD			bitrate;			//码率,Bits/s
	int				quality;			//压缩质量,在压缩时范围为(1-100) 0==>自动
	DWORD			suggestedsize;		//
	void*			pExtraData;			//附加参数数据，必须用_vxmalloc来分配
	DWORD			dwExtraSize;		//附加参数大小和类型
	DWORD			rfourcc;			//真实的音频压缩格式
	DWORD			rfreq;				//真实的采样率 
	WORD			selfadapting;		//自适应参数 VXAUDIOSA_XXX
	BYTE			reservedByte;
	BYTE			splitchs;			//在复用时复用器会根据自己需要修改
	bool			vbr;
	DWORD			reserved[2];		//保留数据，可存储一些附加信息。

	void*			metadata;			//元数据信息
	int				metadatasize;		//元数据信息大小
	DWORD			muxtype;
	DWORD			reserved2[47];		//保留数据，可存储一些附加信息。reserved2[0]已经被占用了，用来存储合成时是否启用音量大小设定标志。
	__int64			trimin, trimout;		//音频有效样本出入点,前包后不包；trimin==triimout无效，目前仅在movReferenc中可能有效
	DWORD			reserved3[12];		//保留数据，可存储一些附加信息。reserved2[0]已经被占用了，用来存储合成时是否启用音量大小设定标志。
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
