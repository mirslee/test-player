/* elst.c
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
 * $Id: elst.c,v 1.4 2002/10/28 13:36:44 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_elst_table_init(oqt_elst_table_t *table)
{
	table->duration = 0;
	table->time = 0;
	table->rate = 1;
}

void oqt_elst_table_delete(oqt_elst_table_t *table)
{
}

void oqt_read_elst_table(oqt_t *file, oqt_elst_table_t *table)
{
	table->duration = oqt_read_int32(file);
	table->time = oqt_read_int32(file);
	table->rate = oqt_read_fixed32(file);
}

void oqt_write_elst_table(oqt_t *file, oqt_elst_table_t *table, long duration)
{
	table->duration = duration;
	oqt_write_int32(file, table->duration);
	oqt_write_int32(file, table->time);
	oqt_write_fixed32(file, table->rate);
}

void oqt_elst_table_dump(oqt_elst_table_t *table)
{
	printf("    edit list table\n");
	printf("     duration %d\n", table->duration);
	printf("     time %d\n", table->time);
	printf("     rate %f\n", (double)table->rate);
}

void oqt_elst_init(oqt_elst_t *elst)
{
	elst->version = 0;
	elst->flags = 0;
	elst->total_entries = 0;
	elst->table = 0;
}

void oqt_elst_init_all(oqt_elst_t *elst)
{
	if(!elst->total_entries)
	{
		elst->total_entries = 1;
		elst->table = (oqt_elst_table_t*)malloc(sizeof(oqt_elst_table_t) * elst->total_entries);
		oqt_elst_table_init(&(elst->table[0]));
	}
}

void oqt_elst_delete(oqt_elst_t *elst)
{
	int i;
	if(elst->total_entries)
	{
		for(i = 0; i < elst->total_entries; i++)
			oqt_elst_table_delete(&(elst->table[i]));
		free(elst->table);
	}
	elst->total_entries = 0;
}

void oqt_elst_dump(oqt_elst_t *elst)
{
	long i;
	printf("   edit list (elst)\n");
	printf("    version %d\n", elst->version);
	printf("    flags %d\n", elst->flags);
	printf("    total_entries %d\n", elst->total_entries);

	for(i = 0; i < elst->total_entries; i++)
	{
		oqt_elst_table_dump(&(elst->table[i]));
	}
}

void oqt_read_elst(oqt_t *file, oqt_elst_t *elst)
{
	long i;

	elst->version = oqt_read_char(file);
	elst->flags = oqt_read_int24(file);
	elst->total_entries = oqt_read_int32(file);
	elst->table = (oqt_elst_table_t*)calloc(1, sizeof(oqt_elst_table_t) * elst->total_entries);
	for(i = 0; i < elst->total_entries; i++)
	{
		oqt_elst_table_init(&(elst->table[i]));
		oqt_read_elst_table(file, &(elst->table[i]));
	}
}

void oqt_write_elst(oqt_t *file, oqt_elst_t *elst, long duration)
{
	oqt_atom_t atom;
	long i;
	oqt_atom_write_header(file, &atom, "elst");

	oqt_write_char(file, (char)elst->version);
	oqt_write_int24(file, elst->flags);
	oqt_write_int32(file, elst->total_entries);
	for(i = 0; i < elst->total_entries; i++)
	{
		oqt_write_elst_table(file, elst->table, duration);
	}

	oqt_atom_write_footer(file, &atom);
}
