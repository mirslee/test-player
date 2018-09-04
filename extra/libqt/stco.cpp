/* stco.c
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
 * $Id: stco.c,v 1.7 2002/12/15 01:51:59 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"



void oqt_stco_init(oqt_stco_t *stco)
{
	stco->version = 0;
	stco->flags = 0;
	stco->total_entries = 0;
	stco->entries_allocated = 0;
}

void oqt_stco_delete(oqt_stco_t *stco)
{
	if(stco->total_entries) free(stco->table);
	stco->total_entries = 0;
	stco->entries_allocated = 0;
}

void oqt_stco_init_common(oqt_t *file, oqt_stco_t *stco)
{
	if(!stco->entries_allocated)
	{
		stco->entries_allocated = 2000;
		stco->total_entries = 0;
		stco->table = (oqt_stco_table_t*)malloc(sizeof(oqt_stco_table_t) * stco->entries_allocated);
/*printf("oqt_stco_init_common %x\n", stco->table); */
	}
}

void oqt_stco_dump(oqt_stco_t *stco)
{
	int i;
	printf("     chunk offset\n");
	printf("      version %d\n", stco->version);
	printf("      flags %ld\n", (long)stco->flags);
	printf("      total_entries %ld\n", (long)stco->total_entries);
	printf("      entries_allocated %ld\n", (long)stco->entries_allocated);
	for(i = 0; i < stco->total_entries; i++)
	{
		printf("       offset %d 0x%llx\n", i, stco->table[i].offset);
	}
}

void oqt_read_stco(oqt_t *file, oqt_stco_t *stco)
{
	int i;
	stco->version = oqt_read_char(file);
	stco->flags = oqt_read_int24(file);
	stco->total_entries = oqt_read_int32(file);
	stco->entries_allocated = stco->total_entries;
	stco->table = (oqt_stco_table_t*)calloc(1, sizeof(oqt_stco_table_t) * stco->entries_allocated);
	for(i = 0; i < stco->total_entries; i++)
	{
		stco->table[i].offset = oqt_read_uint32(file);
	}
}

void oqt_read_stco64(oqt_t *file, oqt_stco_t *stco)
{
	int i;
	stco->version = oqt_read_char(file);
	stco->flags = oqt_read_int24(file);
	stco->total_entries = oqt_read_int32(file);
	stco->entries_allocated = stco->total_entries;
	stco->table = (oqt_stco_table_t*)calloc(1, sizeof(oqt_stco_table_t) * stco->entries_allocated);
	for(i = 0; i < stco->total_entries; i++)
	{
		stco->table[i].offset = oqt_read_int64(file);
	}
}

void oqt_write_stco(oqt_t *file, oqt_stco_t *stco)
{
	int i;
	oqt_atom_t atom;
//	oqt_atom_write_header(file, &atom, "stco");
	oqt_atom_write_header(file, &atom, "co64");

	oqt_write_char(file, (char)stco->version);
	oqt_write_int24(file, stco->flags);
	oqt_write_int32(file, stco->total_entries);
	for(i = 0; i < stco->total_entries; i++)
	{
//		oqt_write_int32(file, (long)stco->table[i].offset);
		oqt_write_int64(file, stco->table[i].offset);
	}

	oqt_atom_write_footer(file, &atom);
}

void oqt_update_stco(oqt_stco_t *stco, long chunk, __int64 offset)
{
	if(chunk > stco->entries_allocated)
	{
		stco->entries_allocated = chunk * 2;
		stco->table = (oqt_stco_table_t*)realloc(stco->table, sizeof(oqt_stco_table_t) * stco->entries_allocated);
	}
	
	if(chunk > 0) {
		stco->table[chunk - 1].offset = offset;
	} else {
		fprintf(stderr, "oqt_update_stco: chunk <= 0\n");
	}
	if(chunk > stco->total_entries) stco->total_entries = chunk;
}


