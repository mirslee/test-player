//matroxavioutif.h  write by bay at 2002-12-24
#ifndef __MXAVI_H__
#define __MXAVI_H__

#pragma warning(disable: 4018)    // signed/unsigned mismatch
#pragma warning(disable: 4200)    // zero-sized array in struct/union
#pragma warning(disable: 4244)    // conversion from LONGLONG, possible loss of data

#ifdef _WIN32
#include "VFW.h"
#include "strmif.h"
#include "amvideo.h"
#include "dvdmedia.h"
#else

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize __attribute__ ((packed));
	LONG  biWidth __attribute__ ((packed));
	LONG  biHeight __attribute__ ((packed));
	WORD  biPlanes __attribute__ ((packed));
	WORD  biBitCount __attribute__ ((packed));
	DWORD biCompression __attribute__ ((packed));
	DWORD biSizeImage __attribute__ ((packed));
	LONG  biXPelsPerMeter __attribute__ ((packed));
	LONG  biYPelsPerMeter __attribute__ ((packed));
	DWORD biClrUsed __attribute__ ((packed));
	DWORD biClrImportant __attribute__ ((packed));
}  BITMAPINFOHEADER;

typedef struct _GUID
	{
		ULONG  Data1;
		WORD Data2;
		WORD Data3;
		BYTE  Data4[ 8 ];
} GUID;


typedef __int64 REFERENCE_TIME;

typedef struct tagVIDEOINFOHEADER {
    VXRECT rcSource;
    VXRECT rcTarget;
    DWORD dwBitRate;
    DWORD dwBitErrorRate;
    REFERENCE_TIME AvgTimePerFrame;
    BITMAPINFOHEADER bmiHeader;
} VIDEOINFOHEADER;

typedef struct tagMPEG1VIDEOINFO {
    VIDEOINFOHEADER hdr;
    DWORD dwStartTimeCode;
    DWORD cbSequenceHeader;
    BYTE bSequenceHeader[1];
} MPEG1VIDEOINFO;

typedef struct tagVIDEOINFOHEADER2 {
	VXRECT                rcSource;
	VXRECT                rcTarget;
	DWORD               dwBitRate;
	DWORD               dwBitErrorRate;
	REFERENCE_TIME      AvgTimePerFrame;
	DWORD               dwInterlaceFlags;   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
	DWORD               dwCopyProtectFlags; // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
	DWORD               dwPictAspectRatioX; // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
	DWORD               dwPictAspectRatioY; // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
	union {
		DWORD dwControlFlags;               // use AMCONTROL_* defines, use this from now on
		DWORD dwReserved1;                  // for backward compatiblity (was "must be 0";  connection rejected otherwise)
	};
	DWORD               dwReserved2;        // must be 0; reject connection otherwise
	BITMAPINFOHEADER    bmiHeader;
} VIDEOINFOHEADER2;

typedef struct tagMPEG2VIDEOINFO {
	VIDEOINFOHEADER2    hdr;
	DWORD               dwStartTimeCode;        //  ?? not used for DVD ??
	DWORD               cbSequenceHeader;       // is 0 for DVD (no sequence header)
	DWORD               dwProfile;              // use enum MPEG2Profile
	DWORD               dwLevel;                // use enum MPEG2Level
	DWORD               dwFlags;                // use AMMPEG2_* defines.  Reject connection if undefined bits are not 0
	DWORD               dwSequenceHeader[1];    // DWORD instead of Byte for alignment purposes
	//   For MPEG-2, if a sequence_header is included, the sequence_extension
	//   should also be included
} MPEG2VIDEOINFO;


typedef struct __DvInfo {
	DWORD dwDVAAuxSrc;
	DWORD dwDVAAuxCtl;
	DWORD dwDVAAuxSrc1;
	DWORD dwDVAAuxCtl1;
	DWORD dwDVVAuxSrc;
	DWORD dwDVVAuxCtl;
	DWORD dwDVReserved[2];
}DVINFO;

typedef   struct   tagJPEGINFOHEADER   {  
	/*   compression-specific   fields   */  
	/*   these   fields   are   defined   for   'JPEG'   and   'MJPG'   */  
	DWORD               JPEGSize;  
	DWORD               JPEGProcess;  
	
	/*   Process   specific   fields   */  
	DWORD               JPEGColorSpaceID;  
	DWORD               JPEGBitsPerSample;  
	DWORD               JPEGHSubSampling;  
	DWORD               JPEGVSubSampling;  
}   JPEGINFOHEADER;   


