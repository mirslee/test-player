/* io.c
 * Copyright (C) 2002 QT4Linux and OpenQuicktime Teams
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
 * $Id: io.c,v 1.9 2003/04/07 21:02:22 shitowax Exp $
 */
 
/* 
 * These are the routines that should be called by the library 
 * and user programs. 
 *
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"



int oqt_read_data(oqt_t *file, char *data, __int64 size)
{
	int result = 1;
//printf("oqt_read_data: file=%p size=%i\n",file,(int)size);
	if(file->decompressed_buffer)
	  {
	    if(file->decompressed_position < file->decompressed_buffer_size)
	      {
				memcpy(data,file->decompressed_buffer+file->decompressed_position,(size_t)size);
				file->decompressed_position+=size;
				return result;
	      }
	    else
	      {
#ifdef DEBUG	      
				fprintf(stderr, "oqt_read_data: deleting decompressed buffer\n");
#endif
				file->decompressed_position = 0;
				file->decompressed_buffer_size = 0;
				free(file->decompressed_buffer);
				file->decompressed_buffer = NULL;
	      }
	  }


	if(!file->preload_size)
	{
		result = file->fileio.libqt_read(file->fileio.file,file->file_position,data, size);
	}
	else
	{
		__int64 selection_start = file->file_position;
		__int64 selection_end = file->file_position + size;
		__int64 fragment_start, fragment_len;

		if(selection_end - selection_start > file->preload_size)
		{
/* Size is larger than preload size.  Should never happen. */
//printf("read data 1\n");
			result = file->fileio.libqt_read(file->fileio.file,file->file_position,data, size);
		}
		else
		if(selection_start >= file->preload_start && 
			selection_start < file->preload_end &&
			selection_end <= file->preload_end &&
			selection_end > file->preload_start)
		{
/* Entire range is in buffer */
//printf("read data 2\n");
			oqt_read_preload(file, data, size);
		}
		else
		if(selection_end > file->preload_end && 
			selection_end - file->preload_size < file->preload_end)
		{
/* Range is after buffer */
/* Move the preload start to within one preload length of the selection_end */
//printf("read data 3\n");
			while(selection_end - file->preload_start > file->preload_size)
			{
				fragment_len = selection_end - file->preload_start - file->preload_size;
				if(file->preload_ptr + fragment_len > file->preload_size) 
					fragment_len = file->preload_size - file->preload_ptr;
				file->preload_start += fragment_len;
				file->preload_ptr += fragment_len;
				if(file->preload_ptr >= file->preload_size) file->preload_ptr = 0;
			}

/* Append sequential data after the preload end to the new end */
			fragment_start = file->preload_ptr + file->preload_end - file->preload_start;
			while(fragment_start >= file->preload_size) fragment_start -= file->preload_size;

			while(file->preload_end < selection_end)
			{
				fragment_len = selection_end - file->preload_end;
				if(fragment_start + fragment_len > file->preload_size) fragment_len = file->preload_size - fragment_start;
				result = file->fileio.libqt_read(file->fileio.file,file->preload_end,&(file->preload_buffer[fragment_start]), fragment_len);

				file->preload_end += fragment_len;
				fragment_start += fragment_len;
				if(fragment_start >= file->preload_size) fragment_start = 0;
			}

			oqt_read_preload(file, data, size);
		}
		else
		{
//printf("oqt_read_data 4 selection_start %lld selection_end %lld preload_start %lld\n", selection_start, selection_end, file->preload_start);
/* Range is before buffer or over a preload_size away from the end of the buffer. */
/* Replace entire preload buffer with range. */
			result = file->fileio.libqt_read(file->fileio.file,file->file_position,file->preload_buffer, size);
			file->preload_start = file->file_position;
			file->preload_end = file->file_position + size;
			file->preload_ptr = 0;
//printf("oqt_read_data 5\n");
			oqt_read_preload(file, data, size);
//printf("oqt_read_data 6\n");
		}
	}

//printf("oqt_read_data 1 %lld %lld\n", file->file_position, size);
	file->file_position += size;
	return result;
}


int oqt_write_data(oqt_t *file, char *data, int size)
{
	int result;
	
	if(size == 0) return 1;
	result = file->fileio.libqt_write(file->fileio.file,file->file_position,data,size);
	if(result)
		file->file_position += size;
	return result;
}


