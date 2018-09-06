
#ifndef __MXMEDIAINFO_H__
#define __MXMEDIAINFO_H__

#include "MxTypes.h"

#define MX_MAXPATH    512
#define MX_MAXPLANES  6                // Number of planes in a picture
#define MX_MAXAUDIOCHANNELS  64
#define MX_MAXFASTIONUM  300

#ifdef ROCKBOX_BIG_ENDIAN
#define MX_FOURCC( a, b, c, d ) ( ((uint32)(d)) | ( ((uint32)(c)) << 8 ) | ( ((uint32)(b)) << 16 ) | ( ((uint32)(a)) << 24 ) )
#define MX_TWOCC( a, b ) ( (uint16)(b) | ( (uint16)(a) << 8 ) )
#else
#define MX_FOURCC( a, b, c, d ) ( ((uint32)(a)) | ( ((uint32)(b)) << 8 ) | ( (uint32)(c) << 16 ) | ( ((uint32)(d)) << 24 ) )
#define MX_TWOCC( a, b ) ( (uint16)(a) | ( (uint16)(b) << 8 ) )
#endif

struct MxPath
{
    uint reffile;
    char szPath[MX_MAXPATH];
    char szExtraPath[MX_MAXPATH];
};

enum MxSourceType {
    MxSourceType_file   = MX_FOURCC('f','i','l','e'),
    MxSourceType_dvd    = MX_FOURCC('d','v','d',' '),
    MxSourceType_bdr    = MX_FOURCC('b','d','r',' '),
    MxSourceType_ftp    = MX_FOURCC('f','t','p',' '),
    MxSourceType_http   = MX_FOURCC('h','t','t','p'),
    MxSourceType_smb    = MX_FOURCC('s','m','b',' '),
    MxSourceType_rtp    = MX_FOURCC('r','t','p',' '),
    MxSourceType_rtcp   = MX_FOURCC('r','t','c','p'),
};

enum MxMuxerType {
    MxMuxerType_avi     = MX_FOURCC('a','v','i',' '),
    MxMuxerType_mov     = MX_FOURCC('m','o','v',' '),
};

enum MxFormatType {
    MxFormatType_h264   = MX_FOURCC('h','2','6','4'),
    MxFormatType_x264   = MX_FOURCC('x','2','6','4'),
    
    
    MxFormatType_pcm   = MX_FOURCC('p','c','m',' '),
};

enum MxStreamType {
    MxStreamType_video  = MX_FOURCC('v','i','d','s'),
    MxStreamType_audio  = MX_FOURCC('a','u','d','s'),
    MxStreamType_subpic = MX_FOURCC('s','p','i','c'),
    MxStreamType_data   = MX_FOURCC('d','a','t','a'),
};

enum MxFrameType {
    MxFrameType_I       = 1,
    MxFrameType_P       = 2,
    MxFrameType_B       = 3,
};

struct MxMuxerInfo{
    uint32  fourcc;             //∏¥∫œ∏Ò Ω
    uint16  pid;                //∏¥∫œ±Í ∂
    uint8   novideo;            //0-->±Ì æ∞¸∫¨ ”∆µ,∑«¡„ƒ£∞Â≤ª–Ë“™—πÀı ”∆µ
    uint8   noaudio;            //0-->±Ì æ∞¸∫¨“Ù∆µ,∑«¡„ƒ£∞Â≤ª–Ë“™—πÀı“Ù∆µ
    uint32  bitrate;            //∏¥∫œ¬Î¬
    void*   pExtraData;         //∏Ωº”≤Œ ˝ ˝æ›£¨±ÿ–Î”√_vxmalloc¿¥∑÷≈‰
    uint32  dwExtraSize;        //∏Ωº”≤Œ ˝¥Û–°∫Õ¿‡–Õ, ∏ﬂ8ŒªŒ™¿‡–Õ
    uint32  pmtid;              //TS÷–PMT ID
    uint32  tsid;               //TS÷–transport_stream ID
    uint32  reserved;           //±£¡Ù ˝æ›£¨ø…¥Ê¥¢“ª–©∏Ωº”–≈œ¢°£‘⁄MXFΩ‚∏¥”√÷–reserved!=0Œ™Opatom,∑Ò‘Ú «Op1a
    void*   metadata;           //‘™ ˝æ›–≈œ¢
    int     metadatasize;       //‘™ ˝æ›–≈œ¢¥Û–°
    int64   start_timecode;     //“‘÷°Œ™µ•Œª
    uint32  reserved2[62];
};