typedef struct _MainAVIHeader
	{
		DWORD	dwMicroSecPerFrame;
		DWORD	dwMaxBytesPerSec;
		DWORD	dwPaddingGranularity;
		DWORD	dwFlags;
		DWORD	dwTotalFrames;
		DWORD	dwInitialFrames;
		DWORD	dwStreams;
		DWORD	dwSuggestedBufferSize;
		DWORD	dwWidth;
		DWORD	dwHeight;
		DWORD	dwReserved[4];
	} MainAVIHeader;

#define FOURCC DWORD
#define FOURCC_RIFF     'FFIR'
#define FOURCC_LIST     'TSIL'

#define WAVE_FORMAT_PCM 1
struct tWAVEFORMATEX
{
	WORD        wFormatTag;         /* format type */
	WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
	DWORD       nSamplesPerSec;     /* sample rate */
	DWORD       nAvgBytesPerSec;    /* for buffer estimation */
	WORD        nBlockAlign;        /* block size of data */
	WORD        wBitsPerSample;     /* number of bits per sample of mono data */
	WORD        cbSize;             /* the count in bytes of the size of */
	/* extra information (after cbSize) */
}__attribute__ ((packed));

typedef struct tWAVEFORMATEX WAVEFORMATEX;
typedef WAVEFORMATEX *PWAVEFORMATEX;
typedef WAVEFORMATEX *NPWAVEFORMATEX;
typedef WAVEFORMATEX *LPWAVEFORMATEX;


//
// MPEG Layer3 WAVEFORMATEX structure
// for WAVE_FORMAT_MPEGLAYER3 (0x0055)
//
#define MPEGLAYER3_WFX_EXTRA_BYTES   12

// WAVE_FORMAT_MPEGLAYER3 format sructure
//
typedef struct mpeglayer3waveformat_tag {
	WAVEFORMATEX  wfx;
	WORD          wID;
	DWORD         fdwFlags;
	WORD          nBlockSize;
	WORD          nFramesPerBlock;
	WORD          nCodecDelay;
}__attribute__ ((packed)) MPEGLAYER3WAVEFORMAT;

typedef MPEGLAYER3WAVEFORMAT     *PMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT     *NPMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT     *LPMPEGLAYER3WAVEFORMAT;

//==========================================================================;

#define MPEGLAYER3_ID_UNKNOWN            0
#define MPEGLAYER3_ID_MPEG               1
#define MPEGLAYER3_ID_CONSTANTFRAMESIZE  2

#define MPEGLAYER3_FLAG_PADDING_ISO      0x00000000
#define MPEGLAYER3_FLAG_PADDING_ON       0x00000001
#define MPEGLAYER3_FLAG_PADDING_OFF      0x00000002

#define FromHex(n)		(((n) >= 'A') ? ((n) + 10 - 'A') : ((n) - '0'))

#define StreamFromFOURCC(fcc)	((WORD)((FromHex(LOBYTE(LOWORD(fcc))) << 4) + \
(FromHex(HIBYTE(LOWORD(fcc))))))


#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif // MAKEFOURCC

/* MMIO macros */
#define mmioFOURCC(ch0, ch1, ch2, ch3)  MAKEFOURCC(ch0, ch1, ch2, ch3)

#define ckidSTREAMHEADER        mmioFOURCC('s', 't', 'r', 'h')
#define ckidSTREAMFORMAT        mmioFOURCC('s', 't', 'r', 'f')

#define formtypeAVI             mmioFOURCC('A', 'V', 'I', ' ')
#define listtypeAVIHEADER       mmioFOURCC('h', 'd', 'r', 'l')
#define ckidAVIMAINHDR          mmioFOURCC('a', 'v', 'i', 'h')
#define listtypeSTREAMHEADER    mmioFOURCC('s', 't', 'r', 'l')
#define ckidSTREAMHEADER        mmioFOURCC('s', 't', 'r', 'h')
#define ckidSTREAMFORMAT        mmioFOURCC('s', 't', 'r', 'f')
#define ckidSTREAMHANDLERDATA   mmioFOURCC('s', 't', 'r', 'd')
#define ckidSTREAMNAME		mmioFOURCC('s', 't', 'r', 'n')