__int64 oqt_get_position(oqt_t *file)
{ 
  if(file->decompressed_buffer /*&& file->decompressed_buffer_size > file->decompressed_position*/)
    return file->decompressed_position;
	
  return file->file_position; 
}


int oqt_set_position(oqt_t *file, __int64 position)
{
//if(file->wr) printf("oqt_set_position 0x%llx\n", position);
  if(file->decompressed_buffer)
    file->decompressed_position = position;
  else
    file->file_position = position;

  return 0;
}


__int64 oqt_get_length(oqt_t *file)
{
	return file->fileio.libqt_flen(file->fileio.file);
}






/* Read entire buffer from the preload buffer */
int oqt_read_preload(oqt_t *file, char *data, __int64 size)
{
	__int64 selection_start = file->file_position;
	__int64 selection_end = file->file_position + size;
	__int64 fragment_start, fragment_len;

	fragment_start = file->preload_ptr + (selection_start - file->preload_start);
	while(fragment_start < 0) fragment_start += file->preload_size;
	while(fragment_start >= file->preload_size) fragment_start -= file->preload_size;

// gcc 2.96 fails here
	while(selection_start < selection_end)
	{
		fragment_len = selection_end - selection_start;
		if(fragment_start + fragment_len > file->preload_size)
			fragment_len = file->preload_size - fragment_start;

		memcpy(data, file->preload_buffer + fragment_start, (size_t)fragment_len);
		fragment_start += fragment_len;
		data = data + fragment_len;

		if(fragment_start >= file->preload_size) fragment_start = (__int64)0;
		selection_start += fragment_len;
	}
	return 0;
}



void oqt_read_pascal(oqt_t *file, char *data)
{
	int len = oqt_read_char(file);
	oqt_read_data(file, data, len);
	data[len] = 0;
}

void oqt_write_pascal(oqt_t *file, char *data)
{
	char len = (char)strlen(data);
	oqt_write_data(file, &len, 1);
	oqt_write_data(file, data, len);
}

float oqt_read_fixed32(oqt_t *file)
{
	ULONG a, b, c, d;
	BYTE  data[4];

	oqt_read_data(file, (char*)data, 4);
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];
	
	a = (a << 8) + b;
	b = (c << 8) + d;

	if(b)
		return (float)a + (float)b / 65536;
	else
		return (float)a;
}

int oqt_write_fixed32(oqt_t *file, float number)
{
	BYTE  data[4];
	int a, b;

	a = (int)number;
	b = (int)((number - a) * 65536);
	data[0] = a >> 8;
	data[1] = a & 0xff;
	data[2] = b >> 8;
	data[3] = b & 0xff;

	return oqt_write_data(file, (char*)data, 4);
}

int oqt_write_int64(oqt_t *file, __int64 value)
{
	BYTE  data[8];

#ifndef WIN32
	data[0] = (value & 0xff00000000000000LL) >> 56;
	data[1] = (value & 0xff000000000000LL) >> 48;
	data[2] = (value & 0xff0000000000LL) >> 40;
	data[3] = (value & 0xff00000000LL) >> 32;
	data[4] = (value & 0xff000000LL) >> 24;
	data[5] = (value & 0xff0000LL) >> 16;
	data[6] = (value & 0xff00LL) >> 8;
	data[7] = value & 0xff;
#else
	data[0] = (BYTE)((value & 0xff00000000000000) >> 56);
	data[1] = (BYTE)((value & 0xff000000000000) >> 48);
	data[2] = (BYTE)((value & 0xff0000000000) >> 40);
	data[3] = (BYTE)((value & 0xff00000000) >> 32);
	data[4] = (BYTE)((value & 0xff000000) >> 24);
	data[5] = (BYTE)((value & 0xff0000) >> 16);
	data[6] = (BYTE)((value & 0xff00) >> 8);
	data[7] = (BYTE)(value & 0xff);
#endif
	return oqt_write_data(file, (char*)data, 8);
}

int oqt_write_int32(oqt_t *file, long value)
{
	BYTE  data[4];

	data[0] = (BYTE)((value & 0xff000000) >> 24);
	data[1] = (BYTE)((value & 0xff0000) >> 16);
	data[2] = (BYTE)((value & 0xff00) >> 8);
	data[3] = (BYTE)(value & 0xff);

	return oqt_write_data(file, (char*)data, 4);
}

