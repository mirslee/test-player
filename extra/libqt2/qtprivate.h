/*******************************************************************************
 qtprivate.h

 libquicktime - A library for reading and writing quicktime/avi/mp4 files.
 http://libquicktime.sourceforge.net

 Copyright (C) 2002 Heroine Virtual Ltd.
 Copyright (C) 2002-2007 Members of the libquicktime project.

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 2.1 of the License, or (at your option)
 any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along
 with this library; if not, write to the Free Software Foundation, Inc., 51
 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*******************************************************************************/

#ifndef PRIVATE_H
#define PRIVATE_H

#include "charset.h"
#include "lqt_atoms.h"
#include <inttypes.h>
#include <stdio.h> // For quicktime_s->stream

/* ================================= structures */


/* Version used internally.  You need to query it with the C functions */
/* These must match quicktime4linux !!! */
#define QUICKTIME_MAJOR   2
#define QUICKTIME_MINOR   0
#define QUICKTIME_RELEASE 0

#define HEADER_LENGTH 8
#define MAXTRACKS 1024
#define MAXNODES 1

/* Crazy Mich R. Soft constants */
#define AVI_HASINDEX       0x00000010  // Index at end of file?
#define AVI_MUSTUSEINDEX   0x00000020
#define AVI_ISINTERLEAVED  0x00000100
#define AVI_TRUSTCKTYPE    0x00000800  // Use CKType to find key frames?
#define AVI_WASCAPTUREFILE 0x00010000
#define AVI_COPYRIGHTED    0x00020000
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVI_KEYFRAME       0x10
#define AVI_INDEX_OF_CHUNKS 0x01
#define AVI_INDEX_OF_INDEXES 0x00
                                                                                                                     
#define AVI_FRAME_RATE_BASE 10000
#define MAX_RIFFS  0x100


#define QTVR_GRABBER_SCROLLER_UI 1
#define QTVR_OLD_JOYSTICK_UI 2 
#define QTVR_JOYSTICK_UI 3
#define QTVR_GRABBER_UI 4
#define QTVR_ABSOLUTE_UI 5

/* Forward declarations */
typedef struct
{
/* for AVI it's the end of the 8 byte header in the file */
/* for Quicktime it's the start of the 8 byte header in the file */
	int64_t start;
	int64_t end;        /* byte endpoint in file */
	int64_t size;       /* byte size for writing */
	int use_64;         /* Use 64 bit header */
	unsigned char type[4];
} quicktime_atom_t;


typedef struct
{
	int64_t start;
	int64_t end;        /* byte endpoint in file */
	int64_t size;       /* byte size for writing */
	unsigned char type[4];
	int child_count;
	int use_64;
	int ID;
} quicktime_qtatom_t;

typedef struct
{
	float values[9];
} quicktime_matrix_t;

typedef struct
  {
  uint32_t major_brand;
  uint32_t minor_version;
  int num_compatible_brands;
  uint32_t * compatible_brands;
  } quicktime_ftyp_t;

typedef struct
{
	int version;
	int flags;
        uint64_t creation_time;
	uint64_t modification_time;
	int track_id;
	int reserved1;
	uint64_t duration;
        uint8_t reserved2[8];
	int layer;
	int alternate_group;
	float volume;
	int reserved3;
	quicktime_matrix_t matrix;
	float track_width;
	float track_height;
} quicktime_tkhd_t;


typedef struct
{
	int seed;
	int flags;
	int size;
	int16_t *alpha;
	int16_t *red;
	int16_t *green;
	int16_t *blue;
} quicktime_ctab_t;



/* ===================== sample table ======================== // */



/* sample description */

typedef struct
{
	int motion_jpeg_quantization_table;
} quicktime_mjqt_t;


typedef struct
{
	int motion_jpeg_huffman_table;
} quicktime_mjht_t;


typedef struct
{
	uint16_t version;
	uint16_t revision;
	uint32_t imagingMode;
	uint32_t imagingValidFlags;
	uint32_t correction;
	uint32_t quality;
	uint32_t directdraw;
	uint32_t imagingProperties[6];
	uint32_t reserved1;
	uint32_t reserved2;
} quicktime_impn_t;

