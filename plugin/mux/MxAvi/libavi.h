#include "MxTypes.h"
#include "MxWave.h"

/* biCompression / Others are FourCC */
#define BI_RGB              0x0000
#define BI_RLE8             0x0001
#define BI_RLE4             0x0002
#define BI_BITFIELDS        0x0003
#define BI_JPEG             0x0004
#define BI_PNG              0x0005
#define BI_CMYK             0x000B
#define BI_CMYKRLE8         0x000C
#define BI_CMYKRLE4         0x000D

/* flags for use in <dwFlags> in AVIFileHdr */
#define AVIF_HASINDEX       0x00000010  /* Index at end of file? */
#define AVIF_MUSTUSEINDEX   0x00000020
#define AVIF_ISINTERLEAVED  0x00000100
#define AVIF_TRUSTCKTYPE    0x00000800  /* Use CKType to find key frames? */
#define AVIF_WASCAPTUREFILE 0x00010000
#define AVIF_COPYRIGHTED    0x00020000

/* Flags for index */
#define AVIIF_LIST          0x00000001L /* chunk is a 'LIST' */
#define AVIIF_KEYFRAME      0x00000010L /* this frame is a key frame.*/
#define AVIIF_NOTIME        0x00000100L /* this frame doesn't take any time */
#define AVIIF_COMPUSE       0x0FFF0000L /* these bits are for compressor use */

#define AVIIF_FIXKEYFRAME   0x00001000L /* invented; used to say that */
                                        /* the keyframe flag isn't a true flag */
                                        /* but have to be verified */

#define AVI_CHUNK_COMMON           \
    MxFourcc i_chunk_fourcc;   \
    uint64_t i_chunk_size;         \
    uint64_t i_chunk_pos;          \
    union  avi_chunk_u *p_next;    \
    union  avi_chunk_u *p_father;  \
    union  avi_chunk_u *p_first;

#define AVI_CHUNK( p_chk ) (avi_chunk_t*)(p_chk)

typedef struct idx1_entry_s
{
    MxFourcc i_fourcc;
    uint32_t i_flags;
    uint32_t i_pos;
    uint32_t i_length;

} idx1_entry_t;

typedef struct avi_chunk_common_s
{
    AVI_CHUNK_COMMON
} avi_chunk_common_t;

typedef struct avi_chunk_list_s
{
    AVI_CHUNK_COMMON
    MxFourcc i_type;
} avi_chunk_list_t;

typedef struct avi_chunk_idx1_s
{
    AVI_CHUNK_COMMON
    unsigned int i_entry_count;
    unsigned int i_entry_max;
    idx1_entry_t *entry;

} avi_chunk_idx1_t;

typedef struct avi_chunk_avih_s
{
    AVI_CHUNK_COMMON
    uint32_t i_microsecperframe;
    uint32_t i_maxbytespersec;
    uint32_t i_reserved1; /* dwPaddingGranularity;    pad to multiples of this
                             size; normally 2K */
    uint32_t i_flags;
    uint32_t i_totalframes;
    uint32_t i_initialframes;
    uint32_t i_streams;
    uint32_t i_suggestedbuffersize;
    uint32_t i_width;
    uint32_t i_height;
    uint32_t i_scale;
    uint32_t i_rate;
    uint32_t i_start;
    uint32_t i_length;
} avi_chunk_avih_t;

typedef struct avi_chunk_strh_s
{
    AVI_CHUNK_COMMON
    MxFourcc i_type;
    uint32_t i_handler;
    uint32_t i_flags;
    uint32_t i_reserved1;    /* wPriority wLanguage */
    uint32_t i_initialframes;
    uint32_t i_scale;
    uint32_t i_rate;
    uint32_t i_start;
    uint32_t i_length;       /* In units above... */
    uint32_t i_suggestedbuffersize;
    uint32_t i_quality;
    uint32_t i_samplesize;
} avi_chunk_strh_t;

typedef struct avi_chunk_strf_auds_s
{
    AVI_CHUNK_COMMON
    int             i_cat;
    WAVEFORMATEX    *p_wf;
} avi_chunk_strf_auds_t;

