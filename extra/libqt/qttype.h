#pragma once


#define OQT_ATOM_HEAD_LENGTH	8
#define OQT_MAX_TRACKS			1024

typedef struct
{
	__int64 start;      /* byte start in file */
	__int64 end;        /* byte endpoint in file */
	__int64 size;       /* byte size for writing */
	long use_64;     /* Use 64 bit header */
	char  type[4];
} oqt_atom_t;

typedef struct
{
	float values[9];
} oqt_matrix_t;


typedef struct
{
	long  version;
	long  flags;
	ULONG creation_time;
	ULONG modification_time;
	long  track_id;
	long  reserved1;
	long  duration;
	char   reserved2[8];
	long  layer;
	long  alternate_group;
	float    volume;
	long  reserved3;
	oqt_matrix_t matrix;
	float track_width;
	float track_height;
} oqt_tkhd_t;


typedef struct
{
	long seed;
	long flags;
	long size;

	short *alpha;
	short *red;
	short *green;
	short *blue;
} oqt_ctab_t;



/* ===================== sample table ======================== */



/* sample description */

typedef struct
{
	long motion_jpeg_quantization_table;
} oqt_mjqt_t;


typedef struct
{
	long motion_jpeg_huffman_table;
} oqt_mjht_t;


#define		OQT_STSDWAVE_PCM		(0x00)
//#define		OQT_STSDWAVE_MPEG1	(0x50)
#define		OQT_STSDWAVE_MPEG	(0x55)

typedef struct
{
	char	fmtType[4];
	int	 	little_endian;

	short	wFormatTag;			// 0x55 for MP3
	short	nChannels;			// Number of channels in the wave: 1 for mono, 2 for stereo.
	long	nSamplesPerSec;		// Sampling frequency (in Hz) of the wave file: 32000, 44100, or 48000 etc.
	long	nAvgBytesPerSec;	// Average data rate
	short	nBlockAlign;		// For streams in which the frame length varies, set to 1.
	// Layer 1: nBlockAlign = 4*(int)(12*BitRate/SamplingFreq)
	// Layers 2 and 3: nBlockAlign = (int)(144*BitRate/SamplingFreq)
	short	wBitsPerSample;		// Not used; set to zero. (or 16?)
	short	cbSize;				// The size in bytes of the extended data after the WAVEFORMATEX structure.
	// For MPEG1WAVEFORMAT this is 22 (0x0016).

	/* The MPEG Audio Extension
	- thanks to Adrian Bourke for help with this
	*/
	short	wID;
	long	fdwFlags;
	short	nBlockSize;
	short	nFramesPerBlock;
	short	nCodecDelay;

	/* Other formats are shoved in buffer */
	long	extra_len;
	char	*extra;

} oqt_stsdwave_t;


/* MP4 Elementary Stream Descriptor */
typedef struct
{
	int			version;
	long	flags;
	int			decoderConfigLen;
	int			objectTypeId;
	int			streamType;
	int			bufferSizeDB;
	long	maxBitrate;
	long	avgBitrate;
	BYTE*	decoderConfig;
} oqt_esds_t;

typedef struct 
{
	long reserved1[6];
	long frames;
	long reserved2[10];
	long width;
	long height;
	long reserved3[2];
	float framerate;
	long reserved4;
}oqt_genl_t;

typedef struct 
{
	long keypos;
	long location;
	long offset;
	long display;
}oqt_midx_t;
typedef struct 
{
	long reserved[3];
	oqt_midx_t* idxs;
}oqt_ttfo_t;

typedef struct 
{

}oqt_time_t;

typedef struct 
{
	long reserved[3];
	oqt_genl_t genl;
	oqt_ttfo_t ttfo;
	oqt_time_t time;
}oqt_m12v_t;