typedef struct
{
	quicktime_impn_t impn;
} quicktime_imgp_t;

typedef struct
{
	int reserved1;
	int reserved2;
	int version;
	int revision;
	
	int STrack; /* Prefix 'S' == Scene */
	int LowResSTrack;
	uint32_t reserved3[6];
	int HSTrack; /* Prefix 'HS' == HotSpot */
	uint32_t reserved4[9];
	
	float HPanStart;
	float HPanEnd;
	float VPanStart;
	float VPanEnd;
	float MinZoom;
	float MaxZoom;
	
	int SHeight;
	int SWidth;
	int NumFrames;
	int reserved5;
	int SNumFramesHeight;
	int SNumFramesWidth;
	int SDepth;
	
	int HSHeight;
	int HSWidth;
	int reserved6;
	int HSNumFramesHeight;
	int HSNumFramesWidth;
	int HSDepth;
} quicktime_pano_t;

typedef struct
{
	int version;
	int revision;
	char nodeType[4];
	int locationFlags;
	int locationData;
	int reserved1;
	int reserved2;
} quicktime_nloc_t;

typedef struct
{
	int version;
	int revision;
	char nodeType[4];
	int nodeID;
	int nameAtomID;
	int commentAtomID;
	int reserved1;
	int reserved2;
} quicktime_ndhd_t;

typedef struct
{
	quicktime_nloc_t nloc;
	int ID;
} quicktime_vrni_t;

typedef struct
{
	quicktime_vrni_t vrni[MAXNODES];
	int children;
} quicktime_vrnp_t;

typedef struct
{
	int version;
	int revision;
	int NameAtomID;
	int DefaultNodeID;
	int flags;
	int reserved1;
	int reserved2;
	
} quicktime_vrsc_t;

typedef struct
{
	quicktime_vrsc_t vrsc;
	quicktime_imgp_t imgp;
	quicktime_vrnp_t vrnp;
} quicktime_qtvr_t;


/* MPEG-4 esds (elementary stream descriptor) */

typedef struct
  {
  int version;
  int flags;

  uint16_t esid;
  uint8_t stream_priority;
  
  uint8_t  objectTypeId;
  uint8_t  streamType;
  uint32_t bufferSizeDB;
  uint32_t maxBitrate;
  uint32_t avgBitrate;

  int      decoderConfigLen;
  uint8_t* decoderConfig;

  } quicktime_esds_t;

/* MPEG-4 iods */

typedef struct
  {
  int version;
  int flags;

  uint16_t ObjectDescriptorID;
  uint8_t  ODProfileLevel;
  uint8_t  sceneProfileLevel;
  uint8_t  audioProfileId;
  uint8_t  videoProfileId;
  uint8_t  graphicsProfileLevel;

  struct track
    {
    uint8_t ES_ID_IncTag;
    uint8_t length;
    uint32_t track_id;
    } * tracks;
  int num_tracks;
  } quicktime_iods_t;

/* User atoms: These can be either inside a wave atom (for audio) or
   in the sample description (for video) */

typedef struct
  {
  int num_atoms;
  uint8_t ** atoms;
  } quicktime_user_atoms_t;

/* wave atom and subatoms */

typedef struct
  {
  char codec[4];
  /* Remainder could be a WAVEFORMATEX structure */
  } quicktime_frma_t;

typedef struct
  {
  int16_t littleEndian;
  } quicktime_enda_t;

typedef struct
  {
  quicktime_frma_t frma;
  int has_frma;
  quicktime_enda_t enda;
  int has_enda;

  quicktime_esds_t esds;
  int has_esds;
  
  quicktime_user_atoms_t user_atoms;
  } quicktime_wave_t;

typedef struct
  {
  int version;
  int flags;
  
  uint32_t mChannelLayoutTag;
  uint32_t mChannelBitmap;
  uint32_t mNumberChannelDescriptions;

  struct chdesc
    {
    uint32_t mChannelLabel;
    uint32_t mChannelFlags;
    float    mCoordinates[3];
    } * ChannelDescriptions;
  } quicktime_chan_t;