int oqt_write_le_int32(oqt_t *file, long value)
{
	BYTE  data[4];

	data[3] = (BYTE)((value & 0xff000000) >> 24);
	data[2] = (BYTE)((value & 0xff0000) >> 16);
	data[1] = (BYTE)((value & 0xff00) >> 8);
	data[0] = (BYTE)(value & 0xff);

	return oqt_write_data(file, (char*)data, 4);
}

/* Is this acutally right ? */
int oqt_write_uint32(oqt_t *file, ULONG value)
{
	BYTE  data[4];

	data[0] = (BYTE)((value & 0xff000000) >> 24);
	data[1] = (BYTE)((value & 0xff0000) >> 16);
	data[2] = (BYTE)((value & 0xff00) >> 8);
	data[3] = (BYTE)(value & 0xff);

	return oqt_write_data(file, (char*)data, 4);
}

int oqt_write_char32(oqt_t *file, char *string)
{
	return oqt_write_data(file, (char*)string, 4);
}


float oqt_read_fixed16(oqt_t *file)
{
	BYTE  data[2];
	
	oqt_read_data(file, (char*)data, 2);
//printf("oqt_read_fixed16 %02x%02x\n", data[0], data[1]);
	if(data[1])
		return (float)data[0] + (float)data[1] / 256;
	else
		return (float)data[0];
}

int oqt_write_fixed16(oqt_t *file, float number)
{
	BYTE  data[2];
	int a, b;

	a = (int)number;
	b = (int)((number - a) * 256);
	data[0] = a;
	data[1] = b;

	return oqt_write_data(file, (char*)data, 2);
}

ULONG oqt_read_uint32(oqt_t *file)
{
	ULONG result;
	ULONG a, b, c, d;
	char data[4];

	oqt_read_data(file, (char*)data, 4);
	a = (BYTE )data[0];
	b = (BYTE )data[1];
	c = (BYTE )data[2];
	d = (BYTE )data[3];

	result = (a << 24) | (b << 16) | (c << 8) | d;
	return result;
}

long oqt_read_int32(oqt_t *file)
{
	ULONG result;
	ULONG a, b, c, d;
	char data[4];

	oqt_read_data(file, (char*)data, 4);
	a = (BYTE )data[0];
	b = (BYTE )data[1];
	c = (BYTE )data[2];
	d = (BYTE )data[3];

	result = (a << 24) | (b << 16) | (c << 8) | d;
	return (long)result;
}

long oqt_read_le_int32(oqt_t *file)
{
	ULONG result;
	ULONG a, b, c, d;
	char data[4];

	oqt_read_data(file, (char*)data, 4);
	a = (BYTE )data[3];
	b = (BYTE )data[2];
	c = (BYTE )data[1];
	d = (BYTE )data[0];

	result = (a << 24) | (b << 16) | (c << 8) | d;
	return (long)result;
}

__int64 oqt_read_int64(oqt_t *file)
{
	__int64 result, a, b, c, d, e, f, g, h;
	char data[8];

	oqt_read_data(file, (char*)data, 8);
	a = (BYTE )data[0];
	b = (BYTE )data[1];
	c = (BYTE )data[2];
	d = (BYTE )data[3];
	e = (BYTE )data[4];
	f = (BYTE )data[5];
	g = (BYTE )data[6];
	h = (BYTE )data[7];

	result = (a << 56) | 
		(b << 48) | 
		(c << 40) | 
		(d << 32) | 
		(e << 24) | 
		(f << 16) | 
		(g << 8) | 
		h;
	return (__int64)result;
}


long oqt_read_int24(oqt_t *file)
{
	ULONG result;
	ULONG a, b, c;
	char data[4];
	
	oqt_read_data(file, (char*)data, 3);
	a = (BYTE )data[0];
	b = (BYTE )data[1];
	c = (BYTE )data[2];

	result = (a << 16) | (b << 8) | c;
	return (long)result;
}

int oqt_write_int24(oqt_t *file, long number)
{
	BYTE  data[3];
	data[0] = (BYTE)((number & 0xff0000) >> 16);
	data[1] = (BYTE)((number & 0xff00) >> 8);
	data[2] = (BYTE)((number & 0xff));
	
	return oqt_write_data(file, (char*)data, 3);
}

