/* stsc.c
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
 * $Id: stsc.c,v 1.8 2003/04/03 04:09:41 jhatala Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"



void oqt_stsc_init(oqt_stsc_t *stsc)
{
	stsc->version = 0;
	stsc->flags = 0;
	stsc->total_entries = 0;
	stsc->entries_allocated = 0;
}

void oqt_stsc_init_table(oqt_t *file, oqt_stsc_t *stsc)
{
	if(!stsc->entries_allocated)
	{
		stsc->total_entries = 0;
		stsc->entries_allocated = 2000;
		stsc->table = (oqt_stsc_table_t*)calloc(1, sizeof(oqt_stsc_table_t) * stsc->entries_allocated);
	}
}

void oqt_stsc_init_video(oqt_t *file, oqt_stsc_t *stsc)
{
	oqt_stsc_table_t *table;
	oqt_stsc_init_table(file, stsc);
	table = &(stsc->table[0]);
	table->chunk = 1;
	table->samples = 1;
	table->id = 1;
}

void oqt_stsc_init_audio(oqt_t *file, oqt_stsc_t *stsc, int sample_rate)
{
	oqt_stsc_table_t *table;
	oqt_stsc_init_table(file, stsc);
	table = &(stsc->table[0]);
	table->chunk = 1;
	table->samples = 0;         /* set this after completion or after every audio chunk is written */
	table->id = 1;
}

void oqt_stsc_delete(oqt_stsc_t *stsc)
{
	if(stsc->total_entries) free(stsc->table);
	stsc->total_entries = 0;
}

void oqt_stsc_dump(oqt_stsc_t *stsc)
{
	int i;
	printf("     sample to chunk\n");
	printf("      version %d\n", stsc->version);
	printf("      flags %d\n", stsc->flags);
	printf("      total_entries %d\n", stsc->total_entries);
	for(i = 0; i < stsc->total_entries; i++)
	{
		printf("       chunk %ld samples %ld id %ld\n", 
			(long)stsc->table[i].chunk,
			(long)stsc->table[i].samples,
			(long)stsc->table[i].id);
	}
}

void oqt_read_stsc(oqt_t *file, oqt_stsc_t *stsc)
{
    int i;
	stsc->version = oqt_read_char(file);
	stsc->flags = oqt_read_int24(file);
	stsc->total_entries = oqt_read_int32(file);
	
	stsc->entries_allocated = stsc->total_entries;
	stsc->table = (oqt_stsc_table_t*)malloc(sizeof(oqt_stsc_table_t) * stsc->total_entries);
	for(i = 0; i < stsc->total_entries; i++)
	{
		stsc->table[i].chunk = oqt_read_int32(file);
		stsc->table[i].samples = oqt_read_int32(file);
		stsc->table[i].id = oqt_read_int32(file);
	}
}


void oqt_write_stsc(oqt_t *file, oqt_stsc_t *stsc)
{
	int i, last_same;
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "stsc");

	for(i = 1, last_same = 0; i < stsc->total_entries; i++)
	{
		if(stsc->table[i].samples != stsc->table[last_same].samples
		   || stsc->table[i].id != stsc->table[last_same].id)
		{
/* An entry has a different sample count or sd id. */
			last_same++;
			if(last_same < i)
			{
/* Move it up the list. */
				stsc->table[last_same] = stsc->table[i];
			}
		}
	}
	last_same++;
	stsc->total_entries = last_same;


	oqt_write_char(file, (char)stsc->version);
	oqt_write_int24(file, stsc->flags);
	oqt_write_int32(file, stsc->total_entries);
	for(i = 0; i < stsc->total_entries; i++)
	{
		oqt_write_int32(file, stsc->table[i].chunk);
		oqt_write_int32(file, stsc->table[i].samples);
		oqt_write_int32(file, stsc->table[i].id);
	}

	oqt_atom_write_footer(file, &atom);
}

int oqt_update_stsc(oqt_stsc_t *stsc, long chunk, __int64 samples, int id)
{
	if(chunk > stsc->entries_allocated)
	{
		stsc->entries_allocated = chunk * 2;
		stsc->table =(oqt_stsc_table_t*)realloc(stsc->table, sizeof(oqt_stsc_table_t) * stsc->entries_allocated);
	}

	if(chunk > 0) {
		stsc->table[chunk - 1].samples = (long)samples;
		stsc->table[chunk - 1].chunk = chunk;
		stsc->table[chunk - 1].id = id;
	} else {
		fprintf(stderr, "oqt_update_stsc: chunk <= 0\n");
	}

	if(chunk > stsc->total_entries) stsc->total_entries = chunk;
	return 0;
}

/* Optimizing while writing doesn't allow seeks during recording so */
/* entries are created for every chunk and only optimized during */
/* writeout.  Unfortunately there's no way to keep audio synchronized */
/* after overwriting  a recording as the fractional audio chunk in the */
/* middle always overwrites the previous location of a larger chunk.  On */
/* writing, the table must be optimized.  RealProducer requires an  */
/* optimized table. */