#define listtypeAVIMOVIE        mmioFOURCC('m', 'o', 'v', 'i')
#define listtypeAVIRECORD       mmioFOURCC('r', 'e', 'c', ' ')

#define ckidAVINEWINDEX         mmioFOURCC('i', 'd', 'x', '1')

/*
 ** Stream types for the <fccType> field of the stream header.
 */
#define streamtypeVIDEO         mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO         mmioFOURCC('a', 'u', 'd', 's')
#define streamtypeMIDI			mmioFOURCC('m', 'i', 'd', 's')
#define streamtypeTEXT          mmioFOURCC('t', 'x', 't', 's')



/* defines for dwFormat field of WAVEINCAPS and WAVEOUTCAPS */
#define WAVE_INVALIDFORMAT     0x00000000       /* invalid format */
#define WAVE_FORMAT_1M08       0x00000001       /* 11.025 kHz, Mono,   8-bit  */
#define WAVE_FORMAT_1S08       0x00000002       /* 11.025 kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_1M16       0x00000004       /* 11.025 kHz, Mono,   16-bit */
#define WAVE_FORMAT_1S16       0x00000008       /* 11.025 kHz, Stereo, 16-bit */
#define WAVE_FORMAT_2M08       0x00000010       /* 22.05  kHz, Mono,   8-bit  */
#define WAVE_FORMAT_2S08       0x00000020       /* 22.05  kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_2M16       0x00000040       /* 22.05  kHz, Mono,   16-bit */
#define WAVE_FORMAT_2S16       0x00000080       /* 22.05  kHz, Stereo, 16-bit */
#define WAVE_FORMAT_4M08       0x00000100       /* 44.1   kHz, Mono,   8-bit  */
#define WAVE_FORMAT_4S08       0x00000200       /* 44.1   kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_4M16       0x00000400       /* 44.1   kHz, Mono,   16-bit */
#define WAVE_FORMAT_4S16       0x00000800       /* 44.1   kHz, Stereo, 16-bit */

#define WAVE_FORMAT_44M08      0x00000100       /* 44.1   kHz, Mono,   8-bit  */
#define WAVE_FORMAT_44S08      0x00000200       /* 44.1   kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_44M16      0x00000400       /* 44.1   kHz, Mono,   16-bit */
#define WAVE_FORMAT_44S16      0x00000800       /* 44.1   kHz, Stereo, 16-bit */
#define WAVE_FORMAT_48M08      0x00001000       /* 48     kHz, Mono,   8-bit  */
#define WAVE_FORMAT_48S08      0x00002000       /* 48     kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_48M16      0x00004000       /* 48     kHz, Mono,   16-bit */
#define WAVE_FORMAT_48S16      0x00008000       /* 48     kHz, Stereo, 16-bit */
#define WAVE_FORMAT_96M08      0x00010000       /* 96     kHz, Mono,   8-bit  */
#define WAVE_FORMAT_96S08      0x00020000       /* 96     kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_96M16      0x00040000       /* 96     kHz, Mono,   16-bit */
#define WAVE_FORMAT_96S16      0x00080000       /* 96     kHz, Stereo, 16-bit */


/*
 ** Main AVI File Header
 */	

// Flags for dwFlags
#define AVIFILEINFO_HASINDEX		0x00000010
#define AVIFILEINFO_MUSTUSEINDEX	0x00000020
#define AVIFILEINFO_ISINTERLEAVED	0x00000100
#define AVIFILEINFO_WASCAPTUREFILE	0x00010000
#define AVIFILEINFO_COPYRIGHTED		0x00020000

// Flags for dwCaps
#define AVIFILECAPS_CANREAD		0x00000001
#define AVIFILECAPS_CANWRITE		0x00000002
#define AVIFILECAPS_ALLKEYFRAMES	0x00000010
#define AVIFILECAPS_NOCOMPRESSION	0x00000020

/* flags for use in <dwFlags> in AVIFileHdr */
#define AVIF_HASINDEX		0x00000010	// Index at end of file?
#define AVIF_MUSTUSEINDEX	0x00000020
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_WASCAPTUREFILE	0x00010000
#define AVIF_COPYRIGHTED	0x00020000

