/* funcprotos.h
 * 
 * OpenQuicktime private function prototypes
 *
 * Copyright (C) 2002 QT4Linux and OpenQuicktime Teams
 *
 * This file is part of OpenQuicktime, a free QuickTime library.
 * Based on QT4Linux by Adam Williams.
 *
 * OpenQuicktime is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * OpenQuicktime is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: funcprotos.h,v 1.22 2003/04/20 00:32:58 nhumfrey Exp $
 */
#pragma once

#include "qttype.h"


/* In atom.c */
int oqt_atom_reset(oqt_atom_t *atom);
int oqt_atom_read_type(char *data, char *type);
int oqt_atom_read_header(oqt_t *file, oqt_atom_t *atom);
int oqt_atom_write_header64(oqt_t *file, oqt_atom_t *atom, char *text);
int oqt_atom_write_header(oqt_t *file, oqt_atom_t *atom, char *text);
void oqt_atom_write_footer(oqt_t *file, oqt_atom_t *atom);
int oqt_atom_is(oqt_atom_t *atom, char *type);
int oqt_atom_is_null(oqt_atom_t *atom);
ULONG oqt_atom_read_size(char *data);
__int64 oqt_atom_read_size64(char *data);
int oqt_atom_skip(oqt_t *file, oqt_atom_t *atom);

/* In codecs.c */
int oqt_init_video_map(oqt_t *file, int track, oqt_trak_t *trak);
int oqt_init_audio_map(oqt_t *file, int track, oqt_trak_t *trak);
int oqt_delete_video_map(oqt_t *file, int track);
int oqt_delete_audio_map(oqt_t *file, int track);

/* In colormodels.c */
void oqt_cmodel_transfer(
	BYTE **output_rows, /* Leave NULL if non existent */
	BYTE **input_rows,
	BYTE *out_y_plane, /* Leave NULL if non existent */
	BYTE *out_u_plane,
	BYTE *out_v_plane,
	BYTE *in_y_plane, /* Leave NULL if non existent */
	BYTE *in_u_plane,
	BYTE *in_v_plane,
	int in_x,        /* Dimensions to capture from input frame */
	int in_y, 
	int in_w, 
	int in_h,
	int out_x,       /* Dimensions to project on output frame */
	int out_y, 
	int out_w, 
	int out_h,
	int in_colormodel, 
	int out_colormodel,
	int bg_color,         /* When transfering BC_RGBA8888 to non-alpha this is the background color in 0xRRGGBB hex */
	int in_rowspan,       /* For planar use the luma rowspan */
	int out_rowspan);     /* For planar use the luma rowspan */
	

/* In ctab.c */
int oqt_ctab_init(oqt_ctab_t *ctab);
int oqt_ctab_delete(oqt_ctab_t *ctab);
void oqt_ctab_dump(oqt_ctab_t *ctab);
int oqt_read_ctab(oqt_t *file, oqt_ctab_t *ctab);
int oqt_ctab_default(oqt_ctab_t *ctab, int ctab_id);
int oqt_write_ctab(oqt_t *file, oqt_ctab_t *ctab);

/* In dinf.c */
void oqt_dinf_init(oqt_dinf_t *dinf);
void oqt_dinf_delete(oqt_dinf_t *dinf);
void oqt_dinf_init_all(oqt_dinf_t *dinf);
void oqt_dinf_dump(oqt_dinf_t *dinf);
void oqt_read_dinf(oqt_t *file, oqt_dinf_t *dinf, oqt_atom_t *dinf_atom);
void oqt_write_dinf(oqt_t *file, oqt_dinf_t *dinf);

/* In dref.c */
void oqt_dref_table_init(oqt_dref_table_t *table);
void oqt_dref_table_delete(oqt_dref_table_t *table);
void oqt_read_dref_table(oqt_t *file, oqt_dref_table_t *table);
void oqt_write_dref_table(oqt_t *file, oqt_dref_table_t *table);
void oqt_dref_table_dump(oqt_dref_table_t *table);
void oqt_dref_init(oqt_dref_t *dref);
void oqt_dref_init_all(oqt_dref_t *dref);
void oqt_dref_delete(oqt_dref_t *dref);
void oqt_dref_dump(oqt_dref_t *dref);
void oqt_read_dref(oqt_t *file, oqt_dref_t *dref);
void oqt_write_dref(oqt_t *file, oqt_dref_t *dref);
int oqt_dref_find_url_entry(oqt_dref_t *dref, char *url);
int oqt_dref_add_url_entry(oqt_dref_t *dref, char *url);