typedef struct
{
	char format[4];
	char reserved[6];
	long data_reference;

	/* common to audio and video */
	long version;
	long revision;
	char vendor[4];

	/* video description */
	long temporal_quality;
	long spatial_quality;
	long width;
	long height;
	float dpi_horizontal;
	float dpi_vertical;
	__int64 data_size;
	long frames_per_sample;
	char compressor_name[32];
	long depth;
	long ctab_id;
	oqt_ctab_t ctab;
	float gamma;
	long fields;    /* 0, 1, or 2 */
	long field_dominance;   /* 0 - unknown     1 - top first     2 - bottom first */
	oqt_mjqt_t mjqt;
	oqt_mjht_t mjht;

	long strf_len;
	BYTE *strf;	/* 'strf' chunk from AVI format */


	/* audio description */
	long channels;
	long sample_size;
	long compression_id;
	long packet_size;
	float sample_rate;

	/* audio description V1 */
	ULONG samplesPerPacket;
	ULONG bytesPerPacket;
	ULONG bytesPerFrame;
	ULONG bytesPerSample;

	/* Wave Extension */
	oqt_stsdwave_t wave;

	/* MP4 ESDS */
	oqt_esds_t esds;

	oqt_m12v_t m12v;
	/* identical to first table entry with only data_reference changed */
	int same_but_external;

} oqt_stsd_table_t;


typedef struct
{
	long version;
	long flags;
	long total_entries;
	oqt_stsd_table_t *table;
} oqt_stsd_t;


/* time to sample */
typedef struct
{
	long sample_count;
	long sample_duration;
} oqt_stts_table_t;

typedef struct
{
	long version;
	long flags;
	long total_entries;
	oqt_stts_table_t *table;
} oqt_stts_t;


/* sync sample */
typedef struct
{
	long sample;
} oqt_stss_table_t;

typedef struct
{
	long version;
	long flags;
	long total_entries;
	long entries_allocated;
	oqt_stss_table_t *table;
} oqt_stss_t;


/* sample to chunk */
typedef struct
{
	long chunk;
	long samples;
	long id;
} oqt_stsc_table_t;

typedef struct
{
	long version;
	long flags;
	long total_entries;

	long entries_allocated;
	oqt_stsc_table_t *table;
} oqt_stsc_t;


/* sample size */
typedef struct
{
	__int64 size;
} oqt_stsz_table_t;

typedef struct
{
	long version;
	long flags;
	__int64 sample_size;
	long total_entries;

	long entries_allocated;    /* used by the library for allocating a table */
	oqt_stsz_table_t *table;
} oqt_stsz_t;


/* chunk offset */
typedef struct
{
	__int64 offset;
} oqt_stco_table_t;

typedef struct
{
	long version;
	long flags;
	long total_entries;

	long entries_allocated;    /* used by the library for allocating a table */
	oqt_stco_table_t *table;
} oqt_stco_t;

/* sample table */
typedef struct
{
	long version;
	long flags;
	oqt_stsd_t stsd;
	oqt_stts_t stts;
	oqt_stss_t stss;
	oqt_stsc_t stsc;
	oqt_stsz_t stsz;
	oqt_stco_t stco;
} oqt_stbl_t;


/* data reference */
typedef struct
{
	__int64 size;
	char type[4];
	long version;
	long flags;
	char *data_reference;
} oqt_dref_table_t;

typedef struct
{
	long version;
	long flags;
	long total_entries;
	oqt_dref_table_t *table;
} oqt_dref_t;

/* data information */
typedef struct
{
	oqt_dref_t dref;
} oqt_dinf_t;


/* video media header */
typedef struct
{
	long version;
	long flags;
	long graphics_mode;
	long opcolor[3];
} oqt_vmhd_t;


/* sound media header */
typedef struct
{
	long version;
	long flags;
	long balance;
	long reserved;
} oqt_smhd_t;

/*generic media info*/
typedef struct
{
	long version;
	long flags;
/* QuickDraw graphic mode
	- mode types are copy = 0x0000 ; dither copy = 0x0040 ; straight alpha = 0x0100
	- mode types are composition dither copy = 0x0103 ; blend = 0x0020
	- mode premultipled types are white alpha = 0x101 ; black alpha = 0x102
	- mode color types are transparent = 0x0024 ; straight alpha blend = 0x0104*/
	long drawmode;//QuickDraw graphic mode
	long color[3];//graphic mode color
	long balance;
	long reserved;
} oqt_gmin_t;
/* generic media info header */
typedef struct
{
	oqt_gmin_t gmin;
} oqt_gmhd_t;