/* The AVI File Header LIST chunk should be padded to this size */
#define AVI_HEADERSIZE  2048                    // size of AVI header list


// -----------------------------------------------------------------------
// video format blocks
// -----------------------------------------------------------------------

enum AM_MPEG2Level {
    AM_MPEG2Level_Low = 1,
    AM_MPEG2Level_Main = 2,
    AM_MPEG2Level_High1440 = 3,
    AM_MPEG2Level_High = 4
};

enum AM_MPEG2Profile {
    AM_MPEG2Profile_Simple = 1,
    AM_MPEG2Profile_Main = 2,
    AM_MPEG2Profile_SNRScalable = 3,
    AM_MPEG2Profile_SpatiallyScalable = 4,
    AM_MPEG2Profile_High = 5
};

/* JPEGProcess Definitions */
#define JPEG_PROCESS_BASELINE           0       /* Baseline DCT */
/* JPEGColorSpaceID Definitions */
#define JPEG_Y          1       /* Y only component of YCbCr */
#define JPEG_YCbCr      2       /* YCbCr as define by CCIR 601 */
#define JPEG_RGB        3       /* 3 component RGB */

/*
 ** Stream header
 */

#define AVISF_DISABLED			0x00000001

#define AVISF_VIDEO_PALCHANGES		0x00010000

typedef struct {
	FOURCC		fccType;
	FOURCC		fccHandler;
	DWORD		dwFlags;	/* Contains AVITF_* flags */
	WORD		wPriority;
	WORD		wLanguage;
	DWORD		dwInitialFrames;
	DWORD		dwScale;	
	DWORD		dwRate;	/* dwRate / dwScale == samples/second */
	DWORD		dwStart;
	DWORD		dwLength; /* In units above... */
	DWORD		dwSuggestedBufferSize;
	DWORD		dwQuality;
	DWORD		dwSampleSize;
	VXRECT		rcFrame;
} AVIStreamHeader;

/* Flags for index */
#define AVIIF_LIST          0x00000001L // chunk is a 'LIST'
#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
#define AVIIF_FIRSTPART     0x00000020L // this frame is the start of a partial frame.
#define AVIIF_LASTPART      0x00000040L // this frame is the end of a partial frame.
#define AVIIF_MIDPART       (AVIIF_LASTPART|AVIIF_FIRSTPART)

#define AVIIF_NOTIME	    0x00000100L // this frame doesn't take any time
#define AVIIF_COMPUSE       0x0FFF0000L // these bits are for compressor use

typedef struct
	{
		DWORD		ckid;
		DWORD		dwFlags;
		DWORD		dwChunkOffset;		// Position of chunk
		DWORD		dwChunkLength;		// Length of chunk
	} AVIINDEXENTRY;



#define  WAVE_FORMAT_MPEGLAYER3                 0x0055 /* ISO/MPEG Layer3 Format Tag */

#endif

#include "vxencif.h"

#define CONST_1MSIZE		0x200000

#define AVIMUX_OPENDML		1			//Open DML AVI
#define AVIMUX_MATROXMPEG	2			//Matrox MPEG AVI
#define AVIMUX_CONSTFRAME	3			//Const framesize AVI
#define AVIMUX_MJPEG		4			//Motion-JPEG AVI

#define AVIMUX_AV			8			//VIDEO+AUDIO AVI
#define AVIMUX_AV_MPEG2		9			//Matrox MPEG+AUDIO AVI

typedef struct
{
	DWORD fcc;
	DWORD dwSize;
	DWORD dwSizeUsed;
	DWORD dwFrameIndex;
	DWORD dwReallocation;
	DWORD dwFrameType;
	PBYTE pOBMem;
}MATROXMPEG2AVI_INDEX;

typedef struct
{
	DWORD fcc;
	DWORD dwSize;
	DWORD dwSizeUsed;
	VXBOOL bKey;
	PBYTE pOBMem;
}MATROXDVAVI_INDEX;

typedef struct
{
	DWORD dwSize;
	DWORD dwSizeUsed;
	DWORD dwField0Size;
	DWORD dwField1Size;
}MATROXMJPEG_INDEX;


struct _video_field_desc
{
   ULONG    CompressedBMHeight;
   ULONG    CompressedBMWidth;
   ULONG    ValidBMHeight;          // Not used
   ULONG    ValidBMWidth;           // Not used
   ULONG    ValidBMXOffset;         // Not used
   ULONG    ValidBMYOffset;         // Not used
   ULONG    VideoXOffsetInT;        // Not used
   ULONG    VideoYValidStartLine;   // Not used
};