typedef struct
  {
  int fields;    /* 0, 1, or 2 */
  int dominance;   /* 0 - unknown     1 - top first     2 - bottom first */
  } quicktime_fiel_t;

typedef struct
  {
  float gamma;
  } quicktime_gama_t;

/* Font table for MPEG-4 timed text */

typedef struct
  {
  uint16_t num_fonts;

  struct qtfont
    {
    uint16_t font_id;
    char font_name[256];
    } * fonts;
  } quicktime_ftab_t;

/* Sample description for Quicktime text tracks */

typedef struct
  {
  uint32_t displayFlags;
  uint32_t textJustification;
  uint16_t bgColor[3];
  uint16_t defaultTextBox[4];
  uint32_t scrpStartChar;              /*starting character position*/
  uint16_t scrpHeight;
  uint16_t scrpAscent;
  uint16_t scrpFont;
  uint16_t scrpFace;
  uint16_t scrpSize;
  uint16_t scrpColor[3];
  char font_name[256];
  } quicktime_stsd_text_t;

/* Sample description for MPEG-4 text tracks */

typedef struct
  {
  uint32_t display_flags;
  uint8_t horizontal_justification;
  uint8_t vertical_justification;
  uint8_t back_color[4];
  uint16_t defaultTextBox[4];
  uint16_t start_char_offset;
  uint16_t end_char_offset;
  uint16_t font_id;
  uint8_t  style_flags;
  uint8_t  font_size;
  uint8_t  text_color[4];
  int has_ftab;
  quicktime_ftab_t ftab;
  } quicktime_stsd_tx3g_t;

/* Sample description for timecode tracks */

typedef struct
  {
  uint32_t reserved2;
  uint32_t flags;
  uint32_t timescale;
  uint32_t frameduration;
  uint8_t numframes;

// http://developer.apple.com/documentation/QuickTime/QTFF/QTFFChap3/chapter_4_section_4.html#//apple_ref/doc/uid/TP40000939-CH205-BBCGABGG
  //  uint8_t reserved3[3];

  // Real life
  uint8_t reserved3;
  
  char * name;
  } quicktime_stsd_tmcd_t;

typedef struct
{
	int glbllen;
	uint8_t* glbldata;
}quicktime_stsd_glbl_t;

typedef struct
{
	char format[4];
	uint8_t reserved[6];
	int data_reference;

/* common to audio and video */
	int version;
	int revision;
	char vendor[4];

/* video description */
	int temporal_quality;
	int spatial_quality;
	int width;
	int height;
	float dpi_horizontal;
	float dpi_vertical;
	int64_t data_size;
	int frames_per_sample;
	char compressor_name[32];
	int depth;
	int ctab_id;
    int has_ctab;
	quicktime_ctab_t ctab;

    quicktime_pasp_t pasp;
    int has_pasp;
	quicktime_colr_t colr;
	int has_colr;
	quicktime_clap_t clap;
    int has_clap;

	quicktime_fiel_t fiel;
	int has_fiel;

	quicktime_gama_t gama;
	int has_gama;

	quicktime_pano_t pano;
	quicktime_qtvr_t qtvr;

	int has_wave;
	quicktime_wave_t wave;
/* audio description */
	uint32_t channels;
	uint32_t sample_size;
/* Audio extension for version == 2 */
    uint32_t formatSpecificFlags;
    uint32_t constBytesPerAudioPacket;
    uint32_t constLPCMFramesPerAudioPacket;
/* LQT: We have int16_t for the compression_id, because otherwise negative
   values don't show up correctly */
    int16_t compression_id;
	int packet_size;
    double samplerate;

/* Audio extension for version == 1 */

    uint32_t audio_samples_per_packet;
    uint32_t audio_bytes_per_packet;
    uint32_t audio_bytes_per_frame;
    uint32_t audio_bytes_per_sample;


    quicktime_esds_t esds;
    int has_esds;

    quicktime_chan_t chan;
    int has_chan;

	quicktime_stsd_glbl_t glbl;
	int has_glbl;

    quicktime_user_atoms_t user_atoms;

/* LQT: We store the complete atom (starting with the fourcc)
   here, because this must be passed to the Sorenson 3 decoder */

    unsigned char * table_raw;
    int table_raw_size;

/* Quicktime text */
    quicktime_stsd_text_t text;

/* Quicktime tx3g */
    quicktime_stsd_tx3g_t tx3g;

    quicktime_stsd_tmcd_t tmcd;

} quicktime_stsd_table_t;


