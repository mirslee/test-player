/* stsz.c
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
 * $Id: stsz.c,v 1.9 2003/04/07 21:02:25 shitowax Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_stsz_init(oqt_stsz_t *stsz)
{
	stsz->version = 0;
	stsz->flags = 0;
	stsz->sample_size = 0;
	stsz->total_entries = 0;
	stsz->entries_allocated = 0;
}

void oqt_stsz_init_video(oqt_t *file, oqt_stsz_t *stsz)
{
	stsz->sample_size = 0;
	if(!stsz->entries_allocated)
	{
		stsz->entries_allocated = 2000;
		stsz->total_entries = 0;
		stsz->table = (oqt_stsz_table_t*)malloc(sizeof(oqt_stsz_table_t) * stsz->entries_allocated);
	}
}

void oqt_stsz_init_audio(oqt_t *file, oqt_stsz_t *stsz, int channels, int bits)
{
	/*stsz->sample_size = channels * bits / 8; */
	stsz->sample_size = 0;   /* default: use Sample Size Table */
	stsz->total_entries = 0;   /* set this when closing */
	stsz->entries_allocated = 0;
}

void oqt_stsz_delete(oqt_stsz_t *stsz)
{
	if(!stsz->sample_size && stsz->total_entries) free(stsz->table);
	stsz->total_entries = 0;
	stsz->entries_allocated = 0;
}

void oqt_stsz_dump(oqt_stsz_t *stsz)
{
    int i;
	printf("     sample size\n");
	printf("      version %d\n", stsz->version);
	printf("      flags %ld\n", (long)stsz->flags);
	printf("      sample_size %lld\n", stsz->sample_size);
	printf("      total_entries %ld\n", (long)stsz->total_entries);
	
	if(!stsz->sample_size)
	{
		for(i = 0; i < stsz->total_entries; i++)
		{
			printf("       sample_size %lld\n", stsz->table[i].size);
		}
	}
}

void oqt_read_stsz(oqt_t *file, oqt_stsz_t *stsz)
{
	long i;
	stsz->version = oqt_read_char(file);
	stsz->flags = oqt_read_int24(file);
	stsz->sample_size = oqt_read_int32(file);
	stsz->total_entries = oqt_read_int32(file);
	stsz->entries_allocated = stsz->total_entries;
	if(!stsz->sample_size)
	{
		stsz->table = (oqt_stsz_table_t*)malloc(sizeof(oqt_stsz_table_t) * stsz->entries_allocated);
		for(i = 0; i < stsz->total_entries; i++)
		{
			stsz->table[i].size = oqt_read_int32(file);
		}
	}
}

void oqt_write_stsz(oqt_t *file, oqt_stsz_t *stsz,long bAudio)
{
	long i;
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "stsz");

/* optimize if possible */
/* Xanim requires an unoptimized table for video. */
	if(bAudio)
	{
		long result, i; 
 	if(!stsz->sample_size) 
 	{ 
 		for(i = 0, result = 0; i < stsz->total_entries && !result; i++) 
 		{ 
 			if(stsz->table[i].size != stsz->table[0].size) result = 1; 
 		} 
 		 
 		if(!result&&stsz->table) 
 		{ 
//	Yann : This seems correct but it doesn't work at all in qt6 ... I'm starting to think qt6 is buggy =)
// 			stsz->sample_size = stsz->table[0].size; 
 //			stsz->total_entries = 0;
//	Yann : This works but doesn't display the correct birate....
 			stsz->sample_size = 1; 
			stsz->total_entries = (long)(stsz->table[0].size*stsz->total_entries);
//	Yann : qt6 produces something like this,  and this works and gives the correct display of the bitrate
// 			stsz->sample_size = 1; 
//			stsz->total_entries = total number of 16 bits samples ... this is really stupid !!!

 			free(stsz->table); 
 		} 
 	} 
	}

	oqt_write_char(file, (char)stsz->version);
	oqt_write_int24(file, stsz->flags);
	oqt_write_int32(file, (long)stsz->sample_size);
	if(stsz->sample_size)
	{
		oqt_write_int32(file, stsz->total_entries);
	}
	else
	{
		oqt_write_int32(file, stsz->total_entries);
		for(i = 0; i < stsz->total_entries; i++)
		{
			oqt_write_int32(file, (long)stsz->table[i].size);
		}
	}

	oqt_atom_write_footer(file, &atom);
}

void oqt_update_stsz(oqt_stsz_t *stsz, __int64 sample, long sample_size)
{
	if(!stsz->sample_size)
	{
		if(sample >= stsz->entries_allocated)
		{
			int idx_uninitialized = stsz->entries_allocated;
			stsz->entries_allocated = (long)((sample+1) * 2);
			stsz->table = (oqt_stsz_table_t*)realloc(stsz->table, sizeof(oqt_stsz_table_t) * stsz->entries_allocated);
			// zero out newly allocated entries
			memset(stsz->table + idx_uninitialized, 0, sizeof(oqt_stsz_table_t) * (stsz->entries_allocated - idx_uninitialized));
		}

		stsz->table[sample].size = sample_size;
		if(sample >= stsz->total_entries) stsz->total_entries = (long)sample + 1;
	}
}
