#ifndef __BAY_VXSYSDEF_H__
#define __BAY_VXSYSDEF_H__

#include "vxtypes.h"

typedef LONG VXRESULT;

#ifdef WIN32
#define MAX_VXPATH	512
#else	
#define MAX_VXPATH	512
#endif

#ifndef MAX_VXFNAME
#define MAX_VXFNAME 256
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable:1899)
#endif

enum VXOBJTYPE
{
	vxObjSource,	//数据源对象，如文件、网络流、板卡等
	vxObjSink,		//数据输出对象。
	vxObjUnpack,	//解包(解复用)对象
	vxObjPack,		//打包(复用)对象
	vxObjVidDecoder,//视频解码对象
	vxObjVidEncoder,//视频编码对象
	vxObjAudDecoder,//音频解码对象
	vxObjAudEncoder,//音频编码对象
	vxObjSubDecoder,//字幕解码对象

	vxObjVideoLive,	//视频预览设备

	vxObjVidInput,	//视频输入设备
	vxObjAudInput, //音频输入设备
	vxObjVidOutput,	//视频输出设备
	vxObjAudOutput, //音频输出设备

	vxObjMAX
};


enum VXOBJSUBTYPE
{
	vxSubtype_empty  = 0,

	vxSourceSub_file = 'file',
	vxSourceSub_dvd  = 'dvd ',
	vxSourceSub_bdr	 = 'bdr ',
	vxSourceSub_ftp  = 'ftp ',
	vxSourceSub_http = 'http',
    vxSourceSub_smb  = 'smb ',
	vxSourceSub_mms  = 'mms ',
	vxSourceSub_rtp  = 'rtp ',
	vxSourceSub_rtcp = 'rtcp',
	vxSourceSub_blued= 'ksdb',//win:bdsk:e:\xxxx\xxxx.mxf,osx:bdsk:/Volumes/sss/xxxx/xxxx.mxf
	vxSourceSub_RAM  = 'dmar',//win:ramd:e:\xxxx\xxxx.mxf,osx:ramd:/Volumes/sss/xxxx/xxxx.mxf
	vxSourceSub_M3U8 = 'm3u8',//m3u8 file

	vxUnparkSub_AVI	 = 'iva.',//AVI File
	vxUnparkSub_SAVI = 'ivas',//AVI File
	vxUnparkSub_AMV	 = 'vma.',//AMV File
	vxUnparkSub_MPG	 = 'epm.',//mpeg file
	vxUnparkSub_PS	 = 'gpm.',//ps mpeg file
	vxUnparkSub_MPEG = 'gepm',
	vxUnparkSub_TS	 = ' st.',//ts mpeg file
	vxUnparkSub_MTS  = 'stm.',
	vxUnparkSub_M2T  = 't2m.',
	vxUnparkSub_MPV	 = 'vpm.',//mpeg video es file
	vxUnparkSub_MPA	 = 'apm.',//mpeg audio es file
	vxUnparkSub_M2P  = 'p2m.',
	vxUnparkSub_MKV  = 'vkm.',
	vxUnparkSub_VOB  = 'bov.',
	vxUnparkSub_DVD	 = 'dvd ',//dvd 
	vxUnparkSub_VCD	 = 'vcd ',//vcd
	vxUnparkSub_SVCD = 'svcd',//super vcd
	vxUnparkSub_ASF	 = 'fsa.',//windows media
	vxUnparkSub_WMV	 = 'vmw.',//windows media
	vxUnparkSub_WMA  = 'amw.',
	vxUnparkSub_3GP  = 'pg3.',
	vxUnparkSub_FLV  = 'vlf.',
	vxUnparkSub_F4V  = 'v4f.',
	vxUnparkSub_FHV  = 'vhf.',
	vxUnparkSub_SWF  = 'fws.',
	vxUnparkSub_WAV	 = 'vaw.',//wav file
	vxUnparkSub_W64	 = '46w.',//wav file
	vxUnparkSub_BWF  = 'fwb.',//bwf file
	vxUnparkSub_AIFF = 'fia.',//AIFF file
	vxUnparkSub_RM	 = 'mr.',//real media file(rm)
	vxUnparkSub_RMVB = 'vmr.',//real media vbv file(rmvb)
	vxUnparkSub_MOV	 = 'vom.',//apple Quick Time file
	vxUnparkSub_M1V	 = 'v1m.',//mpeg1 es file
	vxUnparkSub_M2V	 = 'v2m.',//mpeg2 es file
	vxUnparkSub_MP2	 = '2pm.',//mp2 audio file
	vxUnparkSub_MP3	 = '3pm.',//mp3 audio file
	vxUnparkSub_MP4	 = '4pm.',//Mpeg4 file, ISMA stream format
	vxUnparkSub_MP4V = 'v4pm',
	vxUnparkSub_M4A	 = 'a4m.',
	vxUnparkSub_264  = '462.',
	vxUnparkSub_H264 = '62h.',
	vxUnparkSub_MXF	 = 'fxm.',//Material eXchange Format file
	vxUnparkSub_GXF	 = 'fxg.',//General Exchange Format file
	vxUnparkSub_AC3	 = '3ca.',
	vxUnparkSub_DTS	 = 'std.',
	vxUnparkSub_AAC	 = 'caa.',
	vxUnparkSub_RAWDV= 'vd.',
	vxUnparkSub_YUV  = 'vuy.',
	vxUnparkSub_AMT  = 'amt ',
	vxUnparkSub_DFF  = 'ffd.',
	vxUnparkSub_DSF  = 'fsd.',
	vxUnparkSub_SUBTITLE = 'stil',//Jetsen字幕文件
	vxUnparkSub_XMLTITLE = 'lmx.',//XML freecg字幕
	vxUnparkSub_SRT		 = 'trs.',//SRT字幕
	vxUnparkSub_ASS		 = 'ssa.',//SRT字幕
	vxUnparkSub_DPX		 = 'xpd.',//SRT字幕