typedef struct avi_chunk_strf_vids_s
{
    AVI_CHUNK_COMMON
    int                     i_cat;
    MX_BITMAPINFOHEADER    *p_bih;
} avi_chunk_strf_vids_t;

typedef union avi_chunk_strf_u
{
    avi_chunk_strf_auds_t   auds;
    avi_chunk_strf_vids_t   vids;
    struct
    {
        AVI_CHUNK_COMMON
        int i_cat;
    }                       common;
} avi_chunk_strf_t;

typedef struct avi_chunk_strd_s
{
    AVI_CHUNK_COMMON
    uint8_t  *p_data;
} avi_chunk_strd_t;

typedef struct avi_chunk_vprp_s
{
    AVI_CHUNK_COMMON
    uint32_t i_video_format_token;
    uint32_t i_video_standard;
    uint32_t i_vertical_refresh;
    uint32_t i_h_total_in_t;
    uint32_t i_v_total_in_lines;
    uint32_t i_frame_aspect_ratio;
    uint32_t i_frame_width_in_pixels;
    uint32_t i_frame_height_in_pixels;
    uint32_t i_nb_fields_per_frame;
    struct
    {
        uint32_t i_compressed_bm_height;
        uint32_t i_compressed_bm_width;
        uint32_t i_valid_bm_height;
        uint32_t i_valid_bm_width;
        uint32_t i_valid_bm_x_offset;
        uint32_t i_valid_bm_y_offset;
        uint32_t i_video_x_offset_in_t;
        uint32_t i_video_y_valid_start_line;
    } field_info[2];

} avi_chunk_vprp_t;

typedef struct avi_chunk_dmlh_s
{
    AVI_CHUNK_COMMON
    uint32_t dwTotalFrames;
} avi_chunk_dmlh_t;

#define AVI_ZEROSIZED_CHUNK     0xFF
#define AVI_ZERO_FOURCC         0xFE

#define AVI_INDEX_OF_INDEXES    0x00
#define AVI_INDEX_OF_CHUNKS     0x01
#define AVI_INDEX_IS_DATA       0x80

#define AVI_INDEX_2FIELD        0x01
typedef struct
{
    uint32_t i_offset;
    uint32_t i_size;
} indx_std_entry_t;

typedef struct
{
    uint32_t i_offset;
    uint32_t i_size;
    uint32_t i_offsetfield2;
} indx_field_entry_t;

typedef struct
{
    uint64_t i_offset;
    uint32_t i_size;
    uint32_t i_duration;
} indx_super_entry_t;

typedef struct avi_chunk_indx_s
{
    AVI_CHUNK_COMMON
    int16_t  i_longsperentry;
    int8_t   i_indexsubtype;
    int8_t   i_indextype;
    uint32_t i_entriesinuse;
    MxFourcc i_id;

    uint64_t i_baseoffset;

    union
    {
        indx_std_entry_t    *std;
        indx_field_entry_t  *field;
        indx_super_entry_t  *super;
    } idx;
} avi_chunk_indx_t;

typedef struct avi_chunk_STRING_s
{
    AVI_CHUNK_COMMON
    char *p_type;
    char *p_str;
} avi_chunk_STRING_t;

typedef union avi_chunk_u
{
    avi_chunk_common_t  common;
    avi_chunk_list_t    list;
    avi_chunk_idx1_t    idx1;
    avi_chunk_avih_t    avih;
    avi_chunk_strh_t    strh;
    avi_chunk_strf_t    strf;
    avi_chunk_strd_t    strd;
    avi_chunk_vprp_t    vprp;
    avi_chunk_indx_t    indx;
    avi_chunk_STRING_t  strz;
} avi_chunk_t;

/****************************************************************************
 * Stream(input) access functions
 ****************************************************************************/
int     AVI_ChunkRead( stream_t *,
                       avi_chunk_t *p_chk,
                       avi_chunk_t *p_father );
void    AVI_ChunkClean( stream_t *, avi_chunk_t * );

int     AVI_ChunkCount_( avi_chunk_t *, MxFourcc, bool );
void   *AVI_ChunkFind_ ( avi_chunk_t *, MxFourcc, int, bool );

