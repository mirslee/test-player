#ifndef __MXFOURCC_H__
#define __MXFOURCC_H__

#include "MxCommon.h"

typedef unsigned long MxFourcc;

enum {
    MX_CODEC_UNKNOWN         = MX_FOURCC('u','n','d','f'),
    
    /* Video codec */
    MX_CODEC_MPGV            = MX_FOURCC('m','p','g','v'),
    MX_CODEC_MP4V            = MX_FOURCC('m','p','4','v'),
    MX_CODEC_DIV1            = MX_FOURCC('D','I','V','1'),
    MX_CODEC_DIV2            = MX_FOURCC('D','I','V','2'),
    MX_CODEC_DIV3            = MX_FOURCC('D','I','V','3'),
    MX_CODEC_SVQ1            = MX_FOURCC('S','V','Q','1'),
    MX_CODEC_SVQ3            = MX_FOURCC('S','V','Q','3'),
    MX_CODEC_H264            = MX_FOURCC('h','2','6','4'),
    MX_CODEC_H263            = MX_FOURCC('h','2','6','3'),
    MX_CODEC_H263I           = MX_FOURCC('I','2','6','3'),
    MX_CODEC_H263P           = MX_FOURCC('I','L','V','R'),
    MX_CODEC_FLV1            = MX_FOURCC('F','L','V','1'),
    MX_CODEC_H261            = MX_FOURCC('h','2','6','1'),
    MX_CODEC_MJPG            = MX_FOURCC('M','J','P','G'),
    MX_CODEC_MJPGB           = MX_FOURCC('m','j','p','b'),
    MX_CODEC_LJPG            = MX_FOURCC('L','J','P','G'),
    MX_CODEC_WMV1            = MX_FOURCC('W','M','V','1'),
    MX_CODEC_WMV2            = MX_FOURCC('W','M','V','2'),
    MX_CODEC_WMV3            = MX_FOURCC('W','M','V','3'),
    MX_CODEC_WMVA            = MX_FOURCC('W','M','V','A'),
    MX_CODEC_WMVP            = MX_FOURCC('W','M','V','P'),
    MX_CODEC_WMVP2           = MX_FOURCC('W','V','P','2'),
    MX_CODEC_VC1             = MX_FOURCC('V','C','-','1'),
    MX_CODEC_DAALA           = MX_FOURCC('d','a','a','l'),
    MX_CODEC_THEORA          = MX_FOURCC('t','h','e','o'),
    MX_CODEC_TARKIN          = MX_FOURCC('t','a','r','k'),
    MX_CODEC_DIRAC           = MX_FOURCC('d','r','a','c'),
    MX_CODEC_OGGSPOTS        = MX_FOURCC('S','P','O','T'),
    MX_CODEC_CAVS            = MX_FOURCC('C','A','V','S'),
    MX_CODEC_NUV             = MX_FOURCC('N','J','P','G'),
    MX_CODEC_RV10            = MX_FOURCC('R','V','1','0'),
    MX_CODEC_RV13            = MX_FOURCC('R','V','1','3'),
    MX_CODEC_RV20            = MX_FOURCC('R','V','2','0'),
    MX_CODEC_RV30            = MX_FOURCC('R','V','3','0'),
    MX_CODEC_RV40            = MX_FOURCC('R','V','4','0'),
    MX_CODEC_VP3             = MX_FOURCC('V','P','3',' '),
    MX_CODEC_VP5             = MX_FOURCC('V','P','5',' '),
    MX_CODEC_VP6             = MX_FOURCC('V','P','6','2'),
    MX_CODEC_VP6F            = MX_FOURCC('V','P','6','F'),
    MX_CODEC_VP6A            = MX_FOURCC('V','P','6','A'),
    MX_CODEC_MSVIDEO1        = MX_FOURCC('M','S','V','C'),
    MX_CODEC_FLIC            = MX_FOURCC('F','L','I','C'),
    MX_CODEC_SP5X            = MX_FOURCC('S','P','5','X'),
    MX_CODEC_DV              = MX_FOURCC('d','v',' ',' '),
    MX_CODEC_MSRLE           = MX_FOURCC('m','r','l','e'),
    MX_CODEC_HUFFYUV         = MX_FOURCC('H','F','Y','U'),
    MX_CODEC_FFVHUFF         = MX_FOURCC('F','F','V','H'),
    MX_CODEC_ASV1            = MX_FOURCC('A','S','V','1'),
    MX_CODEC_ASV2            = MX_FOURCC('A','S','V','2'),
    MX_CODEC_FFV1            = MX_FOURCC('F','F','V','1'),
    MX_CODEC_VCR1            = MX_FOURCC('V','C','R','1'),
    MX_CODEC_CLJR            = MX_FOURCC('C','L','J','R'),
    MX_CODEC_RPZA            = MX_FOURCC('r','p','z','a'),
    MX_CODEC_SMC             = MX_FOURCC('s','m','c',' '),
    MX_CODEC_CINEPAK         = MX_FOURCC('C','V','I','D'),
    MX_CODEC_TSCC            = MX_FOURCC('T','S','C','C'),
    MX_CODEC_CSCD            = MX_FOURCC('C','S','C','D'),
    MX_CODEC_ZMBV            = MX_FOURCC('Z','M','B','V'),
    MX_CODEC_VMNC            = MX_FOURCC('V','M','n','c'),
    MX_CODEC_FMVC            = MX_FOURCC('F','M','V','C'),
    MX_CODEC_FRAPS           = MX_FOURCC('F','P','S','1'),
    MX_CODEC_TRUEMOTION1     = MX_FOURCC('D','U','C','K'),
    MX_CODEC_TRUEMOTION2     = MX_FOURCC('T','M','2','0'),
    MX_CODEC_QTRLE           = MX_FOURCC('r','l','e',' '),
    MX_CODEC_QDRAW           = MX_FOURCC('q','d','r','w'),
    MX_CODEC_QPEG            = MX_FOURCC('Q','P','E','G'),
    MX_CODEC_ULTI            = MX_FOURCC('U','L','T','I'),
    MX_CODEC_VIXL            = MX_FOURCC('V','I','X','L'),
    MX_CODEC_LOCO            = MX_FOURCC('L','O','C','O'),
    MX_CODEC_WNV1            = MX_FOURCC('W','N','V','1'),
    MX_CODEC_AASC            = MX_FOURCC('A','A','S','C'),
    MX_CODEC_INDEO2          = MX_FOURCC('I','V','2','0'),
    MX_CODEC_INDEO3          = MX_FOURCC('I','V','3','1'),
    MX_CODEC_INDEO4          = MX_FOURCC('I','V','4','1'),
    MX_CODEC_INDEO5          = MX_FOURCC('I','V','5','0'),
    MX_CODEC_FLASHSV         = MX_FOURCC('F','S','V','1'),
    MX_CODEC_KMVC            = MX_FOURCC('K','M','V','C'),
    MX_CODEC_SMACKVIDEO      = MX_FOURCC('S','M','K','2'),
    MX_CODEC_DNXHD           = MX_FOURCC('A','V','d','n'),
    MX_CODEC_8BPS            = MX_FOURCC('8','B','P','S'),
    MX_CODEC_MIMIC           = MX_FOURCC('M','L','2','O'),
    MX_CODEC_INTERPLAY       = MX_FOURCC('i','m','v','e'),
    MX_CODEC_IDCIN           = MX_FOURCC('I','D','C','I'),
    MX_CODEC_4XM             = MX_FOURCC('4','X','M','V'),
    MX_CODEC_ROQ             = MX_FOURCC('R','o','Q','v'),
    MX_CODEC_MDEC            = MX_FOURCC('M','D','E','C'),
    MX_CODEC_VMDVIDEO        = MX_FOURCC('V','M','D','V'),
    MX_CODEC_CDG             = MX_FOURCC('C','D','G',' '),
    MX_CODEC_FRWU            = MX_FOURCC('F','R','W','U'),
    MX_CODEC_AMV             = MX_FOURCC('A','M','V',' '),
    MX_CODEC_VP7             = MX_FOURCC('V','P','7','0'),
    MX_CODEC_VP8             = MX_FOURCC('V','P','8','0'),
    MX_CODEC_VP9             = MX_FOURCC('V','P','9','0'),
    MX_CODEC_VP10            = MX_FOURCC('V','P',':','0'),
    MX_CODEC_AV1             = MX_FOURCC('a','v','0','1'),
    MX_CODEC_JPEG2000        = MX_FOURCC('J','P','2','K'),
    MX_CODEC_LAGARITH        = MX_FOURCC('L','A','G','S'),
    MX_CODEC_FLASHSV2        = MX_FOURCC('F','S','V','2'),
    MX_CODEC_PRORES          = MX_FOURCC('a','p','c','n'),
    MX_CODEC_MXPEG           = MX_FOURCC('M','X','P','G'),
    MX_CODEC_CDXL            = MX_FOURCC('C','D','X','L'),
    MX_CODEC_BMVVIDEO        = MX_FOURCC('B','M','V','V'),
    MX_CODEC_UTVIDEO         = MX_FOURCC('U','L','R','A'),
    MX_CODEC_VBLE            = MX_FOURCC('V','B','L','E'),
    MX_CODEC_DXTORY          = MX_FOURCC('x','t','o','r'),
    MX_CODEC_MSS1            = MX_FOURCC('M','S','S','1'),
    MX_CODEC_MSS2            = MX_FOURCC('M','S','S','2'),
    MX_CODEC_MSA1            = MX_FOURCC('M','S','A','1'),
    MX_CODEC_TSC2            = MX_FOURCC('T','S','C','2'),
    MX_CODEC_MTS2            = MX_FOURCC('M','T','S','2'),
    MX_CODEC_HEVC            = MX_FOURCC('h','e','v','c'),
    MX_CODEC_ICOD            = MX_FOURCC('i','c','o','d'),
    MX_CODEC_G2M2            = MX_FOURCC('G','2','M','2'),
    MX_CODEC_G2M3            = MX_FOURCC('G','2','M','3'),
    MX_CODEC_G2M4            = MX_FOURCC('G','2','M','4'),
    MX_CODEC_BINKVIDEO       = MX_FOURCC('B','I','K','f'),
    MX_CODEC_BINKAUDIO_DCT   = MX_FOURCC('B','A','U','1'),
    MX_CODEC_BINKAUDIO_RDFT  = MX_FOURCC('B','A','U','2'),
    MX_CODEC_XAN_WC4         = MX_FOURCC('X','x','a','n'),
    MX_CODEC_LCL_MSZH        = MX_FOURCC('M','S','Z','H'),
    MX_CODEC_LCL_ZLIB        = MX_FOURCC('Z','L','I','B'),
    MX_CODEC_THP             = MX_FOURCC('T','H','P','0'),
    MX_CODEC_ESCAPE124       = MX_FOURCC('E','1','2','4'),
    MX_CODEC_KGV1            = MX_FOURCC('K','G','V','1'),
    MX_CODEC_CLLC            = MX_FOURCC('C','L','L','C'),
    MX_CODEC_AURA            = MX_FOURCC('A','U','R','A'),
    MX_CODEC_FIC             = MX_FOURCC('F','I','C','V'),
    MX_CODEC_TMV             = MX_FOURCC('T','M','A','V'),
    MX_CODEC_XAN_WC3         = MX_FOURCC('X','A','N','3'),
    MX_CODEC_WS_VQA          = MX_FOURCC('W','V','Q','A'),
    MX_CODEC_MMVIDEO         = MX_FOURCC('M','M','V','I'),
    MX_CODEC_AVS             = MX_FOURCC('A','V','S','V'),
    MX_CODEC_DSICINVIDEO     = MX_FOURCC('D','C','I','V'),
    MX_CODEC_TIERTEXSEQVIDEO = MX_FOURCC('T','S','E','Q'),
    MX_CODEC_DXA             = MX_FOURCC('D','E','X','A'),
    MX_CODEC_C93             = MX_FOURCC('I','C','9','3'),
    MX_CODEC_BETHSOFTVID     = MX_FOURCC('B','V','I','D'),
    MX_CODEC_VB              = MX_FOURCC('V','B','V','1'),
    MX_CODEC_RL2             = MX_FOURCC('R','L','V','2'),
    MX_CODEC_BFI             = MX_FOURCC('B','F','&','I'),
    MX_CODEC_CMV             = MX_FOURCC('E','C','M','V'),
    MX_CODEC_MOTIONPIXELS    = MX_FOURCC('M','P','I','X'),
    MX_CODEC_TGV             = MX_FOURCC('T','G','V','V'),
    MX_CODEC_TGQ             = MX_FOURCC('T','G','Q','V'),
    MX_CODEC_TQI             = MX_FOURCC('T','Q','I','V'),
    MX_CODEC_MAD             = MX_FOURCC('M','A','D','V'),
    MX_CODEC_ANM             = MX_FOURCC('A','N','I','M'),
    MX_CODEC_YOP             = MX_FOURCC('Y','O','P','V'),
    MX_CODEC_JV              = MX_FOURCC('J','V','0','0'),
    MX_CODEC_DFA             = MX_FOURCC('D','F','I','A'),
    MX_CODEC_HNM4_VIDEO      = MX_FOURCC('H','N','M','4'),
    MX_CODEC_TDSC            = MX_FOURCC('T','D','S','C'),
    MX_CODEC_HQX             = MX_FOURCC('C','H','Q','X'),
    MX_CODEC_HQ_HQA          = MX_FOURCC('C','U','V','C'),
    MX_CODEC_HAP             = MX_FOURCC('H','A','P','1'),
    MX_CODEC_DXV             = MX_FOURCC('D','X','D','3'),
    MX_CODEC_CINEFORM        = MX_FOURCC('C','F','H','D'),
    MX_CODEC_SPEEDHQ         = MX_FOURCC('S','H','Q','2'),
    MX_CODEC_PIXLET          = MX_FOURCC('p','x','l','t'),
    