/* In edts.c */
void oqt_edts_init(oqt_edts_t *edts);
void oqt_edts_delete(oqt_edts_t *edts);
void oqt_edts_init_table(oqt_edts_t *edts);
void oqt_read_edts(oqt_t *file, oqt_edts_t *edts, oqt_atom_t *edts_atom);
void oqt_edts_dump(oqt_edts_t *edts);
void oqt_write_edts(oqt_t *file, oqt_edts_t *edts, long duration);

/* In elst.c */
void oqt_elst_table_init(oqt_elst_table_t *table);
void oqt_elst_table_delete(oqt_elst_table_t *table);
void oqt_read_elst_table(oqt_t *file, oqt_elst_table_t *table);
void oqt_write_elst_table(oqt_t *file, oqt_elst_table_t *table, long duration);
void oqt_elst_table_dump(oqt_elst_table_t *table);
void oqt_elst_init(oqt_elst_t *elst);
void oqt_elst_init_all(oqt_elst_t *elst);
void oqt_elst_delete(oqt_elst_t *elst);
void oqt_elst_dump(oqt_elst_t *elst);
void oqt_read_elst(oqt_t *file, oqt_elst_t *elst);
void oqt_write_elst(oqt_t *file, oqt_elst_t *elst, long duration);

/* In esds.c */
int oqt_esds_init(oqt_esds_t *esds);
int oqt_esds_get_decoder_config(oqt_esds_t *esds, BYTE **ppBuf, int *pBufSize);
int oqt_esds_set_decoder_config(oqt_esds_t *esds, BYTE *pBuf, int bufSize);
int oqt_esds_delete(oqt_esds_t *esds);
int oqt_esds_dump(oqt_esds_t *esds);
int oqt_read_esds(oqt_t *file, oqt_esds_t *esds);
int oqt_write_esds_common(oqt_t *file, oqt_esds_t *esds, int esid, ULONG objectType, ULONG streamType);
int oqt_write_esds_audio(oqt_t *file, oqt_esds_t *esds, int esid);
int oqt_write_esds_video(oqt_t *file, oqt_esds_t *esds, int esid);

int oqt_m12v_init(oqt_m12v_t *m12v);
int oqt_m12v_delete(oqt_m12v_t *m12v);
int oqt_read_m12v(oqt_t *file, oqt_m12v_t *m12v,oqt_atom_t *parent_atom);

/* In hdlr.c */
void oqt_hdlr_init(oqt_hdlr_t *hdlr);
void oqt_hdlr_init_video(oqt_hdlr_t *hdlr);
void oqt_hdlr_init_audio(oqt_hdlr_t *hdlr);
void oqt_hdlr_init_data(oqt_hdlr_t *hdlr);
void oqt_hdlr_delete(oqt_hdlr_t *hdlr);
void oqt_hdlr_dump(oqt_hdlr_t *hdlr);
void oqt_read_hdlr(oqt_t *file, oqt_hdlr_t *hdlr);
void oqt_write_hdlr(oqt_t *file, oqt_hdlr_t *hdlr);

/* In io_stub.c */
int oqt_close_stub(oqt_t *file);
int oqt_read_stub(oqt_t *file, char *data, __int64 size);
int oqt_write_stub(oqt_t *file, char *data, __int64 size);
int oqt_fseek_stub(oqt_t *file, __int64 offset);
__int64 oqt_ftell_stub(oqt_t *file);
__int64 oqt_flen_stub(oqt_t *file);