int     AVI_ChunkReadRoot( stream_t *, avi_chunk_t *p_root );
void    AVI_ChunkFreeRoot( stream_t *, avi_chunk_t *p_chk  );
int     AVI_ChunkFetchIndexes( stream_t *, avi_chunk_t *p_riff );

#define AVI_ChunkCount( p_chk, i_fourcc, b_list ) \
    AVI_ChunkCount_( AVI_CHUNK(p_chk), i_fourcc, b_list )
#define AVI_ChunkFind( p_chk, i_fourcc, i_number, b_list ) \
    AVI_ChunkFind_( AVI_CHUNK(p_chk), i_fourcc, i_number, b_list )

/* *** avi stuff *** */

#define AVIFOURCC_RIFF         MX_FOURCC('R','I','F','F')
#define AVIFOURCC_ON2          MX_FOURCC('O','N','2',' ')
#define AVIFOURCC_LIST         MX_FOURCC('L','I','S','T')
#define AVIFOURCC_JUNK         MX_FOURCC('J','U','N','K')
#define AVIFOURCC_AVI          MX_FOURCC('A','V','I',' ')
#define AVIFOURCC_AVIX         MX_FOURCC('A','V','I','X')
#define AVIFOURCC_ON2f         MX_FOURCC('O','N','2','f')
#define AVIFOURCC_WAVE         MX_FOURCC('W','A','V','E')
#define AVIFOURCC_INFO         MX_FOURCC('I','N','F','O')

#define AVIFOURCC_avih         MX_FOURCC('a','v','i','h')
#define AVIFOURCC_ON2h         MX_FOURCC('O','N','2','h')
#define AVIFOURCC_hdrl         MX_FOURCC('h','d','r','l')
#define AVIFOURCC_movi         MX_FOURCC('m','o','v','i')
#define AVIFOURCC_idx1         MX_FOURCC('i','d','x','1')

#define AVIFOURCC_strl         MX_FOURCC('s','t','r','l')
#define AVIFOURCC_strh         MX_FOURCC('s','t','r','h')
#define AVIFOURCC_strf         MX_FOURCC('s','t','r','f')
#define AVIFOURCC_strd         MX_FOURCC('s','t','r','d')
#define AVIFOURCC_strn         MX_FOURCC('s','t','r','n')
#define AVIFOURCC_indx         MX_FOURCC('i','n','d','x')
#define AVIFOURCC_vprp         MX_FOURCC('v','p','r','p')
#define AVIFOURCC_dmlh         MX_FOURCC('d','m','l','h')

#define AVIFOURCC_rec          MX_FOURCC('r','e','c',' ')
#define AVIFOURCC_auds         MX_FOURCC('a','u','d','s')
#define AVIFOURCC_vids         MX_FOURCC('v','i','d','s')
#define AVIFOURCC_txts         MX_FOURCC('t','x','t','s')
#define AVIFOURCC_mids         MX_FOURCC('m','i','d','s')
#define AVIFOURCC_iavs         MX_FOURCC('i','a','v','s')
#define AVIFOURCC_ivas         MX_FOURCC('i','v','a','s')