typedef struct
{
	int version;
	int flags;
	int total_entries;
	quicktime_stsd_table_t *table;
} quicktime_stsd_t;


/* time to sample */
typedef struct
{
	int sample_count;
	int sample_duration;
} quicktime_stts_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
        int entries_allocated;
        int  default_duration;
	quicktime_stts_table_t *table;
} quicktime_stts_t;

/* Composition time to sample */

typedef struct
{
	int sample_count;
	int sample_duration;
} quicktime_ctts_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
    int entries_allocated;
	quicktime_ctts_table_t *table;
} quicktime_ctts_t;

/* sync sample */
typedef struct
{
	int sample;
} quicktime_stss_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
	int entries_allocated;
	quicktime_stss_table_t *table;
} quicktime_stss_t;


/* Partial sync sample */
typedef struct
{
	int sample;
} quicktime_stps_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
	int entries_allocated;
	quicktime_stps_table_t *table;
} quicktime_stps_t;

/* sample to chunk */
typedef struct
{
	int chunk;
	int samples;
	int id;
} quicktime_stsc_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
	
	int entries_allocated;
	quicktime_stsc_table_t *table;
} quicktime_stsc_t;


/* sample size */
typedef struct
{
	int64_t size;
} quicktime_stsz_table_t;

typedef struct
{
	int version;
	int flags;
	int sample_size;
	int total_entries;
	int entries_allocated;    /* used by the library for allocating a table */
	quicktime_stsz_table_t *table;
} quicktime_stsz_t;


/* chunk offset */
typedef struct
{
	int64_t offset;
} quicktime_stco_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
	
	int entries_allocated;    /* used by the library for allocating a table */
	quicktime_stco_table_t *table;
        int co64;
} quicktime_stco_t;


typedef struct
{
	uint8_t sampledependency;
} quicktime_sdtp_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
	int entries_allocated;    /* used by the library for allocating a table */
	quicktime_sdtp_table_t *table;
} quicktime_sdtp_t;

/* sample table */
typedef struct
{
	int version;
	int flags;
	quicktime_stsd_t stsd;
	quicktime_stts_t stts;
	quicktime_stss_t stss;
	quicktime_stps_t stps;
	quicktime_stsc_t stsc;
	quicktime_stsz_t stsz;
	quicktime_stco_t stco;
	quicktime_ctts_t ctts;
    int has_ctts;
	quicktime_sdtp_t sdtp;
	int has_sdtp;
  } quicktime_stbl_t;

typedef struct
  {
  char type[4];
  int num_tracks;
  uint32_t * tracks;
  } quicktime_track_reference_t;

typedef struct
  {
  int num_references;
  quicktime_track_reference_t * references;
  } quicktime_tref_t;

/* data reference */

typedef struct
{
	int64_t size;
	char type[4];
	int version;
	int flags;
	char *data_reference;
} quicktime_dref_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;
	quicktime_dref_table_t *table;
} quicktime_dref_t;

/* data information */

typedef struct
{
	quicktime_dref_t dref;
} quicktime_dinf_t;

/* video media header */

typedef struct
{
	int version;
	int flags;
	int graphics_mode;
	int opcolor[3];
} quicktime_vmhd_t;


/* sound media header */

typedef struct
{
	int version;
	int flags;
	int balance;
	int reserved;
} quicktime_smhd_t;


/* Base media info */

typedef struct
{
	int version;
	int flags;
	int graphics_mode;
	int opcolor[3];
	int balance;
	int reserved;
} quicktime_gmin_t;