/* handler reference */
typedef struct
{
	long version;
	long flags;
	char component_type[4];
	char component_subtype[4];
	char component_manufacturer[4];
	//long component_manufacturer;
	long component_flags;
	long component_flag_mask;
	char component_name[256];
} oqt_hdlr_t;


/* media information */
typedef struct
{
	long is_video;
	long is_audio;
	long is_strm;
	oqt_vmhd_t vmhd;
	oqt_smhd_t smhd;
	oqt_gmhd_t gmhd;
	oqt_stbl_t stbl;
	oqt_hdlr_t hdlr;
	oqt_dinf_t dinf;
} oqt_minf_t;

/* media header */
typedef struct
{
	long version;
	long flags;
	ULONG creation_time;
	ULONG modification_time;
	long time_scale;
	long duration;
	long language;
	long quality;
} oqt_mdhd_t;

/* media */
typedef struct
{
	oqt_mdhd_t mdhd;
	oqt_minf_t minf;
	oqt_hdlr_t hdlr;
} oqt_mdia_t;

/* edit list */
typedef struct
{
	long duration;
	long time;
	float rate;
} oqt_elst_table_t;

typedef struct
{
	long version;
	long flags;
	long total_entries;

	oqt_elst_table_t *table;
} oqt_elst_t;

typedef struct
{
	oqt_elst_t elst;
} oqt_edts_t;


typedef struct
{
	oqt_tkhd_t tkhd;
	oqt_mdia_t mdia;
	oqt_edts_t edts;
} oqt_trak_t;



typedef struct
{
	long version;
	long flags;
	ULONG creation_time;
	ULONG modification_time;
	long time_scale;
	long duration;
	float preferred_rate;
	float preferred_volume;
	char reserved[10];
	oqt_matrix_t matrix;
	long preview_time;
	long preview_duration;
	long poster_time;
	long selection_time;
	long selection_duration;
	long current_time;
	long next_track_id;
} oqt_mvhd_t;

typedef struct
{
	long version;
	ULONG flags;
	long audioProfileId;
	long videoProfileId;
} oqt_iods_t;




#define OQT_INFO_COPYRIGHT 		"©cpy"
#define OQT_INFO_NAME 			"©nam"
#define OQT_INFO_INFORMATION 	"©inf"
#define OQT_INFO_ARTIST 		"©ART"
#define OQT_INFO_ALBUMN 		"©alb"
#define OQT_INFO_ENCODED_BY 	"©enc"
#define OQT_INFO_TRACK 			"©trk"
#define OQT_INFO_CREATION_DATE 	"©day"
#define OQT_INFO_COMMENT 		"©cmt"
#define OQT_INFO_AUTHOR 		"©aut"
#define OQT_INFO_COMPOSER 		"©com"
#define OQT_INFO_DESCRIPTION 	"©des"
#define OQT_INFO_DIRECTOR 		"©dir"
#define OQT_INFO_DISCLAIMER 	"©dis"
#define OQT_INFO_GENRE 			"©gen"
#define OQT_INFO_HOST_COMPUTER 	"©hst"
#define OQT_INFO_MAKE 			"©mak"
#define OQT_INFO_MODEL 			"©mod"
#define OQT_INFO_ORIG_ARTIST 	"©ope"
#define OQT_INFO_ORIG_FORMAT 	"©fmt"
#define OQT_INFO_ORIG_SOURCE 	"©src"
#define OQT_INFO_PERFORMERS 	"©prf"
#define OQT_INFO_PRODUCER 		"©prd"
#define OQT_INFO_PRODUCT 		"©PRD"
#define OQT_INFO_SOFTWARE 		"©swr"
#define OQT_INFO_SPECL_PLAY_REQ "©req"
#define OQT_INFO_WARNING 		"©wrn"
#define OQT_INFO_WRITER 		"©wrt"
#define OQT_INFO_URL_LINK 		"©url"
#define OQT_INFO_EDIT_DATE_1 	"©ed1"
#define OQT_INFO_EDIT_DATE_2 	"©ed2"
#define OQT_INFO_EDIT_DATE_3 	"©ed3"
#define OQT_INFO_EDIT_DATE_4 	"©ed4"
#define OQT_INFO_EDIT_DATE_5 	"©ed5"
#define OQT_INFO_EDIT_DATE_6 	"©ed6"
#define OQT_INFO_EDIT_DATE_7 	"©ed7"
#define OQT_INFO_EDIT_DATE_8 	"©ed8"
#define OQT_INFO_EDIT_DATE_9 	"©ed9"

