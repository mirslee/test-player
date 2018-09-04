/* tkhd.c
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
 * $Id: tkhd.c,v 1.6 2003/04/07 21:02:26 shitowax Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"

int oqt_tkhd_init(oqt_tkhd_t *tkhd)
{
	int i;
	tkhd->version = 0;
	tkhd->flags = 15;
	tkhd->creation_time = oqt_current_time();
	tkhd->modification_time = oqt_current_time();
	tkhd->track_id = 0;
	tkhd->reserved1 = 0;
	tkhd->duration = 0;      /* need to set this when closing */
	for(i = 0; i < 8; i++) tkhd->reserved2[i] = 0;
	tkhd->layer = 0;
	tkhd->alternate_group = 0;
	tkhd->volume = 1.0;
	tkhd->reserved3 = 0;
	oqt_matrix_init(&(tkhd->matrix));
	tkhd->track_width = 0;
	tkhd->track_height = 0;
	return 0;
}

int oqt_tkhd_delete(oqt_tkhd_t *tkhd)
{
	return 0;
}

void oqt_tkhd_dump(oqt_tkhd_t *tkhd)
{
	printf("  track header\n");
	printf("   version %d\n", tkhd->version);
	printf("   flags %ld\n", (long)tkhd->flags);
	printf("   creation_time %lu\n", (unsigned long)tkhd->creation_time);
	printf("   modification_time %lu\n", (unsigned long)tkhd->modification_time);
	printf("   track_id %d\n", tkhd->track_id);
	printf("   reserved1 %ld\n", (long)tkhd->reserved1);
	printf("   duration %ld\n", (long)tkhd->duration);
	oqt_print_chars("   reserved2 ", (char*)tkhd->reserved2, 8);
	printf("   layer %d\n", tkhd->layer);
	printf("   alternate_group %d\n", tkhd->alternate_group);
	printf("   volume %f\n", (double)tkhd->volume);
	printf("   reserved3 %ld\n", (long)tkhd->reserved3);
	oqt_matrix_dump(&(tkhd->matrix));
	printf("   track_width %f\n", (double)tkhd->track_width);
	printf("   track_height %f\n", (double)tkhd->track_height);
}

void oqt_read_tkhd(oqt_t *file, oqt_tkhd_t *tkhd)
{
//printf("oqt_read_tkhd 1 0x%llx\n", oqt_get_position(file));
	tkhd->version = oqt_read_char(file);
	tkhd->flags = oqt_read_int24(file);
	tkhd->creation_time = oqt_read_int32(file);
	tkhd->modification_time = oqt_read_int32(file);
	tkhd->track_id = oqt_read_int32(file);
	tkhd->reserved1 = oqt_read_int32(file);
	tkhd->duration = oqt_read_int32(file);
	oqt_read_data(file, (char*)tkhd->reserved2, 8);
	tkhd->layer = oqt_read_int16(file);
	tkhd->alternate_group = oqt_read_int16(file);
//printf("oqt_read_tkhd 1 0x%llx\n", oqt_get_position(file));
	tkhd->volume = oqt_read_fixed16(file);
//printf("oqt_read_tkhd 2\n");
	tkhd->reserved3 = oqt_read_int16(file);
	oqt_read_matrix(file, &(tkhd->matrix));
	tkhd->track_width = oqt_read_fixed32(file);
	tkhd->track_height = oqt_read_fixed32(file);
}

void oqt_write_tkhd(oqt_t *file, oqt_tkhd_t *tkhd)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "tkhd");
	oqt_write_char(file, (char)tkhd->version);
	oqt_write_int24(file, tkhd->flags);
	oqt_write_int32(file, tkhd->creation_time);
	oqt_write_int32(file, tkhd->modification_time);
	oqt_write_int32(file, tkhd->track_id);
	oqt_write_int32(file, tkhd->reserved1);
	oqt_write_int32(file, tkhd->duration);
	oqt_write_data(file, (char*)tkhd->reserved2, 8);
	oqt_write_int16(file, (short)tkhd->layer);
	oqt_write_int16(file, (short)tkhd->alternate_group);
	oqt_write_fixed16(file, tkhd->volume);
	oqt_write_int16(file, (short)tkhd->reserved3);
	oqt_write_matrix(file, &(tkhd->matrix));
 	oqt_write_fixed32(file, tkhd->track_width);
 	oqt_write_fixed32(file, tkhd->track_height);
	oqt_atom_write_footer(file, &atom);
}


void oqt_tkhd_init_video(oqt_t *file, 
								oqt_tkhd_t *tkhd, 
								int frame_w, 
								int frame_h)
{
	tkhd->track_width = (short)frame_w;
	tkhd->track_height = (short)frame_h;
	tkhd->volume = 0;
}