	vxFormat_none	 = 'none',	//没有对应的视音频流

	vxFormat_MPEG1	 = 'mp1v',
	vxFormat_MPEG1_X = 'm1vx',	// JetSen mpeg1
	vxFormat_MPEG2	 = 'mp2v',
	vxFormat_MPEG2_X = 'm2vx',	// JetSen mpeg2
	vxFormat_MPEG2_I = 'mp2I',
	vxFormat_MPEG2_I_X = 'm2Ix', // JetSen mpeg2 I
	vxFormat_MIMX	 = 'mimx',
	vxFormat_MIMX_X	 = 'mixx',   // JetSen mpeg IMX
	vxFormat_MPEG4	 = 'mp4v',
	vxFormat_MP42    = '24PM',
	vxFormat_MP43	 = '34PM',
	vxFormat_MPEG4_RAW = 'm4ra',
	vxFormat_H263	 = '362h',
	vxFormat_H264	 = '462h',
	vxFormat_H264P	 = '462e',//1/4原始分辨率压缩
	vxFormat_X264	 = '462x',
	vxFormat_X265	 = '562x',
	vxFormat_XAVC	 = 'cvax',//xavc
	vxFormat_XAVC10	 = 'tvax',//xavc10Bit
	vxFormat_XAVC8	 = 'evax',//xavc8Bit
	vxFormat_XAVC_MC = 'cvam',//MC XAVC Encoder
	vxFormat_dsxhw264 = '462m',
	vxFormat_hevc	 = '562h',//HEVC or H265
	vxFormat_dvsd	 = 'dsvd',
	vxFormat_dv25	 = '52vd',//dvcpro25
	vxFormat_dv50	 = '05vd',//dvcpro50
	vxFormat_dvh1	 = '1hvd',//dvcpro100
	vxFormat_dvh5	 = 'dvh5',//apple dvcpro100
	vxFormat_dvh6	 = 'dvh6',//apple dvcpro100
	vxFormat_apdv	 = 'apdv',//quicktime dv
	vxFormat_ap4x	 = 'ap4x',//Apple ProRes 4444 XQ
	vxFormat_ap4h	 = 'ap4h',//Apple ProRes 4444
	vxFormat_apch	 = 'apch',//Apple ProRes 422 HQ
	vxFormat_apcn	 = 'apcn',//Apple ProRes 422
	vxFormat_apcs    = 'apcs',//Apple ProRes 422 LT
	vxFormat_apco    = 'apco',//Apple ProRes 422 Proxy
	vxFormat_apchex  = 'Jtch',//Apple ProRes 422 HQ Jetsen优化
	vxFormat_apcnex  = 'Jtcn',//Apple ProRes 422 Jetsen优化
	vxFormat_DNxDV	 = 'AVdv',//AVID DV codec
	vxFormat_DNxHD	 = 'AVdn',//AVID DNxHD codec
	vxFormat_DNxHDEX = 'Jtdn',//AVID DNxHD codec Jetsen优化
	vxFormat_DNxHR	 = 'AVdh',//AVID DNxHR codec
	vxFormat_mjpg	 = 'gpjm',//motion-jpeg
	vxFormat_mjp2k	 = '2pjm',//motion-JPEG2000
	vxFormat_AVC_I	 = '462p',
	vxFormat_wmv1    = '1VMW',
	vxFormat_wmv2	 = '2VMW',//Window Media Video 8
	vxFormat_wmv3	 = '3VMW',//Window Media Video 9
	vxFormat_pwmv	 = 'vmwp',//Window Media Video 9
	vxFormat_vc1	 = '1CVW',//Window Media VC-1
	vxFormat_bid	 = ' bid',
	vxFormat_DivX5   = '05xd',
	vxFormat_flv1    = '1vlf',
	vxFormat_vp6f	 = 'f6pv',
	vxFormat_yuy2	 = '2yuy',
	vxFormat_uyvy	 = 'yvyu',
	vxFormat_ap208	 = 'v208',//Apple Component Y'CbCr 8-bit 4:2:2
	vxFormat_ap210	 = 'v210',//Apple Component Y'CbCr 10-bit 4:2:2
	vxFormat_rgb32	 = 'xbgr',
	vxFormat_rgb24	 = ' bgr',
	vxFormat_dpx	 = 'xpdm',
	vxFormat_AMT     = ' tma',
	vxFormat_AVS	 = ' sva',
	vxFormat_CAVS	 = 'svac',
	vxFormat_SWFD    = 'dfws',
	vxFormat_HQX	 = 'XQHC',//Canopus HQX
	vxFormat_LLC	 = 'CLLC',//Canopus Lossless Codec
	vxFormat_UVC	 = 'CVUC',//Canopus HQ/HQA