/* In io.c */
int oqt_read_preload(oqt_t *file, char *data, __int64 size);
void oqt_read_pascal(oqt_t *file, char *data);
void oqt_write_pascal(oqt_t *file, char *data);
float oqt_read_fixed32(oqt_t *file);
int oqt_write_fixed32(oqt_t *file, float number);
int oqt_write_int64(oqt_t *file, __int64 value);
int oqt_write_int32(oqt_t *file, long value);
int oqt_write_le_int32(oqt_t *file, long value);
int oqt_write_uint32(oqt_t *file, ULONG value);
int oqt_write_char32(oqt_t *file, char *string);
float oqt_read_fixed16(oqt_t *file);
int oqt_write_fixed16(oqt_t *file, float number);
ULONG oqt_read_uint32(oqt_t *file);
long oqt_read_int32(oqt_t *file);
long oqt_read_le_int32(oqt_t *file);
__int64 oqt_read_int64(oqt_t *file);
long oqt_read_int24(oqt_t *file);
int oqt_write_int24(oqt_t *file, long number);
short oqt_read_int16(oqt_t *file);
short oqt_read_le_int16(oqt_t *file);
int oqt_write_int16(oqt_t *file, short number);
int oqt_write_le_int16(oqt_t *file, short number);
int oqt_read_char(oqt_t *file);
int oqt_write_char(oqt_t *file, char x);
void oqt_read_char32(oqt_t *file, char *string);
oqt_t * oqt_file_from_sd_id(oqt_t *file, oqt_trak_t *trak, int sd_id);

/* In matrix.c */
void oqt_matrix_init(oqt_matrix_t *matrix);
void oqt_matrix_delete(oqt_matrix_t *matrix);
void oqt_read_matrix(oqt_t *file, oqt_matrix_t *matrix);
void oqt_matrix_dump(oqt_matrix_t *matrix);
void oqt_write_matrix(oqt_t *file, oqt_matrix_t *matrix);

/* In mdat.c */
void oqt_mdat_init(oqt_mdat_t *mdat);
void oqt_mdat_delete(oqt_mdat_t *mdat);
void oqt_read_mdat(oqt_t *file, oqt_mdat_t *mdat, oqt_atom_t *parent_atom);
void oqt_write_mdat(oqt_t *file, oqt_mdat_t *mdat);

/* In mdhd.c */
void oqt_mdhd_init(oqt_mdhd_t *mdhd);
void oqt_mdhd_init_video(oqt_t *file, oqt_mdhd_t *mdhd, int frame_w, int frame_h, float frame_rate);
void oqt_mdhd_init_audio(oqt_t *file, oqt_mdhd_t *mdhd, int channels, int sample_rate, int bits, char *compressor);
void oqt_mdhd_delete(oqt_mdhd_t *mdhd);
void oqt_read_mdhd(oqt_t *file, oqt_mdhd_t *mdhd);
void oqt_mdhd_dump(oqt_mdhd_t *mdhd);
void oqt_write_mdhd(oqt_t *file, oqt_mdhd_t *mdhd);

/* In mdia.c */
void oqt_mdia_init(oqt_mdia_t *mdia);
void oqt_mdia_init_video(oqt_t *file, oqt_mdia_t *mdia, int frame_w, int frame_h, float frame_rate, char *compressor);
void oqt_mdia_init_audio(oqt_t *file, oqt_mdia_t *mdia, int channels, int sample_rate, int bits, char *compressor);
void oqt_mdia_delete(oqt_mdia_t *mdia);
void oqt_mdia_dump(oqt_mdia_t *mdia);
int oqt_read_mdia(oqt_t *file, oqt_mdia_t *mdia, oqt_atom_t *trak_atom);
void oqt_write_mdia(oqt_t *file, oqt_mdia_t *mdia);

/* In minf.c */
void oqt_minf_init(oqt_minf_t *minf);
void oqt_minf_init_video(oqt_t *file, oqt_minf_t *minf, int frame_w, int frame_h, int time_scale, float frame_rate, char *compressor);
void oqt_minf_init_audio(oqt_t *file, oqt_minf_t *minf, int channels, int sample_rate, int bits, char *compressor);
void oqt_minf_delete(oqt_minf_t *minf);
void oqt_minf_dump(oqt_minf_t *minf);
int oqt_read_minf(oqt_t *file, oqt_minf_t *minf, oqt_atom_t *parent_atom);
void oqt_write_minf(oqt_t *file, oqt_minf_t *minf);

/* In moov.c */
int oqt_moov_init(oqt_moov_t *moov);
int oqt_moov_delete(oqt_moov_t *moov);
void oqt_moov_dump(oqt_moov_t *moov);
int oqt_read_moov(oqt_t *file, oqt_moov_t *moov, oqt_atom_t *parent_atom);
void oqt_write_moov(oqt_t *file, oqt_moov_t *moov);
int oqt_shift_offsets(oqt_moov_t *moov, __int64 offset);

