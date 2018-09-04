/* mdia.c
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
 * $Id: mdia.c,v 1.5 2002/12/15 01:51:58 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_mdia_init(oqt_mdia_t *mdia)
{
	oqt_mdhd_init(&(mdia->mdhd));
	oqt_hdlr_init(&(mdia->hdlr));
	oqt_minf_init(&(mdia->minf));
}

void oqt_mdia_init_video(oqt_t *file, 
						oqt_mdia_t *mdia,
						int frame_w,
						int frame_h, 
						float frame_rate,
						char *compressor)
{
//printf("oqt_mdia_init_video 1\n");
	oqt_mdhd_init_video(file, &(mdia->mdhd), frame_w, frame_h, frame_rate);
//printf("oqt_mdia_init_video 1 %d %f\n", mdia->mdhd.time_scale, (double)frame_rate);
	oqt_minf_init_video(file, &(mdia->minf), frame_w, frame_h, mdia->mdhd.time_scale, frame_rate, compressor);
//printf("oqt_mdia_init_video 1\n");
	oqt_hdlr_init_video(&(mdia->hdlr));
//printf("oqt_mdia_init_video 2\n");
}

void oqt_mdia_init_audio(oqt_t *file, 
						oqt_mdia_t *mdia, 
						int channels,
						int sample_rate, 
						int bits, 
						char *compressor)
{
	oqt_mdhd_init_audio(file, &(mdia->mdhd), channels, sample_rate, bits, compressor);
	oqt_minf_init_audio(file, &(mdia->minf), channels, sample_rate, bits, compressor);
	oqt_hdlr_init_audio(&(mdia->hdlr));
}

void oqt_mdia_delete(oqt_mdia_t *mdia)
{
	oqt_mdhd_delete(&(mdia->mdhd));
	oqt_hdlr_delete(&(mdia->hdlr));
	oqt_minf_delete(&(mdia->minf));
}

void oqt_mdia_dump(oqt_mdia_t *mdia)
{
	printf("  media\n");
	oqt_mdhd_dump(&(mdia->mdhd));
	oqt_hdlr_dump(&(mdia->hdlr));
	oqt_minf_dump(&(mdia->minf));
}

int oqt_read_mdia(oqt_t *file, oqt_mdia_t *mdia, oqt_atom_t *trak_atom)
{
	oqt_atom_t leaf_atom;

	do
	{
		oqt_atom_read_header(file, &leaf_atom);
//printf("oqt_read_mdia 0x%llx\n", oqt_get_position(file));

/* mandatory */
		if(oqt_atom_is(&leaf_atom, "mdhd"))
			{ oqt_read_mdhd(file, &(mdia->mdhd)); }
		else
		if(oqt_atom_is(&leaf_atom, "hdlr"))
		{
			oqt_read_hdlr(file, &(mdia->hdlr)); 
/* Main Actor doesn't write component name */
			oqt_atom_skip(file, &leaf_atom);
		}
		else
		if(oqt_atom_is(&leaf_atom, "minf"))
			{ oqt_read_minf(file, &(mdia->minf), &leaf_atom); }
		else
			oqt_atom_skip(file, &leaf_atom);
	}while(oqt_get_position(file) < trak_atom->end);


	return 0;
}

void oqt_write_mdia(oqt_t *file, oqt_mdia_t *mdia)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "mdia");

	oqt_write_mdhd(file, &(mdia->mdhd));
	oqt_write_hdlr(file, &(mdia->hdlr));
	oqt_write_minf(file, &(mdia->minf));

	oqt_atom_write_footer(file, &atom);
}