	vxFormat_PCM	 = ' mcp',
	vxFormat_PCMF_LE = 'elfp',
	vxFormat_PCMF_BE = 'ebfp',
	vxFormat_BPCM	 = 'MCPB',
	vxFormat_PCM2	 = '2mcp',
	vxFormat_AES3	 = '3SEA',//使用MainConcept enc_pcm
	vxFormat_MPA	 = ' apm',
	vxFormat_MPA2	 = '2apm',
	vxFormat_MP3	 = ' 3pm',
	vxFormat_AC3	 = ' 3ca',
	vxFormat_EAC3    = '3cae',
	vxFormat_DTS	 = ' std',
	vxFormat_AAC	 = 'caaf',
	vxFormat_AAC2	 = 'caaj',
	vxFormat_DPCM    = 'mcpd',//dv数据内嵌音频
	vxFormat_wma1	 = 0x0160, //Window Media Audio 9
	vxFormat_wma2	 = 0x0161,
	vxFormat_wma3    = 0x0162,
	vxFormat_wmaPro  = 'PAMW',
	vxFormat_pwma	 = 'amwp',//Window Media Audio 9
	vxFormat_amrn    = 'nrma',
	vxFormat_amrw    = 'wrma',
	vxFormat_TrueHD  = 'dhet',
	vxFormat_VorBis  = 'oVsm',
	vxFormat_s48	 = '84s.',
	vxFormat_dff	 = ' FFD',
	vxFormat_dsf	 = ' FSD',

	vxFormat_pgst	 = 'pgst',
	vxFormat_dvdt	 = 'dvdt',
	vxFormat_dvbt	 = 'dvbt',
	vxFormat_txtt	 = 'txtt',
	vxFormat_bxml    = '1lmx',//B类字幕

	vxSystemClock_mm = 'mmsm',	//windows 多媒体时钟
	vxSystemClock_dsx= 'mxsd',	//dsx时钟

	vxVideoLive_nod	 = 'adon',//没有视频预监窗口
	vxVideoLive_dx9	 = '19xd',
	vxVideoLive_gl	 = 'lgpo',
	vxVideoLive_gl2	 = '2gpo',
	vxVideoLive_dd	 = 'ddxd',

	vxVidInput_dsx	 = 'vxsd',
	vxVidInput_KS	 = 'sksm',
	vxVidInput_CMIO  = 'oimc',