typedef struct 
{
	DWORD VideoFormatToken;
	DWORD VideoStandard;
	DWORD dwVerticalRefreshRate;
	DWORD dwHTotalInT;
	DWORD dwVTotalInLines;
	DWORD dwFrameAspectRatio;
	DWORD dwFrameWidthInPixels;
	DWORD dwFrameHeightInLines;
	DWORD nbFieldPerFrame;
	struct _video_field_desc* a;
}VIDEOPROPHEADER;


#pragma pack(push,2)
typedef struct _AMMediaType2 {
	GUID majortype;
	GUID subtype;
	int bFixedSizeSamples;
	int bTemporalCompression;
	ULONG lSampleSize;
	GUID formattype;
	DWORD pUnk;
	ULONG cbFormat;
	DWORD pbFormat;
} AM_MEDIA_TYPE2;

typedef struct 
{
	BITMAPINFOHEADER bmi;
	DWORD dwSizeMT;
	AM_MEDIA_TYPE2 mt;
	MPEG2VIDEOINFO info;
}MPEG2AVI_FORMAT,*LPMPEG2AVI_FORMAT;

typedef struct 
{
	BITMAPINFOHEADER bmi;
	DWORD dwSizeMT;
	AM_MEDIA_TYPE2 mt;
	MPEG1VIDEOINFO info;
}MPEG1AVI_FORMAT,*LPMPEG1AVI_FORMAT;

typedef struct 
{
	BITMAPINFOHEADER bmi;
	DVINFO info;
}DVAVI_FORMAT,*LPDVAVI_FORMAT;

typedef struct 
{
	BITMAPINFOHEADER bmi;
	DWORD dw;
	JPEGINFOHEADER info;
}MJPGAVI_FORMAT,*LPMJPGAVI_FORMAT;



typedef struct  
{
	BITMAPINFOHEADER bmp;
	DWORD scanmode;		//1:firstfield,2:secondfiled,3:Progressive
	DWORD format;		//5:keMvSurfaceFormatYUYV422,6:keMvSurfaceFormatYUYV4224
	DWORD dwBit;
	DWORD dwFieldFirst;//1 Top First,2 Bottom Fisrt,3 InterlacedFieldsInAFrame
	DWORD dwDataType;  //4 ProgressiveFrame,5 BackToBackFieldsInAFrame
	DWORD dwRowPitch;
}M101AVI_FORMAT,*LPM101AVI_FORMAT;

typedef struct {
	short	left;
	short	top;
	short	right;
	short	bottom;
} RECT16;

typedef struct {
	FOURCC		fccType;
	FOURCC		fccHandler;
	DWORD		dwFlags;
	WORD		wPriority;
	WORD		wLanguage;
	DWORD		dwInitialFrames;
	DWORD		dwScale;	
	DWORD		dwRate;
	DWORD		dwStart;
	DWORD		dwLength;
	DWORD		dwSuggestedBufferSize;
	DWORD		dwQuality;
	DWORD		dwSampleSize;
	RECT16		rcFrame;
} AVIStreamHeader_fixed;

#pragma pack(pop)

#define LIID_IMatroxAVIOutputEx	0xb0000001

vxinterface IMatroxAVIOutputEx : public IVxObject
{
	virtual VXBOOL __stdcall CreateFile(IVxSink* file) = 0;
	virtual VXBOOL __stdcall CreateFile(IVxSink* file,const AVIStreamHeader_fixed* pStreamInfo,const PBYTE pmt,DWORD mtsize,const PBYTE pVideoProp,DWORD vpsize) = 0;
	virtual VXBOOL __stdcall CloseFile(VXBOOL bDelete = FALSE) = 0;

	virtual void __stdcall SetStreamInfo(AVIStreamHeader_fixed* pStreamInfo) = 0;
	virtual PBYTE __stdcall AllocAndGetFormat(DWORD dwSize) = 0;
	virtual PBYTE __stdcall AllocAndGetVideoProp(DWORD dwSize) = 0;
	virtual void* __stdcall GetFormat() = 0;

	virtual DWORD __stdcall GetLastError() = 0;
	virtual VXBOOL __stdcall WriteSingleFrame(PBYTE pBuffer,DWORD dwSize,DWORD dwType) = 0;//MPEG2 File:[1] I frame, [2] P frame, [3] B frame;
};

