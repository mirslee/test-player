/* mvhd.c
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
 * $Id: mvhd.c,v 1.6 2002/12/15 01:51:58 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"



int oqt_mvhd_init(oqt_mvhd_t *mvhd)
{
	int i;
	mvhd->version = 0;
	mvhd->flags = 0;
	mvhd->creation_time = oqt_current_time();
	mvhd->modification_time = oqt_current_time();
	mvhd->time_scale = 600;
	mvhd->duration = 0;
	mvhd->preferred_rate = 1.0;
	mvhd->preferred_volume = 0.996094f; //EH ?!
	//mvhd->preferred_volume = 1.0;
	for(i = 0; i < 10; i++) mvhd->reserved[i] = 0;
	oqt_matrix_init(&(mvhd->matrix));
	mvhd->preview_time = 0;
	mvhd->preview_duration = 0;
	mvhd->poster_time = 0;
	mvhd->selection_time = 0;
	mvhd->selection_duration = 0;
	mvhd->current_time = 0;
	mvhd->next_track_id = 1;
	return 0;
}

int oqt_mvhd_delete(oqt_mvhd_t *mvhd)
{
	return 0;
}

void oqt_mvhd_dump(oqt_mvhd_t *mvhd)
{
	printf(" movie header\n");
	printf("  version %d\n", mvhd->version);
	printf("  flags %ld\n", (long)mvhd->flags);
	printf("  creation_time %lu\n", (unsigned long)mvhd->creation_time);
	printf("  modification_time %lu\n", (unsigned long)mvhd->modification_time);
	printf("  time_scale %ld\n", (long)mvhd->time_scale);
	printf("  duration %ld\n", (long)mvhd->duration);
	printf("  preferred_rate %f\n", (double)mvhd->preferred_rate);
	printf("  preferred_volume %f\n", (double)mvhd->preferred_volume);
	oqt_print_chars("  reserved ", mvhd->reserved, 10);
	oqt_matrix_dump(&(mvhd->matrix));
	printf("  preview_time %ld\n", (long)mvhd->preview_time);
	printf("  preview_duration %ld\n", (long)mvhd->preview_duration);
	printf("  poster_time %ld\n", (long)mvhd->poster_time);
	printf("  selection_time %ld\n", (long)mvhd->selection_time);
	printf("  selection_duration %ld\n", (long)mvhd->selection_duration);
	printf("  current_time %ld\n", (long)mvhd->current_time);
	printf("  next_track_id %ld\n", (long)mvhd->next_track_id);
}

void oqt_read_mvhd(oqt_t *file, oqt_mvhd_t *mvhd)
{
	mvhd->version = oqt_read_char(file);
	mvhd->flags = oqt_read_int24(file);
	mvhd->creation_time = oqt_read_int32(file);
	mvhd->modification_time = oqt_read_int32(file);
	mvhd->time_scale = oqt_read_int32(file);
	mvhd->duration = oqt_read_int32(file);
	mvhd->preferred_rate = oqt_read_fixed32(file);
	mvhd->preferred_volume = oqt_read_fixed16(file);
	oqt_read_data(file, mvhd->reserved, 10);
	oqt_read_matrix(file, &(mvhd->matrix));
	mvhd->preview_time = oqt_read_int32(file);
	mvhd->preview_duration = oqt_read_int32(file);
	mvhd->poster_time = oqt_read_int32(file);
	mvhd->selection_time = oqt_read_int32(file);
	mvhd->selection_duration = oqt_read_int32(file);
	mvhd->current_time = oqt_read_int32(file);
	mvhd->next_track_id = oqt_read_int32(file);
}

void oqt_mhvd_init_video(oqt_t *file, oqt_mvhd_t *mvhd, float frame_rate)
{
	mvhd->time_scale = oqt_get_timescale(frame_rate);
}

void oqt_write_mvhd(oqt_t *file, oqt_mvhd_t *mvhd)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "mvhd");

	oqt_write_char(file, (char)mvhd->version);
	oqt_write_int24(file, mvhd->flags);
	oqt_write_int32(file, mvhd->creation_time);
	oqt_write_int32(file, mvhd->modification_time);
	oqt_write_int32(file, mvhd->time_scale);
	oqt_write_int32(file, mvhd->duration);
	oqt_write_fixed32(file, mvhd->preferred_rate);
	oqt_write_fixed16(file, mvhd->preferred_volume);
	oqt_write_data(file, mvhd->reserved, 10);
	oqt_write_matrix(file, &(mvhd->matrix));
	oqt_write_int32(file, mvhd->preview_time);
	oqt_write_int32(file, mvhd->preview_duration);
	oqt_write_int32(file, mvhd->poster_time);
	oqt_write_int32(file, mvhd->selection_time);
	oqt_write_int32(file, mvhd->selection_duration);
	oqt_write_int32(file, mvhd->current_time);
	oqt_write_int32(file, mvhd->next_track_id);

	oqt_atom_write_footer(file, &atom);
}