	vxAudInput_dsx	 = 'axsd',
	vxAudInput_KS	 = 'sksm',
	vxAudInput_CA	 = 'uatq',
	vxAudInput_WAS	 = 'sawm',
	vxAudInput_ASIO  = 'oisa',

	vxVidOutput_over = 'revo',
	vxVidOutput_dsx	 = 'vxsd',
	vxVidOutput_CMIO = 'oimc',
	vxVidOutput_mt	 = 'rtnm',

	vxAudOutput_dsx	 = 'axsd',
	vxAudOutput_DS	 = 'sdsm',
	vxAudOutput_KS	 = 'sksm',
	vxAudOutput_WAS	 = 'sawm',
	vxAudOutput_ASIO = 'oisa',
	vxAudOutput_CA	 = 'uatq',
	vxAudOutput_CMIO = 'oimc',
};


#define vxstreamVIDEO		'sdiv'//mmioFOURCC('v', 'i', 'd', 's')
#define vxstreamAUDIO		'sdua'//mmioFOURCC('a', 'u', 'd', 's')
#define vxstreamSUBPIC		'cips'//mmioFOURCC('s', 'p', 'i', 'c'),	//字幕格式
#define vxstreamDATA		'atad'//mmioFOURCC('d', 'a', 't', 'a')


//I,B,P帧,与MPEG标准一致
#define vxFrameIType	1	
#define vxFramePType	2
#define vxFrameBType	3

#define FILEWBUFFER_MASK 0xff0000
//音视频流数据源路径参数
typedef struct	
{
	DWORD	reffile;				//if(reffile&1)加入文件hash表，进行cache,可以用fastio
									//reffile&0xFF00获取8-16位保存的多文件source的实际文件个数
									//(reffile&FILEWBUFFER_MASK)>>16在写文件时表示使用的写缓存的id，目前<8
									//if((reffile&0x80000000)==0x80000000)表示这个文件假如是opengop的话要去除I帧后面的B帧
									//if((reffile&0x40000000)==0x40000000)表示这个文件已经存在，不需要清除，仅修改需要修改的部分
									//if((reffile&0x20000000)==0x20000000)表示这个文件是普通文件，不要使用NOBUFFEER模式，sectorsize设为1
									//if((reffile&0x10000000)==0x10000000) 不要使用NOBUFFEER模式，sectorsize设为1,可以异步写
	char	szPath[MAX_VXPATH];		//数据源基本路径 支持http，ftp，mms，rtp，dvd(dvd:+path),多路径(omp:+path)
	char	szExtraPath[MAX_VXPATH];//附加文件,如索引文件等,其类型和命名规则由路径类型定义
	char	szUserName[64];			//访问用户
	char	szPassword[64];			//访问密码
}VX_AVPATH,*LPVX_AVPATH;

vxinline void cpyvxpath(LPVX_AVPATH dst,LPVX_AVPATH src)
{
	memcpy(dst,src,sizeof(VX_AVPATH));
}

vxinline void cpyvxpath(VX_AVPATH& dst,VX_AVPATH& src)
{
	memcpy(&dst,&src,sizeof(VX_AVPATH));
}

#pragma pack(push,2)

typedef struct _VX_MuxInfo
{
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
}VX_MUXINFO,*LPVX_MUXINFO;

typedef struct _VX_MuxInfo2 : public _VX_MuxInfo
{
	void*			metadata;			//元数据信息
	int				metadatasize;		//元数据信息大小
	__int64			start_timecode;		//以帧为单位
	DWORD			reserved2[62];		//保留数据，可存储一些附加信息。
										//当reserved2[0]='clfi'时表示云转码,创建完整的文件,reserved2[1]返回CLOUDFILEINFO的个数，reserved2[2-64]存储返回的CLOUDFILEINFO结构
										//当reserved2[0]='clsg'时表示云转码,片段写入,reserved2[1]=CLOUDSEGMENT的个数，reserved2[2-64]存储CLOUDSEGMENT结构
	                                    //当reserved2[0]='p2md'时表示复用p2 mxf文件，reserved[1-4]存储UMID后16字节，reserved[5-6]存储clipname
										//当reserved2[0]='umid'时表示解复用信息，reserved[1-2]存储UMID中间8字节(16-23)
}VX_MUXINFO2,*LPVX_MUXINFO2;