struct MxVideoInfo {
    uint32          fourcc; //视频压缩格式
    uint32          pid; //视频标识
    uint32          dectype; //解码方式 1->帧间无关，0->帧间相关(IBP)，必须在下一帧数据输给解码器才返回上一帧的数据，2->帧间相关,但是输给解码器哪一帧就能等到哪一帧(IP);3->得到一组压缩数据包含多帧
    uint32          frames;  //视频长度
    int             picture_width; //图像大小
    int             picture_height;  //
    int             display_width; //显示大小
    int             display_height;
    int             display_xoffset;
    int             display_yoffset;
    short           depth; //bit大小
    short           colorimetry; //颜色空间BT.601,BT.709.... VXSURFACE_COLORIMETRY
    int             rate; //帧速率，通过两个参数描述，=rate/scale
    int             scale;
    uint32          bitrate; //码率,Bits/s,最高位指示是否为变码率
    uint16          profile; //档次, MPEG2有效
    uint16          level; //级次, MPEG2有效
    int             chromafmt; //色差格式 chroma_format 1=420, 2=422, 3=444, 4=411
    int             topfirst; //0->场基-底场优先，1->场基-顶场优先，2->帧基，3->帧场自适应
    uint32          aspect; //显示比例 4:3==>0x00030004 16:9==>0x00090010,高4位存储AFD值(0xF0000000)，数值参照SMPTE 2016-1
    int             quality; //压缩质量,在压缩时范围为(1-100) 0==>自动
    uint32          suggestedsize; //
    uint32          dwKeyframeSpace; //关键帧间隔,对于MPEG和H264文件为低8位为GOP长度,8-15位P帧间B帧数,H264 16-23位为参考帧个数,24-31为IDR帧间隔
    void*           pExtraData; //附加参数数据，必须用_vxmalloc来分配
    uint32          dwExtraSize; //附加参数大小和类型, 高8位为类型
    uint32          decoutfmt; //解码后的格式［ＩＮ，ＯＵＴ］
    uint8           vbr;
    uint8           twopass; //在解码时无效
    uint16          selfadapting; //自适应参数 VXVIDEOSA_XXX
    uint32          max_bitrate; //
    uint32          reserved[2]; //保留数据，可存储一些附加信息。
    void*           metadata; //元数据信息
    int             metadatasize; //元数据信息大小
    uint32          muxtype;
    uint32          reserved2[47]; //保留数据，可存储一些附加信息。
    // 0、1：标记画面旋转角度；// 2：标记是否获取ScanType,有值代表未获取
    // 3: 标记是否使用文件中的aspect
    uint32          trimin, trimout;        //视频有效帧出入点,前包后不包；trimin==triimout无效，目前仅在movReferenc中可能有效
    uint32          reserved3[14];        //保留数据，可存储一些附加信息。
};

struct MxAudioInfo
{
    uint32          fourcc; //音频压缩格式
    uint32          pid; //音频标识
    uint32          dectype; //0-->以样本定位解码（可以是连续的pcm数据，也可以是每个帧包含不固定样本数），1-->以帧定位解码（每个帧包含固定的样本数），2-->输出的是数据流
    int64           samples; //音频长度
    uint32          samplesperframe; //音频帧样本数，当dectype=1时为最大帧样本大小
    uint32          framesize; //基本单元大小。对于MPEG音频，为音频帧大小
    uint32          freq; //采样率
    int             channels; //通道数
    int             bitpersample; //样本精度
    int             blockalign; //
    uint32          bitrate; //码率,Bits/s
    int             quality; //压缩质量,在压缩时范围为(1-100) 0==>自动
    uint32          suggestedsize; //
    void*           pExtraData; //附加参数数据，必须用_vxmalloc来分配
    uint32          dwExtraSize; //附加参数大小和类型
    uint32          rfourcc; //真实的音频压缩格式
    uint32          rfreq; //真实的采样率
    uint16          selfadapting; //自适应参数 VXAUDIOSA_XXX
    uint8           reservedByte;
    uint8           splitchs; //在复用时复用器会根据自己需要修改
    bool            vbr;
    uint32          reserved[2]; //保留数据，可存储一些附加信息。
    void*           metadata; //元数据信息
    int             metadatasize; //元数据信息大小
    uint32          muxtype;
    uint32          reserved2[47]; //保留数据，可存储一些附加信息。reserved2[0]已经被占用了，用来存储合成时是否启用音量大小设定标志。
    int64           trimin, trimout; //音频有效样本出入点,前包后不包；trimin==triimout无效，目前仅在movReferenc中可能有效
    uint32          reserved3[12]; //保留数据，可存储一些附加信息。reserved2[0]已经被占用了，用来存储合成时是否启用音量大小设定标志。
};

#endif /* __MXMEDIAINFO_H__ */