typedef struct
{
	char	code[4];
	char*	value;
	int		value_len;
} oqt_udta_t;


typedef struct
{
	int total_tracks;
	int udta_count;

	oqt_atom_t	atom;
	oqt_mvhd_t	mvhd;
	oqt_trak_t	*trak[OQT_MAX_TRACKS];
	oqt_udta_t	*udta;
	oqt_ctab_t	ctab;
	oqt_iods_t	iods;

	short	window_x;
	short	window_y;
} oqt_moov_t;


typedef struct
{
	oqt_atom_t atom;
} oqt_mdat_t;



struct oqt_audio_codec_struct;
struct oqt_video_codec_struct;


/* table of pointers to every track */
typedef struct oqt_audio_map_struct
{
	oqt_trak_t *track; /* real quicktime track corresponding to this table */
	__int64 current_sample;   /* current sample in output file */
	long current_chunk;      /* current chunk in output file */

	// Buffers for the chunk being processed
	char* decoded_buf_ptr;	/* Buffer containing raw audio samples */
	int decoded_buf_size;	/* Total length of the work buffer */
	int decoded_buf_used;		/* Length of data currently in work buffer */

	char* encoded_buf_ptr;	/* Buffer containing encoded audio (to/from file) */
	int encoded_buf_size;		/* Length of the current read buffer */
	int encoded_buf_used;		/* Length of data currently in read buffer */

	int encoder_preped;	/* Set to true when encoder is ready to go */
	int samples_per_chunk;	/* Number of samples for each chunk when encoding */

	void *codec_private; /* Pointer to structure used internally by codecs */
	struct oqt_audio_codec_struct *codec;
} oqt_audio_map_t;



typedef struct oqt_video_map_struct
{
	oqt_trak_t *track;
	__int64 current_frame;	/* current frame in output file */
	long current_chunk;	/* current chunk in output file */

	void *codec_private; /* Pointer to structure used internally by codecs */
	struct oqt_video_codec_struct *codec;
} oqt_video_map_t;

typedef struct  
{
	int		(*libqt_read) (void* file, __int64 offset,char *data, __int64 size);
	int		(*libqt_fseek)(void* file, __int64 offset);
	__int64 (*libqt_ftell)(void* file);
	__int64 (*libqt_flen) (void* file);
	int		(*libqt_write)(void* file, __int64 offset,char *data, __int64 size);
	int		(*libqt_close)(void* file);
	void*	file;
}libqt_file;


#define OQT_MODE_READ		0x01
#define OQT_MODE_WRITE		0x02
#define OQT_MODE_READWRITE	0x03

/* file descriptor passed to all routines */
typedef struct oqt_struct
{
	/* Is file Read/Write ? */
	int stream_mode;
	/* Current position in file - only use within io.c
	Please access using oqt_get_position(), oqt_set_position() */
	__int64 file_position;

	/* Parameters to handle compressed atoms */
	__int64 decompressed_buffer_size;
	char *decompressed_buffer;
	__int64 decompressed_position;


	/* Read ahead buffer */
	__int64 preload_size;      /* Enables preload when nonzero. */
	char *preload_buffer;
	__int64 preload_start;     /* Start of preload_buffer in file */
	__int64 preload_end;       /* End of preload buffer in file */
	__int64 preload_ptr;       /* Offset of preload_start in preload_buffer */


	/* I/O handling callbacks on stream */
	libqt_file fileio;

	/* Roots of the atom tree */
	oqt_mdat_t mdat;
	oqt_moov_t moov;


	/* mapping of audio tracks to movie tracks */
	int total_atracks;
	oqt_audio_map_t *atracks;

	/* mapping of video tracks to movie tracks */
	int total_vtracks;
	oqt_video_map_t *vtracks;

	/* MP4 handling */
	int use_mp4;

	/* external resources */
	int ext_count;
	struct oqt_struct **ext_files;

	/* File to be used in next data access.
	Set by oqt_set_{vide,audi}o_position. */
	struct oqt_struct *data_file;

} oqt_t;



