/* mdhd.c
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
 * $Id: mdhd.c,v 1.6 2002/12/15 01:51:58 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_mdhd_init(oqt_mdhd_t *mdhd)
{
	mdhd->version = 0;
	mdhd->flags = 0;
	mdhd->creation_time = oqt_current_time();
	mdhd->modification_time = oqt_current_time();
	mdhd->time_scale = 0;
	mdhd->duration = 0;
	mdhd->language = 0;
	mdhd->quality = 100;
}

void oqt_mdhd_init_video(oqt_t *file, 
						oqt_mdhd_t *mdhd, 
						int frame_w,
						int frame_h, 
						float frame_rate)
{
	mdhd->time_scale = oqt_get_timescale(frame_rate);
//printf("oqt_mdhd_init_video %ld %f\n", mdhd->time_scale, (double)frame_rate);
	mdhd->duration = 0;      /* set this when closing */
}

void oqt_mdhd_init_audio(oqt_t *file, 
							oqt_mdhd_t *mdhd, 
							int channels, 
							int sample_rate, 
							int bits, 
							char *compressor)
{
	mdhd->time_scale = sample_rate;
	mdhd->duration = 0;      /* set this when closing */
}

void oqt_mdhd_delete(oqt_mdhd_t *mdhd)
{
}

void oqt_read_mdhd(oqt_t *file, oqt_mdhd_t *mdhd)
{
	mdhd->version = oqt_read_char(file);
	mdhd->flags = oqt_read_int24(file);
	mdhd->creation_time = oqt_read_int32(file);
	mdhd->modification_time = oqt_read_int32(file);
	mdhd->time_scale = oqt_read_int32(file);
	mdhd->duration = oqt_read_int32(file);
	mdhd->language = oqt_read_int16(file);
	mdhd->quality = oqt_read_int16(file);
}

void oqt_mdhd_dump(oqt_mdhd_t *mdhd)
{
	printf("   media header\n");
	printf("    version %d\n", mdhd->version);
	printf("    flags %ld\n", (long)mdhd->flags);
	printf("    creation_time %lu\n", (unsigned long)mdhd->creation_time);
	printf("    modification_time %lu\n", (unsigned long)mdhd->modification_time);
	printf("    time_scale %ld\n", (long)mdhd->time_scale);
	printf("    duration %ld\n", (long)mdhd->duration);
	printf("    language %d\n", mdhd->language);
	printf("    quality %d\n", mdhd->quality);
}

void oqt_write_mdhd(oqt_t *file, oqt_mdhd_t *mdhd)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "mdhd");

	oqt_write_char(file, (char)mdhd->version);
	oqt_write_int24(file, mdhd->flags);
	oqt_write_int32(file, mdhd->creation_time);
	oqt_write_int32(file, mdhd->modification_time);
	oqt_write_int32(file, mdhd->time_scale);
	oqt_write_int32(file, mdhd->duration);
	oqt_write_int16(file, (short)mdhd->language);
	oqt_write_int16(file, (short)mdhd->quality);	

	oqt_atom_write_footer(file, &atom);
}

