/* dref.c
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
 * $Id: dref.c,v 1.7 2003/02/26 11:40:26 jhatala Exp $
 */

#include <stdlib.h>

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"

#pragma warning(disable:4996)

 
void oqt_dref_table_init(oqt_dref_table_t *table)
{
	table->size = 0;
	table->type[0] = 'a';
	table->type[1] = 'l';
	table->type[2] = 'i';
	table->type[3] = 's';
	table->version = 0;
	table->flags = 0x0001;
	table->data_reference = (char*)malloc(256);
	table->data_reference[0] = 0;
}

void oqt_dref_table_delete(oqt_dref_table_t *table)
{
	if(table->data_reference) free(table->data_reference);
	table->data_reference = 0;
}

void oqt_read_dref_table(oqt_t *file, oqt_dref_table_t *table)
{
	table->size = oqt_read_int32(file);
	oqt_read_char32(file, table->type);
	table->version = oqt_read_char(file);
	table->flags = oqt_read_int24(file);
	if(table->data_reference) free(table->data_reference);

	table->data_reference = (char*)malloc((size_t)table->size);
	if(table->size > 12)
	  oqt_read_data(file, table->data_reference, table->size - 12);
	table->data_reference[table->size - 12] = 0;
}

void oqt_write_dref_table(oqt_t *file, oqt_dref_table_t *table)
{
	int len = (int)strlen(table->data_reference) + 1;
	if(len == 1) len = 0;
	oqt_write_int32(file, 12 + len);
	oqt_write_char32(file, table->type);
	oqt_write_char(file, (char)table->version);
	oqt_write_int24(file, table->flags);
	if(len) oqt_write_data(file, table->data_reference, len);
}

void oqt_dref_table_dump(oqt_dref_table_t *table)
{
	printf("			data reference table (dref)\n");
	printf("			 type %c%c%c%c\n", table->type[0], table->type[1], table->type[2], table->type[3]);
	printf("			 version %d\n", table->version);
	printf("			 flags %d\n", table->flags);
	printf("			 size %d\n", (int)table->size);
	printf("			 data dump follows:\n");

	oqt_hexdump((const BYTE *)table->data_reference, (int)table->size - 12);
}


void oqt_dref_init(oqt_dref_t *dref)
{
	dref->version = 0;
	dref->flags = 0;
	dref->total_entries = 0;
	dref->table = 0;
}

void oqt_dref_init_all(oqt_dref_t *dref)
{
	if(!dref->total_entries)
	{
		dref->total_entries = 1;
		dref->table = (oqt_dref_table_t *)malloc(sizeof(oqt_dref_table_t) * dref->total_entries);
		oqt_dref_table_init(&(dref->table[0]));
	}
}

void oqt_dref_delete(oqt_dref_t *dref)
{
	if(dref->table)
	{
		int i;
		for(i = 0; i < dref->total_entries; i++)
			oqt_dref_table_delete(&(dref->table[i]));
		free(dref->table);
	}
	dref->total_entries = 0;
}

void oqt_dref_dump(oqt_dref_t *dref)
{
	long i;
	
	printf("     data reference (dref)\n");
	printf("      version %d\n", dref->version);
	printf("      flags %d\n", dref->flags);
	for(i = 0; i < dref->total_entries; i++)
	{
		oqt_dref_table_dump(&(dref->table[i]));
	}
}

void oqt_read_dref(oqt_t *file, oqt_dref_t *dref)
{
	long i;

	dref->version = oqt_read_char(file);
	dref->flags = oqt_read_int24(file);
	dref->total_entries = oqt_read_int32(file);
	dref->table = (oqt_dref_table_t*)malloc(sizeof(oqt_dref_table_t) * dref->total_entries);
	for(i = 0; i < dref->total_entries; i++)
	{
		oqt_dref_table_init(&(dref->table[i]));
		oqt_read_dref_table(file, &(dref->table[i]));
	}
}

void oqt_write_dref(oqt_t *file, oqt_dref_t *dref)
{
	long i;
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "dref");

	oqt_write_char(file, (char)dref->version);
	oqt_write_int24(file, dref->flags);
	oqt_write_int32(file, dref->total_entries);

	for(i = 0; i < dref->total_entries; i++)
	{
		oqt_write_dref_table(file, &(dref->table[i]));
	}
	oqt_atom_write_footer(file, &atom);
}

int oqt_dref_find_url_entry(oqt_dref_t *dref, char *url)
{
	int i;
	for(i = 0; i < dref->total_entries; ++i) {
		oqt_dref_table_t *table = &(dref->table[i]);
		if(table->flags != 1 &&
		   memcmp(table->type, "url ", 4) == 0 &&
		   table->data_reference &&
		   strcmp(table->data_reference, url) == 0) {
		 	// indexes in quicktime are 1-based
		 	return i + 1;
		 }
	}
	return -1;
}

int oqt_dref_add_url_entry(oqt_dref_t *dref, char *url)
{
	oqt_dref_table_t *table;

	dref->total_entries += 1;
	dref->table = (oqt_dref_table_t *)realloc(dref->table, sizeof(oqt_dref_table_t) * dref->total_entries);

	table = &(dref->table[dref->total_entries-1]);

	table->size = 12 + strlen(url) + 1;
	table->type[0] = 'u';
	table->type[1] = 'r';
	table->type[2] = 'l';
	table->type[3] = ' ';
	table->version = 0;
	table->flags = 0;
	table->data_reference = strdup(url);

	return dref->total_entries;
}