    /***********
     * Chromas
     ***********/
    
    /* Planar YUV */
    
    /* Planar YUV 4:1:0 Y:V:U */
    MX_CODEC_YV9             = MX_FOURCC('Y','V','U','9'),
    /* Planar YUV 4:1:0 Y:U:V */
    MX_CODEC_I410            = MX_FOURCC('I','4','1','0'),
    /* Planar YUV 4:1:1 Y:U:V */
    MX_CODEC_I411            = MX_FOURCC('I','4','1','1'),
    
    /* Planar YUV 4:2:0 Y:V:U */
    MX_CODEC_YV12            = MX_FOURCC('Y','V','1','2'),
    /* Planar YUV 4:2:0 Y:U:V 8-bit */
    MX_CODEC_I420            = MX_FOURCC('I','4','2','0'),
    /* Planar YUV 4:2:0 Y:U:V  9-bit stored on 16 bits */
    MX_CODEC_I420_9L         = MX_FOURCC('I','0','9','L'),
    MX_CODEC_I420_9B         = MX_FOURCC('I','0','9','B'),
    /* Planar YUV 4:2:0 Y:U:V 10-bit stored on 16 bits LSB */
    MX_CODEC_I420_10L        = MX_FOURCC('I','0','A','L'),
    MX_CODEC_I420_10B        = MX_FOURCC('I','0','A','B'),
    /* Planar YUV 4:2:0 Y:U:V 12-bit stored on 16 bits */
    MX_CODEC_I420_12L        = MX_FOURCC('I','0','C','L'),
    MX_CODEC_I420_12B        = MX_FOURCC('I','0','C','B'),
    