//视频数据格式描述
#define VXVIDEOSA_BITRATE				0x00000001
#define VXVIDEOSA_RES					0x00000002
#define VXVIDEOSA_FPS					0x00000004
typedef struct _VXVideoInfo
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
	int				rate	;			//帧速率，通过两个参数描述，=rate/scale
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
}VX_VIDEOINFO,*LPVX_VIDEOINFO;

typedef struct _VXVideoInfo2 : public _VXVideoInfo
{
	void*			metadata;			//元数据信息
	int				metadatasize;		//元数据信息大小
	DWORD			muxtype;
	DWORD			reserved2[47];		//保留数据，可存储一些附加信息。
										// 0、1：标记画面旋转角度；// 2：标记是否获取ScanType,有值代表未获取
										// 3: 标记是否使用文件中的aspect
	DWORD			trimin,trimout;		//视频有效帧出入点,前包后不包；trimin==triimout无效，目前仅在movReferenc中可能有效
	DWORD			reserved3[14];		//保留数据，可存储一些附加信息。
}VX_VIDEOINFO2,*LPVX_VIDEOINFO2;


//音频数据格式描述
#define VXAUDIOSA_BITRATE				0x00000001
#define VXAUDIOSA_CHANNELS				0x00000002
#define VXAUDIOSA_BITS					0x00000004
#define VXAUDIOSA_FREQ					0x00000008
typedef struct _VXAudioInfo
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
	VXBOOL			vbr;
	DWORD			reserved[2];		//保留数据，可存储一些附加信息。
}VX_AUDIOINFO,*LPVX_AUDIOINFO;


typedef struct _VXAudioInfo2 : public _VXAudioInfo
{
	void*			metadata;			//元数据信息
	int				metadatasize;		//元数据信息大小
	DWORD			muxtype;
	DWORD			reserved2[47];		//保留数据，可存储一些附加信息。reserved2[0]已经被占用了，用来存储合成时是否启用音量大小设定标志。
	__int64			trimin,trimout;		//音频有效样本出入点,前包后不包；trimin==triimout无效，目前仅在movReferenc中可能有效
	DWORD			reserved3[12];		//保留数据，可存储一些附加信息。reserved2[0]已经被占用了，用来存储合成时是否启用音量大小设定标志。
}VX_AUDIOINFO2,*LPVX_AUDIOINFO2;

//字幕数据格式描述
typedef struct _VXSubTInfo
{
	DWORD			fourcc;				//字幕压缩格式
	__int64			duration;			//时间ms
	int				width,height;
	DWORD		    aspect;
	int				renderw, renderh;
	int				render_topfirst;			//0->场基-底场优先，1->场基-顶场优先，2->帧基，3->帧场自适应
	char			lang[4];
	DWORD			palette[256];
	void*			pExtraData;			//附加参数数据，必须用_vxmalloc来分配
	DWORD			dwExtraSize;		//附加参数大小和类型, 高8位为类型
	void*			metadata;			//元数据信息
	int				metadatasize;		//元数据信息大小
}VX_SUBTINFO,*LPVX_SUBTINFO;


//处理原则是 滤镜-反交织-剪切-缩放原视频在目标中的大小-目标
typedef struct _VXVideoPreprocess
{
	int filter;						//0->表示没有滤镜,1---n滤镜类型
	int filterdatas;				//滤镜参数大小
	void* filterdata;				//滤镜参数
	int deinterlace;				//0->不需要反交织,1---n反交织类型
	int deinterlacedatas;			//反交织参数大小
	void* deinterlacedata;			//反交织参数
	VXRECT crop_rect;				//剪切
	VXSIZE outsize;					//缩放
	int noisereduction;				//0->不需要噪声消除,>=1程度
	VXRECT srcinrect;				//原视频在目标中的位置和大小，为零则outsize就是缩放的大小	
}VX_VIDEOPPREPROCESS;

#pragma pack(pop)


#define VX_MAX_PLANES	6				// Number of planes in a picture

#define PFMT_YUV420P	 '0vuy'	//yuv分量 420
#define PFMT_YUV422P	 '1vuy'	//yuv分量 422
#define PFMT_YUV444P	 '2vuy'	//yuv分量 444
#define PFMT_YUV422P16	 '61py'	//yuv分量 422 16bit
#define PFMT_YUY2		 '2yuy'	
#define PFMT_YUYF		 'fyuy'	//yuy2格式,但两场分别存储
#define PFMT_UYVY		 'yvyu'	
#define PFMT_RGB32		 'xbgr'	
#define PFMT_RGB24		 ' bgr'	
#define PFMT_GRAY8		 'gray'	