/* Obscure text atom found inside the gmhd atom
 * of text tracks
 * TODO: Reverse engineer this
 */

typedef struct
  {
  uint32_t unk[9];
  } quicktime_gmhd_text_t;

typedef struct
  {
  int version;
  int flags;
  int font;
  int face;
  int size;
  int txtcolor[3];
  int bgcolor[3];
  char fontname[256];
  } quicktime_tcmi_t;

typedef struct
  {
  quicktime_tcmi_t tcmi;
  } quicktime_tmcd_t;

/* Base (generic) media header */

typedef struct
  {
  quicktime_gmin_t gmin;

  int has_gmhd_text;
  quicktime_gmhd_text_t gmhd_text;
  
  int has_tmcd;
  quicktime_tmcd_t tmcd;
  
  } quicktime_gmhd_t;

typedef struct
{
	int version;
	int flags;
} quicktime_nmhd_t;

/* handler reference */

typedef struct
{
	int version;
	int flags;
	char component_type[4];
	char component_subtype[4];
        char component_manufacturer[4];
	int component_flags;
	int component_flag_mask;
	char component_name[256];
} quicktime_hdlr_t;

/* media information */

typedef struct
{
	int is_video;
	int is_audio;
        int is_audio_vbr;   /* Special flag indicating VBR audio */
	int is_panorama;
	int is_qtvr;
	int is_object;
        int is_text;
        int is_timecode;
	quicktime_vmhd_t vmhd;
	quicktime_smhd_t smhd;
	quicktime_gmhd_t gmhd;
	int has_gmhd;

        quicktime_nmhd_t nmhd;
	int has_nmhd;

        quicktime_stbl_t stbl;
	quicktime_hdlr_t hdlr;
        int has_hdlr;

        quicktime_dinf_t dinf;
} quicktime_minf_t;


/* media header */

typedef struct
{
	int version;
	int flags;
	uint64_t creation_time;
	uint64_t modification_time;
	int time_scale;
	uint64_t duration;
	int language;
	int quality;
} quicktime_mdhd_t;


/* media */

typedef struct
{
	quicktime_mdhd_t mdhd;
	quicktime_minf_t minf;
	quicktime_hdlr_t hdlr;
} quicktime_mdia_t;

/* edit list */
typedef struct
{
	__int64 duration;
	__int64 time;
	float rate;
} quicktime_elst_table_t;

typedef struct
{
	int version;
	int flags;
	int total_entries;

	quicktime_elst_table_t *table;
} quicktime_elst_t;

typedef struct
{
	quicktime_elst_t elst;
} quicktime_edts_t;

/* qtvr navg (v1.0) */
typedef struct {
    int    version;        // Always 1
    int    columns;    // Number of columns in movie
    int    rows;        // Number rows in movie
    int    reserved;        // Zero
    int    loop_frames;        // Number of frames shot at each position
    int    loop_dur;        // The duration of each frame
    int    movietype;        // kStandardObject, kObjectInScene, or
                    // kOldNavigableMovieScene
    int    loop_timescale;        // Number of ticks before next frame of
                    // loop is displayed
    float    fieldofview;        // 180.0 for kStandardObject or
                    // kObjectInScene, actual  degrees for
                    // kOldNavigableMovieScene.
    float    startHPan;        // Start horizontal pan angle in
                    //  degrees
    float    endHPan;        // End horizontal pan angle in  degrees
    float    endVPan;        // End vertical pan angle in  degrees
    float    startVPan;        // Start vertical pan angle in  degrees
    float    initialHPan;        // Initial horizontal pan angle in
                    //  degrees (poster view)
    float    initialVPan;        // Initial vertical pan angle in  degrees
                    // (poster view)
    int    reserved2;        // Zero
} quicktime_navg_t;


typedef struct
  {
  quicktime_tkhd_t tkhd;
  quicktime_mdia_t mdia;
  quicktime_edts_t edts;
  int has_edts;
  
  quicktime_tref_t tref;
  int chunk_sizes_alloc;
  int64_t * chunk_sizes; /* This contains the chunk sizes for audio
                            tracks. They can not so easily be obtained
                            during decoding */
  int has_tref;
  } quicktime_trak_t;