    /* Planar YUV 4:2:0 Y:U:V 16-bit stored on 16 bits */
    MX_CODEC_I420_16L        = MX_FOURCC('I','0','F','L'),
    MX_CODEC_I420_16B        = MX_FOURCC('I','0','F','B'),
    
    /* Planar YUV 4:2:2 Y:U:V 8-bit */
    MX_CODEC_I422            = MX_FOURCC('I','4','2','2'),
    /* Planar YUV 4:2:2 Y:U:V  9-bit stored on 16 bits */
    MX_CODEC_I422_9L         = MX_FOURCC('I','2','9','L'),
    MX_CODEC_I422_9B         = MX_FOURCC('I','2','9','B'),
    /* Planar YUV 4:2:2 Y:U:V 10-bit stored on 16 bits */
    MX_CODEC_I422_10L        = MX_FOURCC('I','2','A','L'),
    MX_CODEC_I422_10B        = MX_FOURCC('I','2','A','B'),
    /* Planar YUV 4:2:2 Y:U:V 12-bit stored on 16 bits */
    MX_CODEC_I422_12L        = MX_FOURCC('I','2','C','L'),
    MX_CODEC_I422_12B        = MX_FOURCC('I','2','C','B'),
    
    /* Planar YUV 4:4:0 Y:U:V */
    MX_CODEC_I440            = MX_FOURCC('I','4','4','0'),
    /* Planar YUV 4:4:4 Y:U:V 8-bit */
    MX_CODEC_I444            = MX_FOURCC('I','4','4','4'),
    /* Planar YUV 4:4:4 Y:U:V  9-bit stored on 16 bits */
    MX_CODEC_I444_9L         = MX_FOURCC('I','4','9','L'),
    MX_CODEC_I444_9B         = MX_FOURCC('I','4','9','B'),
    /* Planar YUV 4:4:4 Y:U:V 10-bit stored on 16 bits */
    MX_CODEC_I444_10L        = MX_FOURCC('I','4','A','L'),
    MX_CODEC_I444_10B        = MX_FOURCC('I','4','A','B'),
    /* Planar YUV 4:4:4 Y:U:V 12-bit stored on 16 bits */
    MX_CODEC_I444_12L        = MX_FOURCC('I','4','C','L'),
    MX_CODEC_I444_12B        = MX_FOURCC('I','4','C','B'),
    /* Planar YUV 4:4:4 Y:U:V 16-bit */
    MX_CODEC_I444_16L        = MX_FOURCC('I','4','F','L'),
    MX_CODEC_I444_16B        = MX_FOURCC('I','4','F','B'),
    