/* In mvhd.c */
int oqt_mvhd_init(oqt_mvhd_t *mvhd);
int oqt_mvhd_delete(oqt_mvhd_t *mvhd);
void oqt_mvhd_dump(oqt_mvhd_t *mvhd);
void oqt_read_mvhd(oqt_t *file, oqt_mvhd_t *mvhd);
void oqt_mhvd_init_video(oqt_t *file, oqt_mvhd_t *mvhd, float frame_rate);
void oqt_write_mvhd(oqt_t *file, oqt_mvhd_t *mvhd);

/* In openquicktime.c  */

/* In plugin.c */
void oqt_load_all_plugins(void);
int oqt_close_plugin(void* handle);

/* In smhd.c */
void oqt_smhd_init(oqt_smhd_t *smhd);
void oqt_smhd_delete(oqt_smhd_t *smhd);
void oqt_smhd_dump(oqt_smhd_t *smhd);
void oqt_read_smhd(oqt_t *file, oqt_smhd_t *smhd);
void oqt_write_smhd(oqt_t *file, oqt_smhd_t *smhd);


/* In gmhd.c */
void oqt_gmhd_init(oqt_gmhd_t *gmhd);
void oqt_gmhd_delete(oqt_gmhd_t *gmhd);
void oqt_gmhd_dump(oqt_gmhd_t *gmhd);
void oqt_read_gmhd(oqt_t *file, oqt_gmhd_t *gmhd);
void oqt_write_gmhd(oqt_t *file, oqt_gmhd_t *gmhd);

/* In stbl.c */
void oqt_stbl_init(oqt_stbl_t *stbl);
void oqt_stbl_init_video(oqt_t *file, oqt_stbl_t *stbl, int frame_w, int frame_h, int time_scale, float frame_rate, char *compressor);
void oqt_stbl_init_audio(oqt_t *file, oqt_stbl_t *stbl, int channels, int sample_rate, int bits, char *compressor);
void oqt_stbl_delete(oqt_stbl_t *stbl);
void oqt_stbl_dump(void *minf_ptr, oqt_stbl_t *stbl);
int oqt_read_stbl(oqt_t *file, oqt_minf_t *minf, oqt_stbl_t *stbl, oqt_atom_t *parent_atom);
void oqt_write_stbl(oqt_t *file, oqt_minf_t *minf, oqt_stbl_t *stbl);

/* In stco.c */
void oqt_stco_init(oqt_stco_t *stco);
void oqt_stco_delete(oqt_stco_t *stco);
void oqt_stco_init_common(oqt_t *file, oqt_stco_t *stco);
void oqt_stco_dump(oqt_stco_t *stco);
void oqt_read_stco(oqt_t *file, oqt_stco_t *stco);
void oqt_read_stco64(oqt_t *file, oqt_stco_t *stco);
void oqt_write_stco(oqt_t *file, oqt_stco_t *stco);
void oqt_update_stco(oqt_stco_t *stco, long chunk, __int64 offset);

/* In stsc.c */
void oqt_stsc_init(oqt_stsc_t *stsc);
void oqt_stsc_init_table(oqt_t *file, oqt_stsc_t *stsc);
void oqt_stsc_init_video(oqt_t *file, oqt_stsc_t *stsc);
void oqt_stsc_init_audio(oqt_t *file, oqt_stsc_t *stsc, int sample_rate);
void oqt_stsc_delete(oqt_stsc_t *stsc);
void oqt_stsc_dump(oqt_stsc_t *stsc);
void oqt_read_stsc(oqt_t *file, oqt_stsc_t *stsc);
void oqt_write_stsc(oqt_t *file, oqt_stsc_t *stsc);
int oqt_update_stsc(oqt_stsc_t *stsc, long chunk, __int64 samples, int id);

/* In stsd.c */
void oqt_stsd_init(oqt_stsd_t *stsd);
void oqt_stsd_init_table(oqt_stsd_t *stsd);
void oqt_stsd_init_video(oqt_t *file, oqt_stsd_t *stsd, int frame_w, int frame_h, float frame_rate, char *compression);
void oqt_stsd_init_audio(oqt_t *file, oqt_stsd_t *stsd, int channels, int sample_rate, int bits, char *compressor);
void oqt_stsd_delete(oqt_stsd_t *stsd);
void oqt_stsd_dump(void *minf_ptr, oqt_stsd_t *stsd);
void oqt_read_stsd(oqt_t *file, oqt_minf_t *minf, oqt_stsd_t *stsd);
void oqt_write_stsd(oqt_t *file, oqt_minf_t *minf, oqt_stsd_t *stsd);
int oqt_stsd_add_reference_entry(oqt_stsd_t *stsd, int data_reference);
int oqt_stsd_find_reference_entry(oqt_stsd_t *stsd, int data_reference);

