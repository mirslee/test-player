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
	vxObjSource,	//����Դ�������ļ������������忨��
	vxObjSink,		//�����������
	vxObjUnpack,	//���(�⸴��)����
	vxObjPack,		//���(����)����
	vxObjVidDecoder,//��Ƶ�������
	vxObjVidEncoder,//��Ƶ�������
	vxObjAudDecoder,//��Ƶ�������
	vxObjAudEncoder,//��Ƶ�������
	vxObjSubDecoder,//��Ļ�������

	vxObjVideoLive,	//��ƵԤ���豸

	vxObjVidInput,	//��Ƶ�����豸
	vxObjAudInput, //��Ƶ�����豸
	vxObjVidOutput,	//��Ƶ����豸
	vxObjAudOutput, //��Ƶ����豸

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
	vxUnparkSub_SUBTITLE = 'stil',//Jetsen��Ļ�ļ�
	vxUnparkSub_XMLTITLE = 'lmx.',//XML freecg��Ļ
	vxUnparkSub_SRT		 = 'trs.',//SRT��Ļ
	vxUnparkSub_ASS		 = 'ssa.',//SRT��Ļ
	vxUnparkSub_DPX		 = 'xpd.',//SRT��Ļ

	vxFormat_none	 = 'none',	//û�ж�Ӧ������Ƶ��

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
	vxFormat_H264P	 = '462e',//1/4ԭʼ�ֱ���ѹ��
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
	vxFormat_apchex  = 'Jtch',//Apple ProRes 422 HQ Jetsen�Ż�
	vxFormat_apcnex  = 'Jtcn',//Apple ProRes 422 Jetsen�Ż�
	vxFormat_DNxDV	 = 'AVdv',//AVID DV codec
	vxFormat_DNxHD	 = 'AVdn',//AVID DNxHD codec
	vxFormat_DNxHDEX = 'Jtdn',//AVID DNxHD codec Jetsen�Ż�
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
	vxFormat_AES3	 = '3SEA',//ʹ��MainConcept enc_pcm
	vxFormat_MPA	 = ' apm',
	vxFormat_MPA2	 = '2apm',
	vxFormat_MP3	 = ' 3pm',
	vxFormat_AC3	 = ' 3ca',
	vxFormat_EAC3    = '3cae',
	vxFormat_DTS	 = ' std',
	vxFormat_AAC	 = 'caaf',
	vxFormat_AAC2	 = 'caaj',
	vxFormat_DPCM    = 'mcpd',//dv������Ƕ��Ƶ
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
	vxFormat_bxml    = '1lmx',//B����Ļ

	vxSystemClock_mm = 'mmsm',	//windows ��ý��ʱ��
	vxSystemClock_dsx= 'mxsd',	//dsxʱ��

	vxVideoLive_nod	 = 'adon',//û����ƵԤ�ര��
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
#define vxstreamSUBPIC		'cips'//mmioFOURCC('s', 'p', 'i', 'c'),	//��Ļ��ʽ
#define vxstreamDATA		'atad'//mmioFOURCC('d', 'a', 't', 'a')


//I,B,P֡,��MPEG��׼һ��
#define vxFrameIType	1	
#define vxFramePType	2
#define vxFrameBType	3