    /* Planar YUV 4:2:0 Y:U:V full scale */
    MX_CODEC_J420            = MX_FOURCC('J','4','2','0'),
    /* Planar YUV 4:2:2 Y:U:V full scale */
    MX_CODEC_J422            = MX_FOURCC('J','4','2','2'),
    /* Planar YUV 4:4:0 Y:U:V full scale */
    MX_CODEC_J440            = MX_FOURCC('J','4','4','0'),
    /* Planar YUV 4:4:4 Y:U:V full scale */
    MX_CODEC_J444            = MX_FOURCC('J','4','4','4'),
    /* Palettized YUV with palette element Y:U:V:A */
    MX_CODEC_YUVP            = MX_FOURCC('Y','U','V','P'),
    
    /* Planar YUV 4:4:4 Y:U:V:A */
    MX_CODEC_YUVA            = MX_FOURCC('Y','U','V','A'),
    /* Planar YUV 4:2:2 Y:U:V:A */
    MX_CODEC_YUV422A         = MX_FOURCC('I','4','2','A'),
    /* Planar YUV 4:2:0 Y:U:V:A */
    MX_CODEC_YUV420A         = MX_FOURCC('I','4','0','A'),
    
    /* Planar Y:U:V:A 4:4:4 10bits */
    MX_CODEC_YUVA_444_10L    = MX_FOURCC('Y','A','0','L'),
    MX_CODEC_YUVA_444_10B    = MX_FOURCC('Y','A','0','B'),
    
    /* Semi-planar Y/UV */
    
    /* 2 planes Y/UV 4:2:0 */
    MX_CODEC_NV12            = MX_FOURCC('N','V','1','2'),
    /* 2 planes Y/VU 4:2:0 */
    MX_CODEC_NV21            = MX_FOURCC('N','V','2','1'),
    /* 2 planes Y/UV 4:2:2 */
    MX_CODEC_NV16            = MX_FOURCC('N','V','1','6'),
    /* 2 planes Y/VU 4:2:2 */
    MX_CODEC_NV61            = MX_FOURCC('N','V','6','1'),
    /* 2 planes Y/UV 4:4:4 */
    MX_CODEC_NV24            = MX_FOURCC('N','V','2','4'),
    /* 2 planes Y/VU 4:4:4 */
    MX_CODEC_NV42            = MX_FOURCC('N','V','4','2'),
    /* 2 planes Y/UV 4:2:0 10-bit */
    MX_CODEC_P010            = MX_FOURCC('P','0','1','0'),
    
    /* Packed YUV */
    
    /* Packed YUV 4:2:2, U:Y:V:Y */
    MX_CODEC_UYVY            = MX_FOURCC('U','Y','V','Y'),
    /* Packed YUV 4:2:2, V:Y:U:Y */
    MX_CODEC_VYUY            = MX_FOURCC('V','Y','U','Y'),
    /* Packed YUV 4:2:2, Y:U:Y:V */
    MX_CODEC_YUYV            = MX_FOURCC('Y','U','Y','2'),
    /* Packed YUV 4:2:2, Y:V:Y:U */
    MX_CODEC_YVYU            = MX_FOURCC('Y','V','Y','U'),
    /* Packed YUV 2:1:1, Y:U:Y:V */
    MX_CODEC_Y211            = MX_FOURCC('Y','2','1','1'),
    /* Packed YUV 4:2:2, U:Y:V:Y, reverted */
    MX_CODEC_CYUV            = MX_FOURCC('c','y','u','v'),
    /* 10-bit 4:2:2 Component YCbCr */
    MX_CODEC_V210            = MX_FOURCC('v','2','1','0'),
    /* I420 packed for RTP (RFC 4175) */
    MX_CODEC_R420            = MX_FOURCC('r','4','2','0'),
    
    
    /* RGB */
    