LONG CreateMatroxAVIOutputEx(DWORD type,IMatroxAVIOutputEx**);


#define LIID_IOpenDMLAVIOutput	0xb0000002

vxinterface IOpenDMLAVIOutput : public IVxObject
{
	virtual void __stdcall SetAudioInfo(const AVIStreamHeader_fixed* pStreamInfo,const PBYTE pmt,DWORD mtsize) = 0;
	virtual void* __stdcall GetAudFormat() = 0;
};

LONG CreateOpenDMLAVI(IVxSink* dst,IOpenDMLAVIOutput**);

vxinterface IMatroxWAVOutput : public IVxObject
{
	virtual VXBOOL __stdcall CreateFile(IVxSink* file,BYTE* fmt,int fmtsize) = 0;
	virtual VXBOOL __stdcall SetFormat(BYTE* fmt,int fmtsize) = 0;
	virtual VXBOOL __stdcall CloseFile(VXBOOL bDelete = FALSE) = 0;

	virtual VXBOOL __stdcall WriteToFile(PBYTE pBuffer,DWORD dwSize) = 0;		//dwSize必须是磁盘Sector大小的倍数
	virtual DWORD __stdcall GetLastError() = 0;
	virtual void __stdcall AddDolbyEMetadata(BYTE* dbmd, int dbmdsize) = 0; 
};

LONG CreateMatroxWaveOutput(IMatroxWAVOutput** wav);

vxinterface IEsFileOutput
{
	virtual void __stdcall Release() = 0;

	virtual VXBOOL __stdcall CreateFile(const char* lpFileName) = 0;
	virtual VXBOOL __stdcall CloseFile(VXBOOL bDelete = FALSE) = 0;
	virtual void __stdcall DeleteLastFile() = 0;

	virtual VXBOOL __stdcall WriteToFile(PBYTE pBuffer,DWORD dwSize) = 0;		//dwSize必须是磁盘Sector大小的倍数
	virtual DWORD __stdcall GetLastError() = 0;
};
IEsFileOutput* CreateEsFileOutput();

vxinterface IEsFileOutputEx : public IEsFileOutput
{
	virtual VXBOOL __stdcall WriteToFileEx(PBYTE pBuffer,DWORD dwSize) = 0;		//dwSize没有限制
};
IEsFileOutputEx* CreateEsFileOutputEx();

typedef struct  
{
	__int64 pos;
	DWORD dwSize;
	DWORD type;
}IDXFRAME;

#define IFRAME                  1
#define PFRAME                  2
#define BFRAME                  3
#define EFRAME                  4

vxinterface IESIdxFile
{
	virtual void __stdcall Release() = 0;

	virtual VXBOOL __stdcall CreateFile(const char* lpFileName) = 0;
	virtual void __stdcall CloseFile(VXBOOL bDelete = FALSE) = 0;
	virtual void __stdcall DeleteLastFile() = 0;

	virtual void __stdcall SetStreamInfo(AVIStreamHeader_fixed* pStreamInfo) = 0;

	virtual void __stdcall AddIndex(__int64 pos,DWORD dwSize,DWORD type) = 0;
	virtual void __stdcall AddIndex(int count,IDXFRAME*) = 0;
};

IESIdxFile* CreateESIdxFile();

vxinterface IComplexFile
{
	virtual void __stdcall Release() = 0;

	virtual VXBOOL __stdcall CreateFile(IVxSink* file,LPVX_VIDEOINFO2 vinfo,LPVX_AUDIOINFO2 ainfo) = 0;
	virtual void __stdcall CloseFile() = 0;

	virtual VXBOOL __stdcall AddVideo(PBYTE data,LONG len,VXBOOL keyframe = TRUE) = 0;
	virtual VXBOOL __stdcall AddAudio(PBYTE data,LONG len,VXBOOL keyframe = TRUE) = 0;
};

typedef enum
{
	complex_RawDV,
}COMPLEXFILE_TYPE;
IComplexFile* CreateComplexFile(COMPLEXFILE_TYPE type,VXSIZE size,DWORD rate,DWORD scale);

#endif //__MXAVI_H__