// Any codec type less than 1 is invalid and won't be added
// to the registry....
#define OQT_CODEC_TYPE_NONE		0
#define OQT_CODEC_TYPE_VIDEO	1
#define OQT_CODEC_TYPE_AUDIO	2
// Only loads other codecs - does nothing useful itself.
#define OQT_CODEC_TYPE_LOADER	3



#define OQT_NORMAL_MERIT 100

typedef struct oqt_codec_info_struct {
	char fourcc[4];
	int type;
	/* OQT_CODEC_TYPE_NONE, (oqt_codec_info_struct)
	OQT_CODEC_TYPE_VIDEO, (oqt_video_codec_struct)
	OQT_CODEC_TYPE_AUDIO, (oqt_audio_codec_struct) 
	OQT_CODEC_TYPE_LOADER, (oqt_codec_info_struct) */

	const char *name;
	const char *description;	// Not required
	const char *url;			// URL for *Module* not codec
	// and only set for code outside main source tree
	const char *version;
	const char *copyright;
	const char *license;
	const char *module_author;
	const char *codec_author;

	int interface_age;
	char has_children;
	void *plugin_handle;
	struct oqt_codec_info_struct *parent;
	int merit;

} oqt_codec_info_t;






typedef struct
{
	char* name;
	char  type;			// See above
	char* desc;
	/* Still to be done
	Function for validating set_value method ?
	void* min_val;
	void* max_val;
	void* default_val;
	*/
} oqt_codec_param_t;




typedef struct oqt_video_codec_struct
{
	oqt_codec_info_t info;

	int(*init_codec)(oqt_t *file, int track);
	int(*delete_codec)(oqt_t *file, int track);
	const oqt_codec_param_t* (*list_params)(int *o_numparams);

	int(*decode)(oqt_t *file,
		int track,
		ULONG inputsize,
		BYTE *input,
		BYTE **output,
		int color_model);	/* Colour model of output data */

	int(*encode)(oqt_t *file,
		int track,
		BYTE **input,
		ULONG out_size,  /* Size of output buffer (in bytes) */
		BYTE *output,
		int *IsAKeyFrame,	 /* Set to 1 if encoded frame is a keyframe */
		int color_model);	 /* Colour model of input data */


	/* Colour model routines */
	int (*reads_colormodel)(oqt_t *file,
		int track,  		      
		int color_model);

	int (*writes_colormodel)(oqt_t *file, 
		int track,  		      
		int color_model);

	/* The colour model to be used for conversion.
	- if the colour model wanted is not supported by codec */
	int (*preferred_colormodel)(oqt_t *file,
		int track);


	int (*set_param)(oqt_t *file, int track,
		const char* param, const void* data);
	int (*get_param)(oqt_t *file, int track,
		const char* param, void* data);

} oqt_video_codec_t;




typedef struct oqt_audio_codec_struct
{
	oqt_codec_info_t info;

	int(*init_codec)(oqt_t *file, int track);
	int(*delete_codec)(oqt_t *file, int track);
	const oqt_codec_param_t* (*list_params)(int *o_numparams);


	// Returns the number of bytes put in the output buffer //
	int(*decode)(oqt_t *file,
		int track,
		ULONG in_size, /* in bytes */
		BYTE  *input,
		ULONG out_size, /* in bytes */
		BYTE  *output);

	/* Call just before doing any encoding 
	- returns the number of samples to put in each chunk
	- returns 0 to use default chunk size
	- returns -1 if audio format is unsupported/error
	*/
	int(*prepare_encoder)(oqt_t *file, int track);

	/* Converts a number of frames of audio to its length in bytes.
	Audio codecs are only required to give this if it is possible to
	calculate - more complex codecs can rely on the STSZ table. */
	int(*frames_to_bytes)(oqt_t *file, int track, int frames);

	int(*encode)(oqt_t *file,
		int track,
		ULONG num_samples,  /* Number of samples in the L (or R) input buffer */
		char *input,
		ULONG out_size,  /* Size of output buffer (in bytes) */
		char *output);

	int (*set_param)(oqt_t *file, int track,
		const char* param, const void* data);
	int (*get_param)(oqt_t *file, int track,
		const char* param, void* data);

} oqt_audio_codec_t;
