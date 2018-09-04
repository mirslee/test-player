#include "stdafx.h"
#include "lqt_private.h"



void quicktime_sdtp_init(quicktime_sdtp_t *sdtp)
{
	sdtp->version = 0;
	sdtp->flags = 0;
	sdtp->total_entries = 0;
	sdtp->entries_allocated = 2;
	sdtp->table = (quicktime_sdtp_table_t *)calloc(sdtp->entries_allocated, sizeof(*sdtp->table));;
}

void quicktime_sdtp_delete(quicktime_sdtp_t *sdtp)
{
	if (sdtp->entries_allocated) 
		free(sdtp->table);
	sdtp->total_entries = 0;
	sdtp->entries_allocated = 0;
	sdtp->table = 0;
}

void quicktime_sdtp_dump(quicktime_sdtp_t *sdtp)
{
	lqt_dump("      Sample Dependency(sdtp)\n");
	lqt_dump("      version %d\n", sdtp->version);
	lqt_dump("      flags %ld\n", sdtp->flags);
	lqt_dump("      total_entries %ld\n", sdtp->total_entries);
	for (int i = 0; i < sdtp->total_entries; i++)
		lqt_dump("       sampledependency 0x%x\n", sdtp->table[i].sampledependency);
}

void quicktime_read_sdtp(quicktime_t *file, quicktime_sdtp_t *sdtp, quicktime_atom_t* atom)
{
	sdtp->version = quicktime_read_char(file);
	sdtp->flags = quicktime_read_int24(file);
	sdtp->total_entries = (int)(atom->end-quicktime_position(file));
	if (sdtp->entries_allocated < sdtp->total_entries)
	{
		sdtp->entries_allocated = sdtp->total_entries;
		sdtp->table = (quicktime_sdtp_table_t*)realloc(sdtp->table, sizeof(quicktime_sdtp_table_t) * sdtp->entries_allocated);
	}
	for (int i = 0; i < sdtp->total_entries; i++)
	{
		sdtp->table[i].sampledependency = quicktime_read_char(file);
	}
}


void quicktime_write_sdtp(quicktime_t *file, quicktime_sdtp_t *sdtp)
{
	if (sdtp->total_entries)
	{
		quicktime_atom_t atom;
		quicktime_atom_write_header(file, &atom, "sdtp");
		quicktime_write_char(file, sdtp->version);
		quicktime_write_int24(file, sdtp->flags);
		for (int i = 0; i < sdtp->total_entries; i++)
		{
			quicktime_write_char(file, sdtp->table[i].sampledependency);
		}
		quicktime_atom_write_footer(file, &atom);
	}
}


void quicktime_update_sdtp(quicktime_sdtp_t *sdtp, int sample, uint8_t sampledependency)
{
	if (sample >= sdtp->entries_allocated)
	{
		sdtp->entries_allocated = sample + 1024;
		sdtp->table = (quicktime_sdtp_table_t *)realloc(sdtp->table,
			sdtp->entries_allocated * sizeof(*(sdtp->table)));
	}
	sdtp->table[sample].sampledependency = sampledependency;
	if (sample >= sdtp->total_entries)
		sdtp->total_entries = sample + 1;
}