    /* Palettized RGB with palette element R:G:B */
    MX_CODEC_RGBP            = MX_FOURCC('R','G','B','P'),
    /* 8 bits RGB */
    MX_CODEC_RGB8            = MX_FOURCC('R','G','B','8'),
    /* 12 bits RGB padded to 16 bits */
    MX_CODEC_RGB12           = MX_FOURCC('R','V','1','2'),
    /* 15 bits RGB padded to 16 bits */
    MX_CODEC_RGB15           = MX_FOURCC('R','V','1','5'),
    /* 16 bits RGB */
    MX_CODEC_RGB16           = MX_FOURCC('R','V','1','6'),
    /* 24 bits RGB */
    MX_CODEC_RGB24           = MX_FOURCC('R','V','2','4'),
    /* 24 bits RGB padded to 32 bits */
    MX_CODEC_RGB32           = MX_FOURCC('R','V','3','2'),
    /* 32 bits RGBA */
    MX_CODEC_RGBA            = MX_FOURCC('R','G','B','A'),
    /* 32 bits ARGB */
    MX_CODEC_ARGB            = MX_FOURCC('A','R','G','B'),
    /* 32 bits BGRA */
    MX_CODEC_BGRA            = MX_FOURCC('B','G','R','A'),
    
    /* Planar GBR 4:4:4 8 bits */
    MX_CODEC_GBR_PLANAR      = MX_FOURCC('G','B','R','8'),
    MX_CODEC_GBR_PLANAR_9B   = MX_FOURCC('G','B','9','B'),
    MX_CODEC_GBR_PLANAR_9L   = MX_FOURCC('G','B','9','L'),
    MX_CODEC_GBR_PLANAR_10B  = MX_FOURCC('G','B','A','B'),
    MX_CODEC_GBR_PLANAR_10L  = MX_FOURCC('G','B','A','L'),
    MX_CODEC_GBR_PLANAR_16L  = MX_FOURCC('G','B','F','L'),
    MX_CODEC_GBR_PLANAR_16B  = MX_FOURCC('G','B','F','B'),
    
    /* 8 bits grey */
    MX_CODEC_GREY            = MX_FOURCC('G','R','E','Y'),
    
    /* VDPAU video surface YCbCr 4:2:0 */
    MX_CODEC_VDPAU_VIDEO_420 = MX_FOURCC('V','D','V','0'),
    /* VDPAU video surface YCbCr 4:2:2 */
    MX_CODEC_VDPAU_VIDEO_422 = MX_FOURCC('V','D','V','2'),
    /* VDPAU video surface YCbCr 4:4:4 */
    MX_CODEC_VDPAU_VIDEO_444 = MX_FOURCC('V','D','V','4'),
    /* VDPAU output surface RGBA */
    MX_CODEC_VDPAU_OUTPUT    = MX_FOURCC('V','D','O','R'),
    
    /* VAAPI opaque surface */
    MX_CODEC_VAAPI_420 = MX_FOURCC('V','A','O','P'), /* 4:2:0  8 bpc */
    MX_CODEC_VAAPI_420_10BPP = MX_FOURCC('V','A','O','0'), /* 4:2:0 10 bpc */
    
    /* MediaCodec/IOMX opaque buffer type */
    MX_CODEC_ANDROID_OPAQUE  = MX_FOURCC('A','N','O','P'),
    
    /* Broadcom MMAL opaque buffer type */
    MX_CODEC_MMAL_OPAQUE     = MX_FOURCC('M','M','A','L'),
    
    /* DXVA2 opaque video surface for use with D3D9 */
    MX_CODEC_D3D9_OPAQUE     = MX_FOURCC('D','X','A','9'), /* 4:2:0  8 bpc */
    MX_CODEC_D3D9_OPAQUE_10B = MX_FOURCC('D','X','A','0'), /* 4:2:0 10 bpc */
    
    /* D3D11VA opaque video surface for use with D3D11 */
    MX_CODEC_D3D11_OPAQUE          = MX_FOURCC('D','X','1','1'), /* 4:2:0  8 bpc */
    MX_CODEC_D3D11_OPAQUE_10B      = MX_FOURCC('D','X','1','0'), /* 4:2:0 10 bpc */
    
    /* CVPixelBuffer opaque buffer type */
    MX_CODEC_CVPX_NV12       = MX_FOURCC('C','V','P','N'),
    MX_CODEC_CVPX_UYVY       = MX_FOURCC('C','V','P','Y'),
    MX_CODEC_CVPX_I420       = MX_FOURCC('C','V','P','I'),
    MX_CODEC_CVPX_BGRA       = MX_FOURCC('C','V','P','B'),
    MX_CODEC_CVPX_P010       = MX_FOURCC('C','V','P','P'),
    
