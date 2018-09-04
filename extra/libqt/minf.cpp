/* minf.c
 * Copyright (C) 2001 QT4Linux and OpenQuicktime Teams
 *
 * This file is part of OpenQuicktime, a free QuickTime library.
 *
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
 * $Id: minf.c,v 1.6 2002/12/15 01:51:58 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_minf_init(oqt_minf_t *minf)
{
	minf->is_video = minf->is_audio = 0;
	oqt_vmhd_init(&(minf->vmhd));
	oqt_smhd_init(&(minf->smhd));
	oqt_hdlr_init(&(minf->hdlr));
	oqt_dinf_init(&(minf->dinf));
	oqt_stbl_init(&(minf->stbl));
}

void oqt_minf_init_video(oqt_t *file, 
								oqt_minf_t *minf, 
								int frame_w,
								int frame_h, 
								int time_scale, 
								float frame_rate,
								char *compressor)
{
	minf->is_video = 1;
//printf("oqt_minf_init_video 1\n");
	oqt_vmhd_init_video(file, &(minf->vmhd), frame_w, frame_h, frame_rate);
//printf("oqt_minf_init_video 1 %d %f\n", time_scale, (double)frame_rate);
	oqt_stbl_init_video(file, &(minf->stbl), frame_w, frame_h, time_scale, frame_rate, compressor);
//printf("oqt_minf_init_video 2\n");
	oqt_hdlr_init_data(&(minf->hdlr));
//printf("oqt_minf_init_video 1\n");
	oqt_dinf_init_all(&(minf->dinf));
//printf("oqt_minf_init_video 2\n");
}

void oqt_minf_init_audio(oqt_t *file, 
							oqt_minf_t *minf, 
							int channels, 
							int sample_rate, 
							int bits, 
							char *compressor)
{
	minf->is_audio = 1;
/* smhd doesn't store anything worth initializing */
	oqt_stbl_init_audio(file, &(minf->stbl), channels, sample_rate, bits, compressor);
	oqt_hdlr_init_data(&(minf->hdlr));
	oqt_dinf_init_all(&(minf->dinf));
}

void oqt_minf_delete(oqt_minf_t *minf)
{
	oqt_vmhd_delete(&(minf->vmhd));
	oqt_smhd_delete(&(minf->smhd));
	oqt_dinf_delete(&(minf->dinf));
	oqt_stbl_delete(&(minf->stbl));
	oqt_hdlr_delete(&(minf->hdlr));
}

void oqt_minf_dump(oqt_minf_t *minf)
{
	printf("   media info\n");
	printf("    is_audio %d\n", minf->is_audio);
	printf("    is_video %d\n", minf->is_video);
	if(minf->is_audio) oqt_smhd_dump(&(minf->smhd));
	if(minf->is_video) oqt_vmhd_dump(&(minf->vmhd));
	oqt_hdlr_dump(&(minf->hdlr));
	oqt_dinf_dump(&(minf->dinf));
	oqt_stbl_dump(minf, &(minf->stbl));
}

int oqt_read_minf(oqt_t *file, oqt_minf_t *minf, oqt_atom_t *parent_atom)
{
	oqt_atom_t leaf_atom;
	__int64 pos = oqt_get_position(file);

	do
	{
		oqt_atom_read_header(file, &leaf_atom);
//printf("oqt_read_minf 1\n");

/* mandatory */
		if(oqt_atom_is(&leaf_atom, "vmhd"))
			{ minf->is_video = 1; oqt_read_vmhd(file, &(minf->vmhd)); }
		else
		if(oqt_atom_is(&leaf_atom, "smhd"))
			{ minf->is_audio = 1; oqt_read_smhd(file, &(minf->smhd)); }
		else
		if(oqt_atom_is(&leaf_atom, "gmhd"))
			{ minf->is_strm = 1; oqt_read_gmhd(file, &(minf->gmhd)); }
		else
		if(oqt_atom_is(&leaf_atom, "hdlr"))
			{ 
				oqt_read_hdlr(file, &(minf->hdlr)); 
/* Main Actor doesn't write component name */
				oqt_atom_skip(file, &leaf_atom);
			}
		else
		if(oqt_atom_is(&leaf_atom, "dinf"))
			{ oqt_read_dinf(file, &(minf->dinf), &leaf_atom); }
		else
			oqt_atom_skip(file, &leaf_atom);
	}while(oqt_get_position(file) < parent_atom->end);

	oqt_set_position(file, pos);

	do {
		oqt_atom_read_header(file, &leaf_atom);

		if(oqt_atom_is(&leaf_atom, "stbl")) {
			oqt_read_stbl(file, minf, &(minf->stbl), &leaf_atom);
		} else {
			oqt_atom_skip(file, &leaf_atom);
		}
	} while(oqt_get_position(file) < parent_atom->end);


	return 0;
}

void oqt_write_minf(oqt_t *file, oqt_minf_t *minf)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "minf");

	if(minf->is_video) oqt_write_vmhd(file, &(minf->vmhd));
	if(minf->is_audio) oqt_write_smhd(file, &(minf->smhd));
	oqt_write_hdlr(file, &(minf->hdlr));
	oqt_write_dinf(file, &(minf->dinf));
	oqt_write_stbl(file, minf, &(minf->stbl));

	oqt_atom_write_footer(file, &atom);
}
