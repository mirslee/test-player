/* stsd.c
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
 * $Id: stsd.c,v 1.6 2003/02/26 11:40:26 jhatala Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_stsd_init(oqt_stsd_t *stsd)
{
	stsd->version = 0;
	stsd->flags = 0;
	stsd->total_entries = 0;
}

void oqt_stsd_init_table(oqt_stsd_t *stsd)
{
	if(!stsd->total_entries)
	{
		stsd->total_entries = 1;
		stsd->table = (oqt_stsd_table_t*)calloc(1, sizeof(oqt_stsd_table_t) * stsd->total_entries);
		oqt_stsd_table_init(&(stsd->table[0]));
	}
}

void oqt_stsd_init_video(oqt_t *file, 
								oqt_stsd_t *stsd, 
								int frame_w,
								int frame_h, 
								float frame_rate,
								char *compression)
{
	oqt_stsd_table_t *table;
	oqt_stsd_init_table(stsd);
	table = &(stsd->table[0]);

	oqt_copy_char32(table->format, compression);
//	sprintf(table->compressor_name,compression);

	table->width = frame_w;
	table->height = frame_h;
	table->frames_per_sample = 1;
	table->depth = 24;
	table->ctab_id = 65535;
}

void oqt_stsd_init_audio(oqt_t *file, 
							oqt_stsd_t *stsd, 
							int channels,
							int sample_rate, 
							int bits, 
							char *compressor)
{
	oqt_stsd_table_t *table;
	oqt_stsd_init_table(stsd);
	table = &(stsd->table[0]);

	oqt_copy_char32(table->format, compressor);
	table->channels = channels;
	table->sample_size = bits;
	table->sample_rate = (float)sample_rate;
}

void oqt_stsd_delete(oqt_stsd_t *stsd)
{
	int i;
	if(stsd->total_entries)
	{
		for(i = 0; i < stsd->total_entries; i++)
			oqt_stsd_table_delete(&(stsd->table[i]));
		free(stsd->table);
	}

	stsd->total_entries = 0;
}

void oqt_stsd_dump(void *minf_ptr, oqt_stsd_t *stsd)
{
	long i;
	printf("     sample description\n");
	printf("      version %d\n", stsd->version);
	printf("      flags %ld\n", (long)stsd->flags);
	printf("      total_entries %ld\n", (long)stsd->total_entries);
	
	for(i = 0; i < stsd->total_entries; i++)
	{
		oqt_stsd_table_dump(minf_ptr, &(stsd->table[i]));
	}
}

void oqt_read_stsd(oqt_t *file, oqt_minf_t *minf, oqt_stsd_t *stsd)
{
	long i;

	stsd->version = oqt_read_char(file);
	stsd->flags = oqt_read_int24(file);
	stsd->total_entries = oqt_read_int32(file);
	stsd->table = (oqt_stsd_table_t*)malloc(sizeof(oqt_stsd_table_t) * stsd->total_entries);
	for(i = 0; i < stsd->total_entries; i++)
	{
		oqt_stsd_table_init(&(stsd->table[i]));
		oqt_read_stsd_table(file, minf, &(stsd->table[i]));
	}
}

void oqt_write_stsd(oqt_t *file, oqt_minf_t *minf, oqt_stsd_t *stsd)
{
	oqt_atom_t atom;
	long i;
	oqt_atom_write_header(file, &atom, "stsd");

	oqt_write_char(file, (char)stsd->version);
	oqt_write_int24(file, stsd->flags);
	oqt_write_int32(file, stsd->total_entries);
	for(i = 0; i < stsd->total_entries; i++)
	{
		if(i > 0 && stsd->table[i].same_but_external) {
			// copy all the mystical stsd information
			// from the first entry, only preserve
			// the data reference index
			int data_reference = stsd->table[i].data_reference;
			memcpy(&(stsd->table[i]), &(stsd->table[0]), sizeof(oqt_stsd_table_t));
			stsd->table[i].data_reference = data_reference;
			stsd->table[i].same_but_external = 1;
		}

		oqt_write_stsd_table(file, minf, &(stsd->table[i]));
	}

	oqt_atom_write_footer(file, &atom);
}

int oqt_stsd_find_reference_entry(oqt_stsd_t *stsd, int data_reference)
{
	int i;
	for(i = 0; i < stsd->total_entries; ++i) {
		if(stsd->table[i].data_reference == data_reference) {
			// indexes in quicktime are 1-based
			return i+1;
		}
	}
	return -1;
}

int oqt_stsd_add_reference_entry(oqt_stsd_t *stsd, int data_reference)
{
	oqt_stsd_table_t *table;

	stsd->total_entries += 1;
	stsd->table = (oqt_stsd_table_t *)realloc(stsd->table, sizeof(oqt_stsd_table_t) * stsd->total_entries);

	table = &(stsd->table[stsd->total_entries-1]);
	memcpy(table, &(stsd->table[0]), sizeof(oqt_stsd_table_t));
	table->data_reference = data_reference;
	table->same_but_external = 1;

	return stsd->total_entries;
}