    /* Image codec (video) */
    MX_CODEC_PNG             = MX_FOURCC('p','n','g',' '),
    MX_CODEC_PPM             = MX_FOURCC('p','p','m',' '),
    MX_CODEC_PGM             = MX_FOURCC('p','g','m',' '),
    MX_CODEC_PGMYUV          = MX_FOURCC('p','g','m','y'),
    MX_CODEC_PAM             = MX_FOURCC('p','a','m',' '),
    MX_CODEC_JPEG            = MX_FOURCC('j','p','e','g'),
    MX_CODEC_BPG             = MX_FOURCC('B','P','G',0xFB),
    MX_CODEC_JPEGLS          = MX_FOURCC('M','J','L','S'),
    MX_CODEC_BMP             = MX_FOURCC('b','m','p',' '),
    MX_CODEC_TIFF            = MX_FOURCC('t','i','f','f'),
    MX_CODEC_GIF             = MX_FOURCC('g','i','f',' '),
    MX_CODEC_TARGA           = MX_FOURCC('t','g','a',' '),
    MX_CODEC_SVG             = MX_FOURCC('s','v','g',' '),
    MX_CODEC_SGI             = MX_FOURCC('s','g','i',' '),
    MX_CODEC_PNM             = MX_FOURCC('p','n','m',' '),
    MX_CODEC_PCX             = MX_FOURCC('p','c','x',' '),
    MX_CODEC_XWD             = MX_FOURCC('X','W','D',' '),
    MX_CODEC_TXD             = MX_FOURCC('T','X','D',' '),
    
    
    /* Audio codec */
    MX_CODEC_MPGA                       = MX_FOURCC('m','p','g','a'),
    MX_CODEC_MP4A                       = MX_FOURCC('m','p','4','a'),
    MX_CODEC_ALS                        = MX_FOURCC('a','l','s',' '),
    MX_CODEC_A52                        = MX_FOURCC('a','5','2',' '),
    MX_CODEC_EAC3                       = MX_FOURCC('e','a','c','3'),
    MX_CODEC_DTS                        = MX_FOURCC('d','t','s',' '),
    MX_CODEC_WMA1                       = MX_FOURCC('W','M','A','1'),
    MX_CODEC_WMA2                       = MX_FOURCC('W','M','A','2'),
    MX_CODEC_WMAP                       = MX_FOURCC('W','M','A','P'),
    MX_CODEC_WMAL                       = MX_FOURCC('W','M','A','L'),
    MX_CODEC_WMAS                       = MX_FOURCC('W','M','A','S'),
    MX_CODEC_FLAC                       = MX_FOURCC('f','l','a','c'),
    MX_CODEC_MLP                        = MX_FOURCC('m','l','p',' '),
    MX_CODEC_TRUEHD                     = MX_FOURCC('t','r','h','d'),
    MX_CODEC_DVAUDIO                    = MX_FOURCC('d','v','a','u'),
    MX_CODEC_SPEEX                      = MX_FOURCC('s','p','x',' '),
    MX_CODEC_OPUS                       = MX_FOURCC('O','p','u','s'),
    MX_CODEC_VORBIS                     = MX_FOURCC('v','o','r','b'),
    MX_CODEC_MACE3                      = MX_FOURCC('M','A','C','3'),
    MX_CODEC_MACE6                      = MX_FOURCC('M','A','C','6'),
    MX_CODEC_MUSEPACK7                  = MX_FOURCC('M','P','C',' '),
    MX_CODEC_MUSEPACK8                  = MX_FOURCC('M','P','C','K'),
    MX_CODEC_RA_144                     = MX_FOURCC('1','4','_','4'),
    MX_CODEC_RA_288                     = MX_FOURCC('2','8','_','8'),
    MX_CODEC_INTERPLAY_DPCM             = MX_FOURCC('i','d','p','c'),
    MX_CODEC_ROQ_DPCM                   = MX_FOURCC('R','o','Q','a'),
    MX_CODEC_DSICINAUDIO                = MX_FOURCC('D','C','I','A'),
    MX_CODEC_ADPCM_4XM                  = MX_FOURCC('4','x','m','a'),
    MX_CODEC_ADPCM_EA                   = MX_FOURCC('A','D','E','A'),
    MX_CODEC_ADPCM_XA                   = MX_FOURCC('x','a',' ',' '),
    MX_CODEC_ADPCM_ADX                  = MX_FOURCC('a','d','x',' '),
    MX_CODEC_ADPCM_IMA_WS               = MX_FOURCC('A','I','W','S'),
    MX_CODEC_ADPCM_G722                 = MX_FOURCC('g','7','2','2'),
    MX_CODEC_ADPCM_G726                 = MX_FOURCC('g','7','2','6'),
    MX_CODEC_ADPCM_SWF                  = MX_FOURCC('S','W','F','a'),
    MX_CODEC_ADPCM_MS                   = MX_FOURCC('m','s',0x00,0x02),
    MX_CODEC_ADPCM_IMA_WAV              = MX_FOURCC('m','s',0x00,0x11),
    MX_CODEC_ADPCM_IMA_AMV              = MX_FOURCC('i','m','a','v'),
    MX_CODEC_ADPCM_IMA_QT               = MX_FOURCC('i','m','a','4'),
    MX_CODEC_ADPCM_YAMAHA               = MX_FOURCC('m','s',0x00,0x20),
    MX_CODEC_ADPCM_DK3                  = MX_FOURCC('m','s',0x00,0x62),
    MX_CODEC_ADPCM_DK4                  = MX_FOURCC('m','s',0x00,0x61),
    MX_CODEC_ADPCM_CREATIVE             = MX_FOURCC('m','s',0x00,0xC0),
    MX_CODEC_ADPCM_SBPRO_2              = MX_FOURCC('m','s',0x00,0xC2),
    MX_CODEC_ADPCM_SBPRO_3              = MX_FOURCC('m','s',0x00,0xC3),
    MX_CODEC_ADPCM_SBPRO_4              = MX_FOURCC('m','s',0x00,0xC4),
    MX_CODEC_ADPCM_THP                  = MX_FOURCC('T','H','P','A'),
    MX_CODEC_ADPCM_XA_EA                = MX_FOURCC('X','A','J', 0),
    MX_CODEC_G723_1                     = MX_FOURCC('g','7','2', 0x31),
    MX_CODEC_G729                       = MX_FOURCC('g','7','2','9'),
    MX_CODEC_VMDAUDIO                   = MX_FOURCC('v','m','d','a'),
    MX_CODEC_AMR_NB                     = MX_FOURCC('s','a','m','r'),
    MX_CODEC_AMR_WB                     = MX_FOURCC('s','a','w','b'),
    MX_CODEC_ALAC                       = MX_FOURCC('a','l','a','c'),
    MX_CODEC_QDM2                       = MX_FOURCC('Q','D','M','2'),
    MX_CODEC_QDMC                       = MX_FOURCC('Q','D','M','C'),
    MX_CODEC_COOK                       = MX_FOURCC('c','o','o','k'),
    MX_CODEC_SIPR                       = MX_FOURCC('s','i','p','r'),
    MX_CODEC_TTA                        = MX_FOURCC('T','T','A','1'),
    MX_CODEC_SHORTEN                    = MX_FOURCC('s','h','n',' '),
    MX_CODEC_WAVPACK                    = MX_FOURCC('W','V','P','K'),
    MX_CODEC_GSM                        = MX_FOURCC('g','s','m',' '),
    MX_CODEC_GSM_MS                     = MX_FOURCC('a','g','s','m'),
    MX_CODEC_ATRAC1                     = MX_FOURCC('a','t','r','1'),
    MX_CODEC_ATRAC3                     = MX_FOURCC('a','t','r','c'),
    MX_CODEC_ATRAC3P                    = MX_FOURCC('a','t','r','p'),
    MX_CODEC_IMC                        = MX_FOURCC(0x1,0x4,0x0,0x0),
    MX_CODEC_TRUESPEECH                 = MX_FOURCC(0x22,0x0,0x0,0x0),
    MX_CODEC_NELLYMOSER                 = MX_FOURCC('N','E','L','L'),
    MX_CODEC_APE                        = MX_FOURCC('A','P','E',' '),
    MX_CODEC_QCELP                      = MX_FOURCC('Q','c','l','p'),
    MX_CODEC_302M                       = MX_FOURCC('3','0','2','m'),
    MX_CODEC_DVD_LPCM                   = MX_FOURCC('l','p','c','m'),
    MX_CODEC_DVDA_LPCM                  = MX_FOURCC('a','p','c','m'),
    MX_CODEC_BD_LPCM                    = MX_FOURCC('b','p','c','m'),
    MX_CODEC_WIDI_LPCM                  = MX_FOURCC('w','p','c','m'),
    MX_CODEC_SDDS                       = MX_FOURCC('s','d','d','s'),
    MX_CODEC_MIDI                       = MX_FOURCC('M','I','D','I'),
    MX_CODEC_RALF                       = MX_FOURCC('R','A','L','F'),
    