/* In stsdtable.c */
void oqt_mjqt_init(oqt_mjqt_t *mjqt);
void oqt_mjqt_delete(oqt_mjqt_t *mjqt);
void oqt_mjqt_dump(oqt_mjqt_t *mjqt);
void oqt_mjht_init(oqt_mjht_t *mjht);
void oqt_mjht_delete(oqt_mjht_t *mjht);
void oqt_mjht_dump(oqt_mjht_t *mjht);

int oqt_read_stsdwave(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom);
void oqt_read_stsd_audio(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom);
void oqt_write_stsdwave(oqt_t *file, oqt_stsdwave_t *wave, oqt_esds_t *esds);
void oqt_write_stsd_audio(oqt_t *file, oqt_stsd_table_t *table);
void oqt_read_stsd_video(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom);
void oqt_write_stsd_video(oqt_t *file, oqt_stsd_table_t *table);
void oqt_read_stsd_strm(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom);
void oqt_write_stsd_strm(oqt_t *file, oqt_stsd_table_t *table);
void oqt_read_stsd_table(oqt_t *file, oqt_minf_t *minf, oqt_stsd_table_t *table);
void oqt_stsdwave_init(oqt_stsdwave_t *wave);
void oqt_stsd_table_init(oqt_stsd_table_t *table);
void oqt_stsdwave_delete(oqt_stsdwave_t *wave);
void oqt_stsd_table_delete(oqt_stsd_table_t *table);
void oqt_stsd_video_dump(oqt_stsd_table_t *table);
void oqt_stsdwave_dump(oqt_stsdwave_t *wave);
void oqt_stsd_audio_dump(oqt_stsd_table_t *table);
void oqt_stsd_table_dump(void *minf_ptr, oqt_stsd_table_t *table);
void oqt_write_stsd_table(oqt_t *file, oqt_minf_t *minf, oqt_stsd_table_t *table);

/* In stss.c */
void oqt_stss_init(oqt_stss_t *stss);
void oqt_stss_delete(oqt_stss_t *stss);
void oqt_stss_dump(oqt_stss_t *stss);
void oqt_read_stss(oqt_t *file, oqt_stss_t *stss);
void oqt_write_stss(oqt_t *file, oqt_stss_t *stss);

/* In stsz.c */
void oqt_stsz_init(oqt_stsz_t *stsz);
void oqt_stsz_init_video(oqt_t *file, oqt_stsz_t *stsz);
void oqt_stsz_init_audio(oqt_t *file, oqt_stsz_t *stsz, int channels, int bits);
void oqt_stsz_delete(oqt_stsz_t *stsz);
void oqt_stsz_dump(oqt_stsz_t *stsz);
void oqt_read_stsz(oqt_t *file, oqt_stsz_t *stsz);
void oqt_write_stsz(oqt_t *file, oqt_stsz_t *stsz,long bAudio);
void oqt_update_stsz(oqt_stsz_t *stsz, __int64 sample, long sample_size);

/* In stts.c */
void oqt_stts_init(oqt_stts_t *stts);
void oqt_stts_init_table(oqt_stts_t *stts);
void oqt_stts_init_video(oqt_t *file, oqt_stts_t *stts, int time_scale, float frame_rate);
void oqt_stts_init_audio(oqt_t *file, oqt_stts_t *stts, int sample_rate);
void oqt_stts_delete(oqt_stts_t *stts);
void oqt_stts_dump(oqt_stts_t *stts);
void oqt_read_stts(oqt_t *file, oqt_stts_t *stts);
void oqt_write_stts(oqt_t *file, oqt_stts_t *stts);

/* In tkhd.c */
int oqt_tkhd_init(oqt_tkhd_t *tkhd);
int oqt_tkhd_delete(oqt_tkhd_t *tkhd);
void oqt_tkhd_dump(oqt_tkhd_t *tkhd);
void oqt_read_tkhd(oqt_t *file, oqt_tkhd_t *tkhd);
void oqt_write_tkhd(oqt_t *file, oqt_tkhd_t *tkhd);
void oqt_tkhd_init_video(oqt_t *file, oqt_tkhd_t *tkhd, int frame_w, int frame_h);