#define FILEWBUFFER_MASK 0xff0000
//����Ƶ������Դ·������
typedef struct	
{
	DWORD	reffile;				//if(reffile&1)�����ļ�hash������cache,������fastio
									//reffile&0xFF00��ȡ8-16λ����Ķ��ļ�source��ʵ���ļ�����
									//(reffile&FILEWBUFFER_MASK)>>16��д�ļ�ʱ��ʾʹ�õ�д�����id��Ŀǰ<8
									//if((reffile&0x80000000)==0x80000000)��ʾ����ļ�������opengop�Ļ�Ҫȥ��I֡�����B֡
									//if((reffile&0x40000000)==0x40000000)��ʾ����ļ��Ѿ����ڣ�����Ҫ��������޸���Ҫ�޸ĵĲ���
									//if((reffile&0x20000000)==0x20000000)��ʾ����ļ�����ͨ�ļ�����Ҫʹ��NOBUFFEERģʽ��sectorsize��Ϊ1
									//if((reffile&0x10000000)==0x10000000) ��Ҫʹ��NOBUFFEERģʽ��sectorsize��Ϊ1,�����첽д
	char	szPath[MAX_VXPATH];		//����Դ����·�� ֧��http��ftp��mms��rtp��dvd(dvd:+path),��·��(omp:+path)
	char	szExtraPath[MAX_VXPATH];//�����ļ�,�������ļ���,�����ͺ�����������·�����Ͷ���
	char	szUserName[64];			//�����û�
	char	szPassword[64];			//��������
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
	DWORD		fourcc;				//���ϸ�ʽ
	WORD		pid;				//���ϱ�ʶ
	BYTE		novideo;			//0-->��ʾ������Ƶ,����ģ�岻��Ҫѹ����Ƶ
	BYTE		noaudio;			//0-->��ʾ������Ƶ,����ģ�岻��Ҫѹ����Ƶ
	DWORD		bitrate;			//��������
	void*		pExtraData;			//���Ӳ������ݣ�������_vxmalloc������
	DWORD		dwExtraSize;		//���Ӳ�����С������, ��8λΪ����
	DWORD		pmtid;				//TS��PMT ID
	DWORD		tsid;				//TS��transport_stream ID
	DWORD		reserved;			//�������ݣ��ɴ洢һЩ������Ϣ����MXF�⸴����reserved!=0ΪOpatom,������Op1a
}VX_MUXINFO,*LPVX_MUXINFO;

typedef struct _VX_MuxInfo2 : public _VX_MuxInfo
{
	void*			metadata;			//Ԫ������Ϣ
	int				metadatasize;		//Ԫ������Ϣ��С
	__int64			start_timecode;		//��֡Ϊ��λ
	DWORD			reserved2[62];		//�������ݣ��ɴ洢һЩ������Ϣ��
										//��reserved2[0]='clfi'ʱ��ʾ��ת��,�����������ļ�,reserved2[1]����CLOUDFILEINFO�ĸ�����reserved2[2-64]�洢���ص�CLOUDFILEINFO�ṹ
										//��reserved2[0]='clsg'ʱ��ʾ��ת��,Ƭ��д��,reserved2[1]=CLOUDSEGMENT�ĸ�����reserved2[2-64]�洢CLOUDSEGMENT�ṹ
	                                    //��reserved2[0]='p2md'ʱ��ʾ����p2 mxf�ļ���reserved[1-4]�洢UMID��16�ֽڣ�reserved[5-6]�洢clipname
										//��reserved2[0]='umid'ʱ��ʾ�⸴����Ϣ��reserved[1-2]�洢UMID�м�8�ֽ�(16-23)
}VX_MUXINFO2,*LPVX_MUXINFO2;