    MX_CODEC_S8                         = MX_FOURCC('s','8',' ',' '),
    MX_CODEC_U8                         = MX_FOURCC('u','8',' ',' '),
    MX_CODEC_S16L                       = MX_FOURCC('s','1','6','l'),
    MX_CODEC_S16L_PLANAR                = MX_FOURCC('s','1','l','p'),
    MX_CODEC_S16B                       = MX_FOURCC('s','1','6','b'),
    MX_CODEC_U16L                       = MX_FOURCC('u','1','6','l'),
    MX_CODEC_U16B                       = MX_FOURCC('u','1','6','b'),
    MX_CODEC_S20B                       = MX_FOURCC('s','2','0','b'),
    MX_CODEC_S24L                       = MX_FOURCC('s','2','4','l'),
    MX_CODEC_S24B                       = MX_FOURCC('s','2','4','b'),
    MX_CODEC_U24L                       = MX_FOURCC('u','2','4','l'),
    MX_CODEC_U24B                       = MX_FOURCC('u','2','4','b'),
    MX_CODEC_S24L32                     = MX_FOURCC('s','2','4','4'),
    MX_CODEC_S24B32                     = MX_FOURCC('S','2','4','4'),
    MX_CODEC_S32L                       = MX_FOURCC('s','3','2','l'),
    MX_CODEC_S32B                       = MX_FOURCC('s','3','2','b'),
    MX_CODEC_U32L                       = MX_FOURCC('u','3','2','l'),
    MX_CODEC_U32B                       = MX_FOURCC('u','3','2','b'),
    MX_CODEC_F32L                       = MX_FOURCC('f','3','2','l'),
    MX_CODEC_F32B                       = MX_FOURCC('f','3','2','b'),
    MX_CODEC_F64L                       = MX_FOURCC('f','6','4','l'),
    MX_CODEC_F64B                       = MX_FOURCC('f','6','4','b'),
    
    MX_CODEC_ALAW                       = MX_FOURCC('a','l','a','w'),
    MX_CODEC_MULAW                      = MX_FOURCC('m','l','a','w'),
    MX_CODEC_DAT12                      = MX_FOURCC('L','P','1','2'),
    MX_CODEC_S24DAUD                    = MX_FOURCC('d','a','u','d'),
    MX_CODEC_TWINVQ                     = MX_FOURCC('T','W','I','N'),
    MX_CODEC_BMVAUDIO                   = MX_FOURCC('B','M','V','A'),
    MX_CODEC_ULEAD_DV_AUDIO_NTSC        = MX_FOURCC('m','s',0x02,0x15),
    MX_CODEC_ULEAD_DV_AUDIO_PAL         = MX_FOURCC('m','s',0x02,0x16),
    MX_CODEC_INDEO_AUDIO                = MX_FOURCC('m','s',0x04,0x02),
    MX_CODEC_METASOUND                  = MX_FOURCC('m','s',0x00,0x75),
    MX_CODEC_ON2AVC                     = MX_FOURCC('m','s',0x05,0x00),
    MX_CODEC_TAK                        = MX_FOURCC('t','a','k',' '),
    MX_CODEC_SMACKAUDIO                 = MX_FOURCC('S','M','K','A'),
    MX_CODEC_ADPCM_IMA_EA_SEAD          = MX_FOURCC('S','E','A','D'),
    MX_CODEC_ADPCM_EA_R1                = MX_FOURCC('E','A','R','1'),
    MX_CODEC_ADPCM_IMA_APC              = MX_FOURCC('A','I','P','C'),
    