typedef enum{ RES_480, RES_576, RES_720, RES_HDV_1080, RES_1080,RES_3D_1080, RES_360, RES_UHD2160,
			  RES_2K1152, RES_2K1536, RES_2KDCIFLAT, RES_2KDCIFULL, RES_2KDCISCOPE, RES_2KFULL,
			  RES_4KDCIFLAT, RES_4KDCIFULL, RES_4KDCISCOPE, RES_4KFULL,
			  RES_CUSTOM,RES_LAST
} VXSURFACE_RES;

#define VXCUSTOMRESMASK		0x80000000
#define MAKEVXCUSTOMRES(w,h) ((VXSURFACE_RES)((h<<16)|w|VXCUSTOMRESMASK))

//FMT_*P16所描述格式16比特存储，14比特有效数据
typedef enum{ FMT_BGRA, FMT_YUY2, FMT_A8, FMT_L8, FMT_A8L8, FMT_YUY2A, FMT_YUV422P8, FMT_YUV422P16, FMT_YUV444P8, FMT_YUV444P16, FMT_YUVA4224P8, FMT_YUVA4224P16, FMT_YUVA4444P8, FMT_YUVA4444P16, FMT_A16, FMT_L16, FMT_BGRX, FMT_LAST } VXSURFACE_FMT;
typedef enum{ LOC_HOST,LOC_IO,LOC_GPU,LOC_GPUIN,LOC_GPUOUT,LOC_LAST} VXSURFACE_LOC;
typedef enum{ COLORIMETRY_UNKNOWN, COLORIMETRY_BT601, COLORIMETRY_BT709, COLORIMETRY_DCIP3, COLORIMETRY_BT2020, COLORIMETRY_LAST } VXSURFACE_COLORIMETRY;
typedef enum{ SCANTYPE_BOTTOMFIRST, SCANTYPE_TOPFIRST, SCANTYPE_PROGRESSIVE, SCANTYPE_LAST} VXSURFACE_SCANTYPE;

struct resinfo{int w,h;DWORD aspect;VXSURFACE_COLORIMETRY colorimetry;};
static const resinfo g_resinfo2[RES_LAST] = 
{	
	{ 720,	 480, 0x00030004,	COLORIMETRY_BT601}, // RES_480
	{ 720,	 576, 0x00030004,	COLORIMETRY_BT601}, // RES_576
	{1280,	 720, 0x00090010,	COLORIMETRY_BT709}, // RES_720
	{1440,	1080, 0x00090010,	COLORIMETRY_BT709}, // RES_HDV_1080
	{1920,	1080, 0x00090010,	COLORIMETRY_BT709}, // RES_1080
	{ 960,	1080, 0x00090010,	COLORIMETRY_BT709}, // RES_3D_1080
	{ 640,   360, 0x00090010,	COLORIMETRY_BT709}, // RES_360
	{ 3840, 2160, 0x00090010, COLORIMETRY_BT709 }, // RES_UHD2160
	{ 2048, 1152, 0x00090010, COLORIMETRY_BT709 }, // RES_2K1152
	{ 2048, 1536, 0x00030004, COLORIMETRY_BT709 }, // RES_2K1536
	{ 1998, 1080, 0x021c03e7, COLORIMETRY_BT709 }, // RES_2KDCIFLAT
	{ 2048, 1080, 0x00870100, COLORIMETRY_BT709 }, // RES_2KDCIFULL
	{ 2048,  858, 0x00090010, COLORIMETRY_BT709 }, // RES_2KDCISCOPE
	{ 2048, 1556, 0x01850200, COLORIMETRY_BT709 }, // RES_2KFULL
	{ 3996, 2160, 0x021c03e7, COLORIMETRY_BT709 }, // RES_4KDCIFLAT
	{ 4096, 2160, 0x00870100, COLORIMETRY_BT709 }, // RES_4KDCIFULL
	{ 4096, 1716, 0x01ad0400, COLORIMETRY_BT709 }, // RES_4KDCISCOPE
	{ 4096, 3112, 0x01850200, COLORIMETRY_BT709 }, // RES_4KFULL
};