//��Ƶ���ݸ�ʽ����
#define VXVIDEOSA_BITRATE				0x00000001
#define VXVIDEOSA_RES					0x00000002
#define VXVIDEOSA_FPS					0x00000004
typedef struct _VXVideoInfo
{
	DWORD			fourcc;				//��Ƶѹ����ʽ
	DWORD			pid;				//��Ƶ��ʶ
	DWORD			dectype;			//���뷽ʽ 1->֡���޹أ�0->֡�����(IBP)����������һ֡��������������ŷ�����һ֡�����ݣ�2->֡�����,���������������һ֡���ܵȵ���һ֡(IP);3->�õ�һ��ѹ�����ݰ�����֡
	DWORD			frames;				//��Ƶ����
	int				picture_width;		//ͼ���С
	int				picture_height;		//
	int				display_width;		//��ʾ��С
	int				display_height;
	int				display_xoffset;
	int				display_yoffset;
	short			depth;				//������С
	short			colorimetry;		//��ɫ�ռ�BT.601,BT.709.... VXSURFACE_COLORIMETRY
	int				rate	;			//֡���ʣ�ͨ����������������=rate/scale
	int				scale;
	DWORD			bitrate;			//����,Bits/s,���λָʾ�Ƿ�Ϊ������
	WORD			profile;			//����, MPEG2��Ч
	WORD			level;				//����, MPEG2��Ч
	int				chromafmt;			//ɫ���ʽ chroma_format 1=420, 2=422, 3=444, 4=411
	int				topfirst;			//0->����-�׳����ȣ�1->����-�������ȣ�2->֡����3->֡������Ӧ
	DWORD			aspect;				//��ʾ���� 4:3==>0x00030004 16:9==>0x00090010,��4λ�洢AFDֵ(0xF0000000)����ֵ����SMPTE 2016-1
	int				quality;			//ѹ������,��ѹ��ʱ��ΧΪ(1-100) 0==>�Զ�
	DWORD			suggestedsize;		//
	DWORD			dwKeyframeSpace;	//�ؼ�֡���,����MPEG��H264�ļ�Ϊ��8λΪGOP����,8-15λP֡��B֡��,H264 16-23λΪ�ο�֡����,24-31ΪIDR֡���
	void*			pExtraData;			//���Ӳ������ݣ�������_vxmalloc������
	DWORD			dwExtraSize;		//���Ӳ�����С������, ��8λΪ����
	DWORD			decoutfmt;			//�����ĸ�ʽ�ۣɣΣ��ϣգԣ�
	BYTE			vbr;
	BYTE			twopass;			//�ڽ���ʱ��Ч
	WORD			selfadapting;		//����Ӧ���� VXVIDEOSA_XXX
	DWORD			max_bitrate;		//
	DWORD			reserved[2];		//�������ݣ��ɴ洢һЩ������Ϣ��
}VX_VIDEOINFO,*LPVX_VIDEOINFO;

typedef struct _VXVideoInfo2 : public _VXVideoInfo
{
	void*			metadata;			//Ԫ������Ϣ
	int				metadatasize;		//Ԫ������Ϣ��С
	DWORD			muxtype;
	DWORD			reserved2[47];		//�������ݣ��ɴ洢һЩ������Ϣ��
										// 0��1����ǻ�����ת�Ƕȣ�// 2������Ƿ��ȡScanType,��ֵ����δ��ȡ
										// 3: ����Ƿ�ʹ���ļ��е�aspect
	DWORD			trimin,trimout;		//��Ƶ��Ч֡�����,ǰ���󲻰���trimin==triimout��Ч��Ŀǰ����movReferenc�п�����Ч
	DWORD			reserved3[14];		//�������ݣ��ɴ洢һЩ������Ϣ��
}VX_VIDEOINFO2,*LPVX_VIDEOINFO2;


//��Ƶ���ݸ�ʽ����
#define VXAUDIOSA_BITRATE				0x00000001
#define VXAUDIOSA_CHANNELS				0x00000002
#define VXAUDIOSA_BITS					0x00000004
#define VXAUDIOSA_FREQ					0x00000008
typedef struct _VXAudioInfo
{
	DWORD			fourcc;				//��Ƶѹ����ʽ
	DWORD			pid;				//��Ƶ��ʶ
	DWORD			dectype;			//0-->��������λ���루������������pcm���ݣ�Ҳ������ÿ��֡�������̶�����������1-->��֡��λ���루ÿ��֡�����̶�������������2-->�������������
	__int64			samples;			//��Ƶ����
	DWORD			samplesperframe;	//��Ƶ֡����������dectype=1ʱΪ���֡������С
	DWORD			framesize;			//������Ԫ��С������MPEG��Ƶ��Ϊ��Ƶ֡��С
	DWORD			freq;				//������ 
	int				channels;			//ͨ����
	int				bitpersample;		//��������
	int				blockalign;			//
	DWORD			bitrate;			//����,Bits/s
	int				quality;			//ѹ������,��ѹ��ʱ��ΧΪ(1-100) 0==>�Զ�
	DWORD			suggestedsize;		//
	void*			pExtraData;			//���Ӳ������ݣ�������_vxmalloc������
	DWORD			dwExtraSize;		//���Ӳ�����С������
	DWORD			rfourcc;			//��ʵ����Ƶѹ����ʽ
	DWORD			rfreq;				//��ʵ�Ĳ����� 
	WORD			selfadapting;		//����Ӧ���� VXAUDIOSA_XXX
	BYTE			reservedByte;
	BYTE			splitchs;			//�ڸ���ʱ������������Լ���Ҫ�޸�
	VXBOOL			vbr;
	DWORD			reserved[2];		//�������ݣ��ɴ洢һЩ������Ϣ��
}VX_AUDIOINFO,*LPVX_AUDIOINFO;