    /* Subtitle */
    MX_CODEC_SPU       = MX_FOURCC('s','p','u',' '),
    MX_CODEC_DVBS      = MX_FOURCC('d','v','b','s'),
    MX_CODEC_SUBT      = MX_FOURCC('s','u','b','t'),
    MX_CODEC_XSUB      = MX_FOURCC('X','S','U','B'),
    MX_CODEC_SSA       = MX_FOURCC('s','s','a',' '),
    MX_CODEC_TEXT      = MX_FOURCC('T','E','X','T'),
    MX_CODEC_TELETEXT  = MX_FOURCC('t','e','l','x'),
    MX_CODEC_KATE      = MX_FOURCC('k','a','t','e'),
    MX_CODEC_CMML      = MX_FOURCC('c','m','m','l'),
    MX_CODEC_ITU_T140  = MX_FOURCC('t','1','4','0'),
    MX_CODEC_USF       = MX_FOURCC('u','s','f',' '),
    MX_CODEC_OGT       = MX_FOURCC('o','g','t',' '),
    MX_CODEC_CVD       = MX_FOURCC('c','v','d',' '),
    MX_CODEC_TX3G      = MX_FOURCC('t','x','3','g'),
    MX_CODEC_ARIB_A    = MX_FOURCC('a','r','b','a'),
    MX_CODEC_ARIB_C    = MX_FOURCC('a','r','b','c'),
    /* Blu-ray Presentation Graphics */
    MX_CODEC_BD_PG     = MX_FOURCC('b','d','p','g'),
    MX_CODEC_BD_TEXT   = MX_FOURCC('b','d','t','x'),
    /* EBU STL (TECH. 3264-E) */
    MX_CODEC_EBU_STL   = MX_FOURCC('S','T','L',' '),
    MX_CODEC_SCTE_18   = MX_FOURCC('S','C','1','8'),
    MX_CODEC_SCTE_27   = MX_FOURCC('S','C','2','7'),
    /* EIA/CEA-608/708 */
    MX_CODEC_CEA608    = MX_FOURCC('c','6','0','8'),
    MX_CODEC_CEA708    = MX_FOURCC('c','7','0','8'),
    MX_CODEC_TTML      = MX_FOURCC('T','T','M','L'),
    MX_CODEC_WEBVTT    = MX_FOURCC('w','v','t','t'),
    
    /* XYZ colorspace 12 bits packed in 16 bits, organisation |XXX0|YYY0|ZZZ0| */
    MX_CODEC_XYZ12     = MX_FOURCC('X','Y','1','2'),
    
    
    /* Special endian dependent values
     * The suffic N means Native
     * The suffix I means Inverted (ie non native) */
#ifdef WORDS_BIGENDIAN
    MX_CODEC_S16N       = MX_CODEC_S16B,
    MX_CODEC_U16N       = MX_CODEC_U16B,
    MX_CODEC_S24N       = MX_CODEC_S24B,
    MX_CODEC_U24N       = MX_CODEC_U24B,
    MX_CODEC_S32N       = MX_CODEC_S32B,
    MX_CODEC_U32N       = MX_CODEC_U32B,
    MX_CODEC_FL32       = MX_CODEC_F32B,
    MX_CODEC_FL64       = MX_CODEC_F64B,
    
    MX_CODEC_S16I       = MX_CODEC_S16L,
    MX_CODEC_U16I       = MX_CODEC_U16L,
    MX_CODEC_S24I       = MX_CODEC_S24L,
    MX_CODEC_U24I       = MX_CODEC_U24L,
    MX_CODEC_S32I       = MX_CODEC_S32L,
    MX_CODEC_U32I       = MX_CODEC_U32L,
#else
    MX_CODEC_S16N       = MX_CODEC_S16L,
    MX_CODEC_U16N       = MX_CODEC_U16L,
    MX_CODEC_S24N       = MX_CODEC_S24L,
    MX_CODEC_U24N       = MX_CODEC_U24L,
    MX_CODEC_S32N       = MX_CODEC_S32L,
    MX_CODEC_U32N       = MX_CODEC_U32L,
    MX_CODEC_FL32       = MX_CODEC_F32L,
    MX_CODEC_FL64       = MX_CODEC_F64L,
    
    MX_CODEC_S16I       = MX_CODEC_S16B,
    MX_CODEC_U16I       = MX_CODEC_U16B,
    MX_CODEC_S24I       = MX_CODEC_S24B,
    MX_CODEC_U24I       = MX_CODEC_U24B,
    MX_CODEC_S32I       = MX_CODEC_S32B,
    MX_CODEC_U32I       = MX_CODEC_U32B,
#endif
    
    /* Non official codecs, used to force a profile in an encoder */
    /* MPEG-1 video */
    MX_CODEC_MP1V      = MX_FOURCC('m','p','1','v'),
    /* MPEG-2 video */
    MX_CODEC_MP2V      = MX_FOURCC('m','p','2','v'),
    /* MPEG-I/II layer 2 audio */
    MX_CODEC_MP2       = MX_FOURCC('m','p','2',' '),
    /* MPEG-I/II layer 3 audio */
    MX_CODEC_MP3       = MX_FOURCC('m','p','3',' '),
};

#endif //__MXFOURCC_H__
