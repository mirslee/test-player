/* stts.c
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
 * $Id: stts.c,v 1.5 2002/12/15 01:51:59 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_stts_init(oqt_stts_t *stts)
{
	stts->version = 0;
	stts->flags = 0;
	stts->total_entries = 0;
}

void oqt_stts_init_table(oqt_stts_t *stts)
{
	if(!stts->total_entries)
	{
		stts->total_entries = 1;
		stts->table = (oqt_stts_table_t*)malloc(sizeof(oqt_stts_table_t) * stts->total_entries);
	}
}

void oqt_stts_init_video(oqt_t *file, oqt_stts_t *stts, int time_scale, float frame_rate)
{
	oqt_stts_table_t *table;
	oqt_stts_init_table(stts);
	table = &(stts->table[0]);

	table->sample_count = 0;      /* need to set this when closing */
	table->sample_duration = (ULONG)(time_scale / frame_rate);
//printf("oqt_stts_init_video %ld %f\n", time_scale, (double)frame_rate);
}

void oqt_stts_init_audio(oqt_t *file, oqt_stts_t *stts, int sample_rate)
{
	oqt_stts_table_t *table;
	oqt_stts_init_table(stts);
	table = &(stts->table[0]);

	table->sample_count = 0;     /* need to set this when closing */
	table->sample_duration = 1;
}

void oqt_stts_delete(oqt_stts_t *stts)
{
	if(stts->total_entries) free(stts->table);
	stts->total_entries = 0;
}

void oqt_stts_dump(oqt_stts_t *stts)
{
	int i;
	printf("     time to sample\n");
	printf("      version %d\n", stts->version);
	printf("      flags %ld\n", (long)stts->flags);
	printf("      total_entries %ld\n", (long)stts->total_entries);
	for(i = 0; i < stts->total_entries; i++)
	{
		printf("       count %ld duration %ld\n",
			(long)stts->table[i].sample_count,
			(long)stts->table[i].sample_duration);
	}
}

void oqt_read_stts(oqt_t *file, oqt_stts_t *stts)
{
	int i;
	stts->version = oqt_read_char(file);
	stts->flags = oqt_read_int24(file);
	stts->total_entries = oqt_read_int32(file);

	stts->table = (oqt_stts_table_t*)malloc(sizeof(oqt_stts_table_t) * stts->total_entries);
	for(i = 0; i < stts->total_entries; i++)
	{
		stts->table[i].sample_count = oqt_read_int32(file);
		stts->table[i].sample_duration = oqt_read_int32(file);
	}
}

void oqt_write_stts(oqt_t *file, oqt_stts_t *stts)
{
	int i;
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "stts");

	oqt_write_char(file, (char)stts->version);
	oqt_write_int24(file, stts->flags);
	oqt_write_int32(file, stts->total_entries);
	for(i = 0; i < stts->total_entries; i++)
	{
		oqt_write_int32(file, stts->table[i].sample_count);
		oqt_write_int32(file, stts->table[i].sample_duration);
	}

	oqt_atom_write_footer(file, &atom);
}