typedef struct
{
	int version;
	int flags;
	uint64_t creation_time;
	uint64_t modification_time;
	int time_scale;
	uint64_t duration;
	float preferred_rate;
	float preferred_volume;
	uint8_t reserved[10];
	quicktime_matrix_t matrix;
	int preview_time;
	int preview_duration;
	int poster_time;
	int selection_time;
	int selection_duration;
	int current_time;
	int next_track_id;
} quicktime_mvhd_t;


typedef struct
{
	char *copyright;
	int copyright_len;
	char *name;
	int name_len;
	char *info;
	int info_len;
/* Additional Metadata for libquicktime */
        char *album;
        int album_len;
        char *author;
        int author_len;
        char *artist;
        int artist_len;
        char *genre;
        int genre_len;
        char *track;
        int track_len;
        char *comment;
        int comment_len;
	int is_qtvr;
	/* player controls */
	char ctyp[4];
	quicktime_navg_t navg;
	quicktime_hdlr_t hdlr;
        int has_hdlr;
} quicktime_udta_t;


typedef struct
{
	int total_tracks;

	quicktime_mvhd_t mvhd;
	quicktime_trak_t *trak[MAXTRACKS];
	quicktime_udta_t udta;
        int has_ctab;
	quicktime_ctab_t ctab;
        int has_iods;
        quicktime_iods_t iods;

} quicktime_moov_t;

typedef struct
{
	quicktime_atom_t atom;
} quicktime_mdat_t;

/* table of pointers to every track */
typedef struct
{
	quicktime_trak_t *track; /* real quicktime track corresponding to this table */
	int channels;
    int samplerate;

    /* number of audio channels in the track */
    lqt_channel_t * channel_setup;
    int64_t current_position;   /* current sample in output file */
	int64_t current_chunk;      /* current chunk in output file */

        int eof; /* This is set to 1 by the core if one tries to read beyond EOF */

/* VBR stuff */
        int vbr_num_frames; /* Frames written since start of chunk */
        int64_t vbr_frame_start;
        int64_t vbr_frames_written;
} quicktime_audio_map_t;

typedef struct
{
	quicktime_trak_t *track;
	quicktime_trak_t *timecode_track;
	int current_position;   /* current frame in output file */
	int current_chunk;      /* current chunk in output file */

    lqt_interlace_mode_t interlace_mode;

// Timecode stuff
    uint32_t encode_timecode;
    int has_encode_timecode;
	int current_timecode_chunk;      /* current chunk in output file */

/* Timestamp of the last encoded timecode */
    int64_t timecode_timestamp;

/* For decoding, *all* timecodes are read here.
 * For encoding, this contains a small buffer such that we don't
   start a new chunk for each timecode. */
        int timecodes_alloc;
        int num_timecodes;
        uint32_t * timecodes;
        int timecodes_written;
  } quicktime_video_map_t;

/* Text track */

typedef struct
  {
  quicktime_trak_t *track;
  int is_chapter_track; /* For encoding only */
  int64_t current_position;
  
  lqt_charset_converter_t * cnv;

  char * text_buffer;
  int text_buffer_alloc;

  int initialized; /* For encoding only */
  int64_t current_chunk;
  
  } quicktime_text_map_t;

/* obji */

typedef struct
{
    int version;
    int revision;
    int movieType;
    int viewStateCount;
    int defaultViewState;
    int mouseDownViewState;
    int viewDuration;
    int columns;
    int rows;
    float mouseMotionScale;
    float minPan;
    float maxPan;
    float defaultPan;
    float minTilt;
    float maxTilt;
    float defaultTilt;
    float minFOV;
    float FOV;
    float defaultFOV;
    float defaultViewCenterH;
    float defaultViewCenterV;
    float viewRate;
    float frameRate;
    int animSettings;
    int controlSettings;
} quicktime_obji_t;

/* pdat */