short oqt_read_int16(oqt_t *file)
{
	ULONG result;
	ULONG a, b;
	char data[2];
	
	oqt_read_data(file, (char*)data, 2);
	a = (BYTE )data[0];
	b = (BYTE )data[1];

	result = (a << 8) | b;
	return (short)result;
}

/* Read little endian integer from file
   (not very common) */
short oqt_read_le_int16(oqt_t *file)
{
	ULONG result;
	ULONG a, b;
	char data[2];
	
	oqt_read_data(file, (char*)data, 2);
	a = (BYTE )data[1];
	b = (BYTE )data[0];

	result = (a << 8) | b;
	return (short)result;
}


int oqt_write_int16(oqt_t *file, short number)
{
	BYTE  data[2];
	data[0] = (number & 0xff00) >> 8;
	data[1] = (number & 0xff);
	
	return oqt_write_data(file, (char*)data, 2);
}

int oqt_write_le_int16(oqt_t *file, short number)
{
	BYTE  data[2];
	data[1] = (number & 0xff00) >> 8;
	data[0] = (number & 0xff);
	
	return oqt_write_data(file, (char*)data, 2);
}


int oqt_read_char(oqt_t *file)
{
	char output;
	oqt_read_data(file, &output, 1);
	return output;
}

int oqt_write_char(oqt_t *file, char x)
{
	return oqt_write_data(file, &x, 1);
}

void oqt_read_char32(oqt_t *file, char *string)
{
	oqt_read_data(file, string, 4);
}

// access an external resource (file or url)
oqt_t * oqt_file_from_sd_id(oqt_t *file, oqt_trak_t *trak, int sd_id)
{
	int data_reference;
	oqt_stsd_t *stsd = &trak->mdia.minf.stbl.stsd;
	oqt_dref_t *dref = &trak->mdia.minf.dinf.dref;
	oqt_dref_table_t *entry;
	oqt_t *new_file;
	char *resource_name = NULL;
	// indexes in qt are 1-based
	if(sd_id < 1 || sd_id > stsd->total_entries) {
		fprintf(stderr, "oqt_file_from_sd_id: (sd_id) %d > %d (stsd.total_entries)\n", sd_id, stsd->total_entries);
		return NULL;
	}

	data_reference = stsd->table[sd_id-1].data_reference;

	if(data_reference < 1 || data_reference > dref->total_entries) {
		fprintf(stderr, "oqt_file_from_sd_id: (data_reference) %d > %d (dref.total_entries)\n", data_reference, dref->total_entries);
		return NULL;
	}

	// indexes in qt are 1-based
	entry = &dref->table[data_reference-1];
	if(entry->flags == 1) {
		// self reference (the most usual case)
		return file;
	}

	if(memcmp(entry->type, "url ", 4) == 0) {
		resource_name = entry->data_reference;
	} else if(memcmp(entry->type, "alis", 4) == 0) {
		// XXX this is only a guess!
		int len = entry->data_reference[0x32];
		resource_name = entry->data_reference + 0x33;
		resource_name[len] = 0;
	} else {
		fprintf(stderr, "oqt_file_from_sd_id: '%4s' data references unsupported\n", entry->type);
		return NULL;
	}
	return NULL;

	__asm int 3;
/*
	for(i = 0; i < file->ext_count; ++i) {
		if(strcmp(resource_name, file->ext_files[i]->stream_reference) == 0) {
			return file->ext_files[i];
		}
	}

	fprintf(stderr, "Accessing external resource '%s'\n", resource_name);
	new_file = oqt_open(resource_name);

	if(!new_file) {
		fprintf(stderr, "oqt_file_from_sd_id: failed to open reference '%s'\n", resource_name);
		return NULL;
	}

	if(new_file->stream_reference) {
		free(new_file->stream_reference);
	}

	new_file->stream_reference = strdup(resource_name);
*/
	file->ext_files = (oqt_t**)realloc(file->ext_files, (file->ext_count+1)*sizeof(oqt_t *));
	file->ext_files[file->ext_count] = new_file;
	file->ext_count += 1;
	return new_file;
}
