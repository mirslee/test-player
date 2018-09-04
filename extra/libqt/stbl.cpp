/* stbl.c
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
 * $Id: stbl.c,v 1.6 2002/12/15 01:51:59 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_stbl_init(oqt_stbl_t *stbl)
{
	stbl->version = 0;
	stbl->flags = 0;
	oqt_stsd_init(&(stbl->stsd));
	oqt_stts_init(&(stbl->stts));
	oqt_stss_init(&(stbl->stss));
	oqt_stsc_init(&(stbl->stsc));
	oqt_stsz_init(&(stbl->stsz));
	oqt_stco_init(&(stbl->stco));
}

void oqt_stbl_init_video(oqt_t *file, 
								oqt_stbl_t *stbl, 
								int frame_w,
								int frame_h, 
								int time_scale, 
								float frame_rate,
								char *compressor)
{
	oqt_stsd_init_video(file, &(stbl->stsd), frame_w, frame_h, frame_rate, compressor);
	oqt_stts_init_video(file, &(stbl->stts), time_scale, frame_rate);
	oqt_stsc_init_video(file, &(stbl->stsc));
	oqt_stsz_init_video(file, &(stbl->stsz));
	oqt_stco_init_common(file, &(stbl->stco));
}


void oqt_stbl_init_audio(oqt_t *file, 
							oqt_stbl_t *stbl, 
							int channels, 
							int sample_rate, 
							int bits, 
							char *compressor)
{
	oqt_stsd_init_audio(file, &(stbl->stsd), channels, sample_rate, bits, compressor);
	oqt_stts_init_audio(file, &(stbl->stts), sample_rate);
	oqt_stsc_init_audio(file, &(stbl->stsc), sample_rate);
	oqt_stsz_init_audio(file, &(stbl->stsz), channels, bits);
	oqt_stco_init_common(file, &(stbl->stco));
}

void oqt_stbl_delete(oqt_stbl_t *stbl)
{
	oqt_stsd_delete(&(stbl->stsd));
	oqt_stts_delete(&(stbl->stts));
	oqt_stss_delete(&(stbl->stss));
	oqt_stsc_delete(&(stbl->stsc));
	oqt_stsz_delete(&(stbl->stsz));
	oqt_stco_delete(&(stbl->stco));
}

void oqt_stbl_dump(void *minf_ptr, oqt_stbl_t *stbl)
{
	printf("    sample table\n");
	oqt_stsd_dump(minf_ptr, &(stbl->stsd));
	oqt_stts_dump(&(stbl->stts));
	oqt_stss_dump(&(stbl->stss));
	oqt_stsc_dump(&(stbl->stsc));
	oqt_stsz_dump(&(stbl->stsz));
	oqt_stco_dump(&(stbl->stco));
}

int oqt_read_stbl(oqt_t *file, oqt_minf_t *minf, oqt_stbl_t *stbl, oqt_atom_t *parent_atom)
{
	oqt_atom_t leaf_atom;

	do
	{
		oqt_atom_read_header(file, &leaf_atom);

/* mandatory */
		if(oqt_atom_is(&leaf_atom, "stsd"))
		{ 
		  //printf("STSD start %lld end %lld", leaf_atom.start, leaf_atom.end);
			oqt_read_stsd(file, minf, &(stbl->stsd)); 
/* Some codecs store extra information at the end of this */
			oqt_atom_skip(file, &leaf_atom);
		}
		else
		if(oqt_atom_is(&leaf_atom, "stts"))
			{ oqt_read_stts(file, &(stbl->stts)); }
		else
		if(oqt_atom_is(&leaf_atom, "stss"))
			{ oqt_read_stss(file, &(stbl->stss)); }
		else
		if(oqt_atom_is(&leaf_atom, "stsc"))
			{ oqt_read_stsc(file, &(stbl->stsc)); }
		else
		if(oqt_atom_is(&leaf_atom, "stsz"))
			{ oqt_read_stsz(file, &(stbl->stsz)); }
		else
		if(oqt_atom_is(&leaf_atom, "co64"))
			{ oqt_read_stco64(file, &(stbl->stco)); }
		else
		if(oqt_atom_is(&leaf_atom, "stco"))
			{ oqt_read_stco(file, &(stbl->stco)); }
		else
			oqt_atom_skip(file, &leaf_atom);
	}while(oqt_get_position(file) < parent_atom->end);

	return 0;
}

void oqt_write_stbl(oqt_t *file, oqt_minf_t *minf, oqt_stbl_t *stbl)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "stbl");

	oqt_write_stsd(file, minf, &(stbl->stsd));
	oqt_write_stts(file, &(stbl->stts));
	oqt_write_stss(file, &(stbl->stss));
	oqt_write_stsc(file, &(stbl->stsc));
	oqt_write_stsz(file, &(stbl->stsz),minf->is_audio);
	oqt_write_stco(file, &(stbl->stco));

	oqt_atom_write_footer(file, &atom);
}


