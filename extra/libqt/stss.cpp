/* stss.c
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
 * $Id: stss.c,v 1.4 2002/10/28 13:36:45 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"



void oqt_stss_init(oqt_stss_t *stss)
{
	stss->version = 0;
	stss->flags = 0;
	stss->total_entries = 0;
}

void oqt_stss_delete(oqt_stss_t *stss)
{
	if(stss->total_entries) free(stss->table);
	stss->total_entries = 0;
}

void oqt_stss_dump(oqt_stss_t *stss)
{
	long i;
	printf("     sync sample\n");
	printf("      version %d\n", stss->version);
	printf("      flags %ld\n", (long)stss->flags);
	printf("      total_entries %ld\n", (long)stss->total_entries);
	printf("      entries_allocated %ld\n", (long)stss->entries_allocated);
	for(i = 0; i < stss->total_entries; i++)
	{
		printf("       sample %lx\n", (long)stss->table[i].sample);
	}
}

void oqt_read_stss(oqt_t *file, oqt_stss_t *stss)
{
	long i;
	stss->version = oqt_read_char(file);
	stss->flags = oqt_read_int24(file);
	stss->total_entries = oqt_read_int32(file);
	
	stss->table = (oqt_stss_table_t*)malloc(sizeof(oqt_stss_table_t) * stss->total_entries);
	for(i = 0; i < stss->total_entries; i++)
	{
		stss->table[i].sample = oqt_read_int32(file);
	}
}


void oqt_write_stss(oqt_t *file, oqt_stss_t *stss)
{
	long i;
	oqt_atom_t atom;

	//	printf("Je suis dans oqt_write_stss %u\n", stss->total_entries);

	if(stss->total_entries)
	{
		oqt_atom_write_header(file, &atom, "stss");

		oqt_write_char(file, (char)stss->version);
		oqt_write_int24(file, stss->flags);
		oqt_write_int32(file, stss->total_entries);
		for(i = 0; i < stss->total_entries; i++)
		{
			oqt_write_int32(file, stss->table[i].sample);
		}

		oqt_atom_write_footer(file, &atom);
	}
}