typedef struct _VXAudioInfo2 : public _VXAudioInfo
{
	void*			metadata;			//Ԫ������Ϣ
	int				metadatasize;		//Ԫ������Ϣ��С
	DWORD			muxtype;
	DWORD			reserved2[47];		//�������ݣ��ɴ洢һЩ������Ϣ��reserved2[0]�Ѿ���ռ���ˣ������洢�ϳ�ʱ�Ƿ�����������С�趨��־��
	__int64			trimin,trimout;		//��Ƶ��Ч���������,ǰ���󲻰���trimin==triimout��Ч��Ŀǰ����movReferenc�п�����Ч
	DWORD			reserved3[12];		//�������ݣ��ɴ洢һЩ������Ϣ��reserved2[0]�Ѿ���ռ���ˣ������洢�ϳ�ʱ�Ƿ�����������С�趨��־��
}VX_AUDIOINFO2,*LPVX_AUDIOINFO2;

//��Ļ���ݸ�ʽ����
typedef struct _VXSubTInfo
{
	DWORD			fourcc;				//��Ļѹ����ʽ
	__int64			duration;			//ʱ��ms
	int				width,height;
	DWORD		    aspect;
	int				renderw, renderh;
	int				render_topfirst;			//0->����-�׳����ȣ�1->����-�������ȣ�2->֡����3->֡������Ӧ
	char			lang[4];
	DWORD			palette[256];
	void*			pExtraData;			//���Ӳ������ݣ�������_vxmalloc������
	DWORD			dwExtraSize;		//���Ӳ�����С������, ��8λΪ����
	void*			metadata;			//Ԫ������Ϣ
	int				metadatasize;		//Ԫ������Ϣ��С
}VX_SUBTINFO,*LPVX_SUBTINFO;


//����ԭ���� �˾�-����֯-����-����ԭ��Ƶ��Ŀ���еĴ�С-Ŀ��
typedef struct _VXVideoPreprocess
{
	int filter;						//0->��ʾû���˾�,1---n�˾�����
	int filterdatas;				//�˾�������С
	void* filterdata;				//�˾�����
	int deinterlace;				//0->����Ҫ����֯,1---n����֯����
	int deinterlacedatas;			//����֯������С
	void* deinterlacedata;			//����֯����
	VXRECT crop_rect;				//����
	VXSIZE outsize;					//����
	int noisereduction;				//0->����Ҫ��������,>=1�̶�
	VXRECT srcinrect;				//ԭ��Ƶ��Ŀ���е�λ�úʹ�С��Ϊ����outsize�������ŵĴ�С	
}VX_VIDEOPPREPROCESS;

#pragma pack(pop)


#define VX_MAX_PLANES	6				// Number of planes in a picture

#define PFMT_YUV420P	 '0vuy'	//yuv���� 420
#define PFMT_YUV422P	 '1vuy'	//yuv���� 422
#define PFMT_YUV444P	 '2vuy'	//yuv���� 444
#define PFMT_YUV422P16	 '61py'	//yuv���� 422 16bit
#define PFMT_YUY2		 '2yuy'	
#define PFMT_YUYF		 'fyuy'	//yuy2��ʽ,�������ֱ�洢
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

//FMT_*P16��������ʽ16���ش洢��14������Ч����
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