typedef struct
{
    int version;
    int revision;
    int imageRefTrackIndex;
    int hotSpotRefTrackIndex;
    float minPan;
    float maxPan;
    float defaultPan;
    float minTilt;
    float maxTilt;
    float defaultTilt;
    float minFOV;
    float maxFOV;
    float defaultFOV;
    int imageSizeX;
    int imageSizeY;
    int imageNumFramesX;
    int imageNumFramesY;
    int hotSpotSizeX;
    int hotSpotSizeY;
    int hotSpotNumFramesX;
    int hotSpotNumFramesY;
    int flags;
    char panoType[4];
    int reserved;
} quicktime_pdat_t;

/* pHdr */

typedef struct
{
    unsigned int    nodeID;
    float        defHPan;
    float        defVPan;
    float        defZoom;

    // constraints for this node; use zero for default
    float        minHPan;
    float        minVPan;
    float        minZoom;
    float        maxHPan;
    float        maxVPan;
    float        maxZoom;

    int        reserved1;        // must be zero
    int        reserved2;        // must be zero
    int        nameStrOffset;        // offset into string table atom
    int        commentStrOffset;    // offset into string table atom
} quicktime_pHdr_t;

/* qtvr node */

typedef struct
{
    	int node_type;
	int64_t node_start; /* start frame */ 
	int64_t node_size; /* size of node in frames */
	quicktime_ndhd_t ndhd;
	quicktime_obji_t obji;
	quicktime_pdat_t pdat;
} quicktime_qtvr_node_t;

/* file descriptor passed to all routines */
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

#define qt2fseek(file,b) file->fileio.libqt_fseek(file->fileio.file,b)
#define qt2read(file,a,b,c) file->fileio.libqt_read(file->fileio.file,a,(char*)b,c)
#define qt2write(file,a,b,c) file->fileio.libqt_write(file->fileio.file,a,(char*)b,c)
#define qt2flen(file) file->fileio.libqt_flen(file->fileio.file)

struct quicktime_s
{
	/* I/O handling callbacks on stream */
	libqt_file fileio;

	int64_t total_length;
        int encoding_started;
	quicktime_mdat_t mdat;
	quicktime_moov_t moov;
        quicktime_ftyp_t ftyp;
        int has_ftyp;

        lqt_file_type_t file_type;

	int rd;
	int wr;

/* If the moov atom is compressed */
        int compressed_moov;
        unsigned char *moov_data;
/*
 * Temporary storage of compressed sizes.  If the file length is shorter than the
 * uncompressed sizes, it won't work.
 */
        int64_t moov_end;
        int64_t moov_size;

/* for begining and ending frame writes where the user wants to write the  */
/* file descriptor directly */
	int64_t offset;
/* I/O */
/* Current position of virtual file descriptor */
	int64_t file_position;      
// Work around a bug in glibc where ftello returns only 32 bits by maintaining
// our own position
	int64_t ftell_position;

/* Read ahead buffer */
	int64_t preload_size;      /* Enables preload when nonzero. */
	uint8_t *preload_buffer;
	int64_t preload_start;     /* Start of preload_buffer in file */
	int64_t preload_end;       /* End of preload buffer in file */
	int64_t preload_ptr;       /* Offset of preload_start in preload_buffer */


/* mapping of audio channels to movie tracks */
/* one audio map entry exists for each channel */
	int total_atracks;
	quicktime_audio_map_t *atracks;

/* mapping of video tracks to movie tracks */
	int total_vtracks;
	quicktime_video_map_t *vtracks;

/* Mapping of text tracks to movie tracks */
	int total_ttracks;
	quicktime_text_map_t *ttracks;

/* Parameters for frame currently being decoded */
	int in_x, in_y, in_w, in_h, out_w, out_h;
/*      Libquicktime change: color_model and row_span are now saved per track */
/*	int color_model, row_span; */

	quicktime_qtvr_node_t qtvr_node[MAXNODES];

/* Logging support */
        lqt_log_callback_t log_callback;
        void * log_data;

/* I/O Error is saved here: It has the advantage, that
   codecs don't have to check the return values of quicktime_[read|write]_data
   all the time. */
        int io_error;
        int io_eof;
};


#endif