struct fmtinfo{int plane,bpc,chs;int planeratio[4];};
static const fmtinfo g_fmtinfo[FMT_LAST] = 
{	
	{ 1, 8,4,{1}}, // FMT_BGRA
	{ 1, 8,2,{1}}, // FMT_YUY2
	{ 1, 8,1,{1}}, // FMT_A8
	{ 1, 8,1,{1}}, // FMT_L8
	{ 1, 8,2,{1}}, // FMT_A8L8
	{ 2, 8,3,{2,1}}, // FMT_YUY2A
	{ 3, 8,2,{2,1,1}}, // FMT_YUV422P8
	{ 3,16,2,{2,1,1}}, // FMT_YUV422P16
	{ 3, 8,3,{1,1,1 } }, // FMT_YUV444P8
	{ 3,16,3,{1,1,1 } }, // FMT_YUV444P16
	{ 4, 8,3,{2,1,1,2 } }, // FMT_YUVA4224P8
	{ 4,16,3,{2,1,1,2 } }, // FMT_YUVA4224P16
	{ 4, 8,4,{1,1,1,1 } }, // FMT_YUVA4444P8
	{ 4,16,4,{1,1,1,1 } }, // FMT_YUVA4444P16
	{ 1, 16, 1, { 1 } }, // FMT_L16
	{ 1, 16, 1, { 1 } }, // FMT_A16
	{ 1, 8, 4, { 1 } }, // FMT_BGRX
};

vxinline int __getfmtbytes(VXSURFACE_FMT fmt)
{
	const fmtinfo& finfo = g_fmtinfo[fmt];
	return finfo.bpc*finfo.chs / 8;
}

typedef enum{FREQ_48000,FREQ_96000,FREQ_192000,FREQ_8000,FREQ_24000,FREQ_32000,FREQ_44100,FREQ_LAST} VXAUD_FREQ;
typedef enum{ALOC_HOST,ALOC_IO,ALOC_LAST} VXAUD_LOC;
static const int g_vxfreq[FREQ_LAST] = {48000,96000,192000,8000,24000,32000,44100};

vxinline LONG vx_gcd(LONG a, LONG b){
	if(b) return vx_gcd(b, a%b);
	else  return a;
}

vxinline LONG vx_lcm(LONG a, LONG b)
{
	return __int64(a)*b/vx_gcd(a,b);
}

vxinline void type2low(BYTE* type)
{
#define A2a  (int)('a'-'A');
	if((type[0]<='Z')&&(type[0]>='A')) type[0] += A2a;
	if((type[1]<='Z')&&(type[1]>='A')) type[1] += A2a;
	if((type[2]<='Z')&&(type[2]>='A')) type[2] += A2a;
	if((type[3]<='Z')&&(type[3]>='A')) type[3] += A2a;
}

vxinline void type2high(BYTE* type)
{
#define A2a  (int)('a'-'A');
	if((type[0]<='z')&&(type[0]>='a')) type[0] -= A2a;
	if((type[1]<='z')&&(type[1]>='a')) type[1] -= A2a;
	if((type[2]<='z')&&(type[2]>='a')) type[2] -= A2a;
	if((type[3]<='z')&&(type[3]>='a')) type[3] -= A2a;
}

#ifdef __GNUC__
#ifdef ARCH_X86_64
#  define LEGACY_REGS "=Q"
#else
#  define LEGACY_REGS "=q"
#endif

static vxinline DWORD bswap32(DWORD x)
{
    __asm("bswap	%0":
		"=r" (x)     :
        "0" (x));
    return x;
}
#else
#ifdef _WIN64
#include "stdlib.h"
#endif
static vxinline const DWORD bswap32(DWORD x)
{
#ifdef _WIN64
	return _byteswap_ulong(x);
#else
	__asm mov eax,x __asm bswap eax// __asm mov x, eax;
#endif
}

#endif
#define swab32 bswap32


vxinline void __vxgetreswh(VXSURFACE_RES res, int& width, int& height, DWORD* aspect = NULL)
{
	if (res&VXCUSTOMRESMASK)
	{
		width = res & 0xFFFF;
		height = (res & 0x7FFF0000) >> 16;
		if (aspect)
		{
			int gcd = vx_gcd(width, height);
			DWORD aspectw = width / gcd;
			DWORD aspecth = height / gcd;
			*aspect = aspectw | (aspecth << 16);
		}
	}
	else
	{
		width = g_resinfo2[res].w;
		height = g_resinfo2[res].h;
		if (aspect) *aspect = g_resinfo2[res].aspect;
	}
}

#endif//__BAY_VXSYSDEF_H__