/* In trak.c */
int oqt_trak_init(oqt_trak_t *trak);
int oqt_trak_init_video(oqt_t *file, oqt_trak_t *trak, int frame_w, int frame_h, float frame_rate, char *compressor);
int oqt_trak_init_audio(oqt_t *file, oqt_trak_t *trak, int channels, int sample_rate, int bits, char *compressor);
int oqt_trak_delete(oqt_trak_t *trak);
int oqt_trak_dump(oqt_trak_t *trak);
oqt_trak_t *oqt_add_trak(oqt_moov_t *moov);
int oqt_delete_trak(oqt_moov_t *moov);
int oqt_read_trak(oqt_t *file, oqt_trak_t *trak, oqt_atom_t *trak_atom);
int oqt_write_trak(oqt_t *file, oqt_trak_t *trak, long moov_time_scale);
__int64 oqt_track_samples(oqt_t *file, oqt_trak_t *trak);
__int64 oqt_sample_of_chunk(oqt_trak_t *trak, long chunk);
int oqt_chunk_of_sample(__int64 *chunk_sample, long *chunk, oqt_trak_t *trak, long *sd_id, __int64 sample);
__int64 oqt_chunk_to_offset(oqt_trak_t *trak, long chunk);
//__int64 oqt_chunk_to_offset(oqt_trak_t *trak, long chunk);
__int64 oqt_sample_range_size(oqt_trak_t *trak, __int64 chunk_sample, __int64 sample);
__int64 oqt_sample_to_offset(oqt_trak_t *trak, long *sd_id, __int64 sample);
int oqt_update_tables( oqt_t *file, oqt_trak_t *trak, char *url, __int64 offset, __int64 chunk, __int64 sample, __int64 num_samples, long sample_size, long num_frames, long *frame_size_array);
int oqt_trak_duration(oqt_trak_t *trak, long *duration, long *timescale);
int oqt_trak_fix_counts(oqt_t *file, oqt_trak_t *trak);
__int64 oqt_chunk_samples(oqt_trak_t *trak, long chunk);
int oqt_trak_shift_offsets(oqt_trak_t *trak, __int64 offset);

/* In udta.c */
int oqt_udta_init(oqt_moov_t *moov);
int oqt_udta_delete(oqt_moov_t *moov);
void oqt_udta_dump(oqt_moov_t *moov);
int oqt_set_udta_value(oqt_moov_t *moov, char *code, char *value, int size);
int oqt_read_udta(oqt_t *file, oqt_moov_t *moov, oqt_atom_t *udta_atom);
void oqt_write_udta(oqt_t *file, oqt_moov_t *moov);
int oqt_read_udta_string(oqt_t *file, char **string, int *size);
int oqt_write_udta_string(oqt_t *file, char *string, int size);

/* In util.c */
void oqt_copy_char32(char *output, char *input);
void oqt_print_chars(char *desc, char *input, int len);
ULONG oqt_current_time(void);
int oqt_match_32(const char *input, const char *output);
int oqt_read_mp4_descr_length(oqt_t *file);
int oqt_write_mp4_descr_length(oqt_t *file, int length, int compact);
int oqt_get_timescale(float frame_rate);
void oqt_hexdump(const BYTE *data, int size);

/* In vmhd.c */
void oqt_vmhd_init(oqt_vmhd_t *vmhd);
void oqt_vmhd_init_video(oqt_t *file, oqt_vmhd_t *vmhd, int frame_w, int frame_h, float frame_rate);
void oqt_vmhd_delete(oqt_vmhd_t *vmhd);
void oqt_vmhd_dump(oqt_vmhd_t *vmhd);
void oqt_read_vmhd(oqt_t *file, oqt_vmhd_t *vmhd);
void oqt_write_vmhd(oqt_t *file, oqt_vmhd_t *vmhd);

/* In iods.c */
int oqt_write_iods(oqt_t *file, oqt_iods_t *iods);
int oqt_read_iods(oqt_t *file, oqt_iods_t *iods);
int oqt_iods_dump(oqt_iods_t *iods);
int oqt_iods_delete(oqt_iods_t *iods);
int oqt_iods_set_video_profile(oqt_iods_t* iods, int id);
int oqt_iods_set_audio_profile(oqt_iods_t* iods, int id);
int oqt_iods_init(oqt_iods_t *iods);