#define AVIFOURCC_IARL         MX_FOURCC('I','A','R','L')
#define AVIFOURCC_IART         MX_FOURCC('I','A','R','T')
#define AVIFOURCC_ICMS         MX_FOURCC('I','C','M','S')
#define AVIFOURCC_ICMT         MX_FOURCC('I','C','M','T')
#define AVIFOURCC_ICOP         MX_FOURCC('I','C','O','P')
#define AVIFOURCC_ICRD         MX_FOURCC('I','C','R','D')
#define AVIFOURCC_ICRP         MX_FOURCC('I','C','R','P')
#define AVIFOURCC_IDIM         MX_FOURCC('I','D','I','M')
#define AVIFOURCC_IDPI         MX_FOURCC('I','D','P','I')
#define AVIFOURCC_IENG         MX_FOURCC('I','E','N','G')
#define AVIFOURCC_IGNR         MX_FOURCC('I','G','N','R')
#define AVIFOURCC_ISGN         MX_FOURCC('I','S','G','N')
#define AVIFOURCC_IKEY         MX_FOURCC('I','K','E','Y')
#define AVIFOURCC_ILGT         MX_FOURCC('I','L','G','T')
#define AVIFOURCC_IMED         MX_FOURCC('I','M','E','D')
#define AVIFOURCC_INAM         MX_FOURCC('I','N','A','M')
#define AVIFOURCC_IPLT         MX_FOURCC('I','P','L','T')
#define AVIFOURCC_IPRD         MX_FOURCC('I','P','R','D')
#define AVIFOURCC_ISBJ         MX_FOURCC('I','S','B','J')
#define AVIFOURCC_ISFT         MX_FOURCC('I','S','F','T')
#define AVIFOURCC_ISHP         MX_FOURCC('I','S','H','P')
#define AVIFOURCC_ISRC         MX_FOURCC('I','S','R','C')
#define AVIFOURCC_ISRF         MX_FOURCC('I','S','R','F')
#define AVIFOURCC_ITCH         MX_FOURCC('I','T','C','H')
#define AVIFOURCC_ISMP         MX_FOURCC('I','S','M','P')
#define AVIFOURCC_IDIT         MX_FOURCC('I','D','I','T')
#define AVIFOURCC_ILNG         MX_FOURCC('I','L','N','G')
#define AVIFOURCC_IRTD         MX_FOURCC('I','R','T','D')
#define AVIFOURCC_IWEB         MX_FOURCC('I','W','E','B')
#define AVIFOURCC_IPRT         MX_FOURCC('I','P','R','T')
#define AVIFOURCC_IWRI         MX_FOURCC('I','W','R','I')
#define AVIFOURCC_IPRO         MX_FOURCC('I','P','R','O')
#define AVIFOURCC_ICNM         MX_FOURCC('I','C','N','M')
#define AVIFOURCC_IPDS         MX_FOURCC('I','P','D','S')
#define AVIFOURCC_IEDT         MX_FOURCC('I','E','D','T')
#define AVIFOURCC_ICDS         MX_FOURCC('I','C','D','S')
#define AVIFOURCC_IMUS         MX_FOURCC('I','M','U','S')
#define AVIFOURCC_ISTD         MX_FOURCC('I','S','T','D')
#define AVIFOURCC_IDST         MX_FOURCC('I','D','S','T')
#define AVIFOURCC_ICNT         MX_FOURCC('I','C','N','T')
#define AVIFOURCC_ISTR         MX_FOURCC('I','S','T','R')
#define AVIFOURCC_IFRM         MX_FOURCC('I','F','R','M')

#define AVIFOURCC_IAS1         MX_FOURCC('I','A','S','1')
#define AVIFOURCC_IAS2         MX_FOURCC('I','A','S','2')
#define AVIFOURCC_IAS3         MX_FOURCC('I','A','S','3')
#define AVIFOURCC_IAS4         MX_FOURCC('I','A','S','4')
#define AVIFOURCC_IAS5         MX_FOURCC('I','A','S','5')
#define AVIFOURCC_IAS6         MX_FOURCC('I','A','S','6')
#define AVIFOURCC_IAS7         MX_FOURCC('I','A','S','7')
#define AVIFOURCC_IAS8         MX_FOURCC('I','A','S','8')
#define AVIFOURCC_IAS9         MX_FOURCC('I','A','S','9')

#define AVITWOCC_wb            MX_TWOCC('w','b')
#define AVITWOCC_db            MX_TWOCC('d','b')
#define AVITWOCC_dc            MX_TWOCC('d','c')
#define AVITWOCC_pc            MX_TWOCC('p','c')
#define AVITWOCC_AC            MX_TWOCC('A','C')
#define AVITWOCC_tx            MX_TWOCC('t','x')
#define AVITWOCC_sb            MX_TWOCC('s','b')

/* *** codex stuff ***  */

/* DV */
#define FOURCC_dvsd         MX_FOURCC('d','v','s','d')
#define FOURCC_dvhd         MX_FOURCC('d','v','h','d')
#define FOURCC_dvsl         MX_FOURCC('d','v','s','l')
#define FOURCC_dv25         MX_FOURCC('d','v','2','5')
#define FOURCC_dv50         MX_FOURCC('d','v','5','0')
