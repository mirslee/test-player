/*******************************************************************************
 util.c

 libquicktime - A library for reading and writing quicktime/avi/mp4 files.
 http://libquicktime.sourceforge.net

 Copyright (C) 2002 Heroine Virtual Ltd.
 Copyright (C) 2002-2007 Members of the libquicktime project.

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 2.1 of the License, or (at your option)
 any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along
 with this library; if not, write to the Free Software Foundation, Inc., 51
 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*******************************************************************************/ 

#include "stdafx.h"
#include "lqt_private.h"
#include "workarounds.h"
#include <sys/stat.h>
#ifdef __GUNC__
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef HAVE_LRINT
#define lrint(x) ((int)(x))
#endif

/* Disk I/O */

int64_t quicktime_get_file_length(const char *path)
{
      struct stat status;
      if(stat(path, &status))
              perror("quicktime_get_file_length stat:");
      return status.st_size;
}

/*
int quicktime_file_open(quicktime_t *file, const char *path, int rd, int wr)
{
	int exists = 0;
	char flags[10];
	if(rd && (file->stream = fopen(path, "rb")))
	{
		exists = 1; 
		fclose(file->stream); 
	}

	if(rd && !wr) sprintf(flags, "rb");
	else
	if(!rd && wr) sprintf(flags, "wb");
	else
	if(rd && wr)
	{
		if(exists) 
			sprintf(flags, "rb+");
		else
			sprintf(flags, "wb+");
	}

	if(!(file->stream = fopen(path, flags)))
	{
		return 1;
	}

	if(rd && exists)
	{
		file->total_length = quicktime_get_file_length(path);		
	}
        if(wr)
          file->presave_buffer = (uint8_t*)calloc(1, QUICKTIME_PRESAVE);	
	return 0;
}

int quicktime_file_close(quicktime_t *file)
{
        if(file->presave_size)
        {
                quicktime_fseek(file, file->presave_position - file->presave_size);
                fwrite(file->presave_buffer, 1, file->presave_size, file->stream);
                file->presave_size = 0;
        }
 
        if(file->stream)
        {
                fclose(file->stream);
        }
        file->stream = 0;
        return 0;
}
*/

int64_t quicktime_ftell(quicktime_t *file)
{
	return file->ftell_position;
}

int quicktime_fseek(quicktime_t *file, int64_t offset)
{
	file->ftell_position = offset;
	if(offset > file->total_length || offset < 0) return 1;
	if(qt2fseek(file, file->ftell_position))
	{
//		perror("quicktime_fseek fseeko");
		return 1;
	}
	return 0;
}

/* Read entire buffer from the preload buffer */
static int read_preload(quicktime_t *file, uint8_t *data, int64_t size)
{
	int64_t selection_start = 0;
	int64_t selection_end = 0;
	int64_t fragment_start = 0;
	int64_t fragment_len = 0;

	selection_start = file->file_position;
	selection_end = quicktime_add(file->file_position, size);

	fragment_start = file->preload_ptr + (selection_start - file->preload_start);
	while(fragment_start < 0) fragment_start += file->preload_size;
	while(fragment_start >= file->preload_size) fragment_start -= file->preload_size;

	while(selection_start < selection_end)
	{
		fragment_len = selection_end - selection_start;
		if(fragment_start + fragment_len > file->preload_size)
			fragment_len = file->preload_size - fragment_start;

		memcpy(data, file->preload_buffer + fragment_start, (size_t)fragment_len);
		fragment_start += fragment_len;
		data += fragment_len;

		if(fragment_start >= file->preload_size) fragment_start = (int64_t)0;
		selection_start += fragment_len;
	}
	return 0;
}

int quicktime_read_data(quicktime_t *file, uint8_t *data, int64_t size)
  {
  int result = 1;
  int trytimes = 5;
  int reads = 0;
  // ĳЩ�����������ļ�,�ڶ�����ʱҪ���ȡ���ݵ�size < 0
  if (size < 0)
  {
	  return result = (int)size;
  }
  /* Return if we had an error before */
  if(file->io_error || file->io_eof)
    return 0;
  
  if(!file->preload_size)
  {
    result = qt2read(file,file->file_position,data,size);
    if(result < size)
      {
      file->io_error = 0;
      file->io_eof   = 1;
      }
		file->ftell_position += size;
  }
  else
  {
    /* Region requested for loading */
    int64_t selection_start = file->file_position;
    int64_t selection_end = file->file_position + size;
    int64_t fragment_start, fragment_len;

    if(selection_end - selection_start > file->preload_size)
      {
      /* Size is larger than preload size.  Should never happen. */
      result = qt2read(file,file->file_position,data,size);
      if(result < size)
        {
        file->io_error = 0;
        file->io_eof   = 1;
        }
      file->ftell_position += size;
      }
    else if(selection_start >= file->preload_start && 
            selection_start < file->preload_end &&
            selection_end <= file->preload_end &&
            selection_end > file->preload_start)
      {
      /* Entire range is in buffer */
      read_preload(file, data, size);
      result = (int)size;
      }
    else if(selection_end > file->preload_end && 
            selection_end - file->preload_size < file->preload_end)
      {
      /* Range is after buffer */
      /* Move the preload start to within one preload length of the selection_end */
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
      while(fragment_start >= file->preload_size) 
        fragment_start -= file->preload_size;
      
      while(file->preload_end < selection_end)
        {
        fragment_len = selection_end - file->preload_end;
        if(fragment_start + fragment_len > file->preload_size)
          fragment_len = file->preload_size - fragment_start;
        result = qt2read(file,file->preload_end,&(file->preload_buffer[fragment_start]),fragment_len);
        if(result < fragment_len)
          {
          file->io_error = 0;
          file->io_eof   = 1;
          }
        file->ftell_position += fragment_len;
        file->preload_end += fragment_len;
        fragment_start += fragment_len;
        if(fragment_start >= file->preload_size)
          fragment_start = 0;
        }
      
      read_preload(file, data, size);
      result = (int)size;
      }
    else
      {
      /* Range is before buffer or over a preload_size away from the end of the buffer. */
      /* Replace entire preload buffer with range. */
      result = qt2read(file,file->file_position,file->preload_buffer,size);
	  if(result > 0 && result < size)
	  {
		  do{
			  reads = qt2read(file,file->file_position+result, file->preload_buffer+result, size-result);
			  if(reads > 0) result += reads;
		  }while(trytimes-- > 0 && result < size);
	  }
      if(result < size)
        {
        file->io_error = 0;
        file->io_eof   = 1;
        }
      file->ftell_position += size;
      file->preload_start = file->file_position;
      file->preload_end = file->file_position + size;
      file->preload_ptr = 0;
      read_preload(file, data, size);
      

      }
    }
  file->file_position += size;
  return result;
}

int quicktime_write_data(quicktime_t *file, const uint8_t *data, int size)
{
  if(file->io_error)
    return 0;

	int result = qt2write(file,file->file_position,data,size);
	if(result<size)
	{
		file->io_error = 1;
		return 0;
	}
	file->file_position += size;
	return size;  
}

int quicktime_write_pad(quicktime_t *file,int align)
{
	__int64 walign = align - 1;
	int pad = ((file->file_position + walign)&~walign) - file->file_position;
	static uint8_t padbuf[4096] = { 0 };
	if (pad > 0)
		return quicktime_write_data(file, padbuf, pad);
	else
		return 0;
}

int64_t quicktime_byte_position(quicktime_t *file)
{
	return quicktime_position(file);
}


void quicktime_read_pascal(quicktime_t *file, char *data)
{
	char len = quicktime_read_char(file);
	quicktime_read_data(file, (uint8_t*)data, len);
	data[(int)len] = 0;
}

void quicktime_write_pascal(quicktime_t *file, char *data)
{
	uint8_t len = (uint8_t)strlen(data);
	quicktime_write_data(file, &len, 1);
	quicktime_write_data(file, (uint8_t*)data, len);
}

float quicktime_read_fixed32(quicktime_t *file)
{
	unsigned int a, b, c, d;
	uint8_t data[4];

	quicktime_read_data(file, data, 4);
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


int quicktime_write_fixed32(quicktime_t *file, float number)
{
	unsigned char data[4];
	int a, b;

	a = (int)number;
	b = (int)((number - a) * 65536);
	data[0] = a >> 8;
	data[1] = a & 0xff;
	data[2] = b >> 8;
	
	data[3] = b & 0xff;

	return quicktime_write_data(file, data, 4);
}

static float
float32_be_read (unsigned char *cptr)
{       int             exponent, mantissa, negative ;
        float   fvalue ;

        negative = cptr [0] & 0x80 ;
        exponent = ((cptr [0] & 0x7F) << 1) | ((cptr [1] & 0x80) ? 1 : 0) ;
        mantissa = ((cptr [1] & 0x7F) << 16) | (cptr [2] << 8) | (cptr [3]) ;

        if (! (exponent || mantissa))
                return 0.0 ;

        mantissa |= 0x800000 ;
        exponent = exponent ? exponent - 127 : 0 ;

        fvalue = mantissa ? ((float) mantissa) / ((float) 0x800000) : 0.f ;

        if (negative)
                fvalue *= -1 ;

        if (exponent > 0)
                fvalue *= (1 << exponent) ;
        else if (exponent < 0)
                fvalue /= (1 << abs (exponent)) ;

        return fvalue ;
} /* float32_be_read */

static double
double64_be_read (unsigned char *cptr)
{       int             exponent, negative ;
        double  dvalue ;

        negative = (cptr [0] & 0x80) ? 1 : 0 ;
        exponent = ((cptr [0] & 0x7F) << 4) | ((cptr [1] >> 4) & 0xF) ;

        /* Might not have a 64 bit int, so load the mantissa into a double. */
        dvalue = (((cptr [1] & 0xF) << 24) | (cptr [2] << 16) | (cptr [3] << 8) | cptr [4]) ;
        dvalue += ((cptr [5] << 16) | (cptr [6] << 8) | cptr [7]) / ((double) 0x1000000) ;

        if (exponent == 0 && dvalue == 0.0)
                return 0.0 ;

        dvalue += 0x10000000 ;

        exponent = exponent - 0x3FF ;

        dvalue = dvalue / ((double) 0x10000000) ;

        if (negative)
                dvalue *= -1 ;

        if (exponent > 0)
                dvalue *= (1 << exponent) ;
        else if (exponent < 0)
                dvalue /= (1 << abs (exponent)) ;

        return dvalue ;
} /* double64_be_read */

static void
float32_be_write (float in, unsigned char *out)
{       int             exponent, mantissa, negative = 0 ;

        memset (out, 0, sizeof (int)) ;

        if (in == 0.0)
                return ;

        if (in < 0.0)
        {       in *= -1.0 ;
                negative = 1 ;
                } ;

        in = frexp (in, &exponent) ;

        exponent += 126 ;

        in *= (float) 0x1000000 ;
        mantissa = (((int) in) & 0x7FFFFF) ;

        if (negative)
                out [0] |= 0x80 ;

        if (exponent & 0x01)
                out [1] |= 0x80 ;

        out [3] = mantissa & 0xFF ;
        out [2] = (mantissa >> 8) & 0xFF ;
        out [1] |= (mantissa >> 16) & 0x7F ;
        out [0] |= (exponent >> 1) & 0x7F ;

        return ;
} /* float32_be_write */


static void
double64_be_write (double in, unsigned char *out)
{       int             exponent, mantissa ;

        memset (out, 0, sizeof (double)) ;

        if (in == 0.0)
                return ;

        if (in < 0.0)
        {       in *= -1.0 ;
                out [0] |= 0x80 ;
                } ;

        in = frexp (in, &exponent) ;

        exponent += 1022 ;

        out [0] |= (exponent >> 4) & 0x7F ;
        out [1] |= (exponent << 4) & 0xF0 ;

        in *= 0x20000000 ;
        mantissa = lrint (floor (in)) ;

        out [1] |= (mantissa >> 24) & 0xF ;
        out [2] = (mantissa >> 16) & 0xFF ;
        out [3] = (mantissa >> 8) & 0xFF ;
        out [4] = mantissa & 0xFF ;

        in = fmod (in, 1.0) ;
        in *= 0x1000000 ;
        mantissa = lrint (floor (in)) ;

        out [5] = (mantissa >> 16) & 0xFF ;
        out [6] = (mantissa >> 8) & 0xFF ;
        out [7] = mantissa & 0xFF ;

        return ;
} /* double64_be_write */


float quicktime_read_float32(quicktime_t *file)
  {
  unsigned char b[4];
  quicktime_read_data(file, b, 4);
  return float32_be_read(b);
  }

double quicktime_read_double64(quicktime_t *file)
  {
  unsigned char b[8];
  quicktime_read_data(file, b, 8);
  return double64_be_read(b);
  }


int quicktime_write_float32(quicktime_t *file, float value)
  {
  unsigned char b[4];
  float32_be_write(value, b);
  return quicktime_write_data(file, b, 4);
  }

int quicktime_write_double64(quicktime_t *file, double value)
  {
  unsigned char b[8];
  double64_be_write(value, b);
  return quicktime_write_data(file, b, 8);
  }

int quicktime_write_int64(quicktime_t *file, int64_t value)
{
	unsigned char data[8];

	data[0] = (unsigned char)((((uint64_t)value) & 0xff00000000000000LL) >> 56);
	data[1] = (unsigned char)((((uint64_t)value) & 0xff000000000000LL) >> 48);
	data[2] = (unsigned char)((((uint64_t)value) & 0xff0000000000LL) >> 40);
	data[3] = (unsigned char)((((uint64_t)value) & 0xff00000000LL) >> 32);
	data[4] = (unsigned char)((((uint64_t)value) & 0xff000000LL) >> 24);
	data[5] = (unsigned char)((((uint64_t)value) & 0xff0000LL) >> 16);
	data[6] = (unsigned char)((((uint64_t)value) & 0xff00LL) >> 8);
	data[7] =  (unsigned char)(((uint64_t)value) & 0xff);

	return quicktime_write_data(file, data, 8);
}

int quicktime_write_int64_le(quicktime_t *file, int64_t value)
{
        unsigned char data[8];
 
        data[7] = (unsigned char)((((uint64_t)value) & 0xff00000000000000LL) >> 56);
        data[6] = (unsigned char)((((uint64_t)value) & 0xff000000000000LL) >> 48);
        data[5] = (unsigned char)((((uint64_t)value) & 0xff0000000000LL) >> 40);
        data[4] = (unsigned char)((((uint64_t)value) & 0xff00000000LL) >> 32);
        data[3] = (unsigned char)((((uint64_t)value) & 0xff000000LL) >> 24);
        data[2] = (unsigned char)((((uint64_t)value) & 0xff0000LL) >> 16);
        data[1] = (unsigned char)((((uint64_t)value) & 0xff00LL) >> 8);
        data[0] = (unsigned char)( ((uint64_t)value) & 0xff);
                                                                                                                  
        return quicktime_write_data(file, data, 8);
}


int quicktime_write_int32(quicktime_t *file, int value)
{
	unsigned char data[4];

	data[0] = (unsigned char)((value & 0xff000000) >> 24);
	data[1] = (unsigned char)((value & 0xff0000) >> 16);
	data[2] = (unsigned char)((value & 0xff00) >> 8);
	data[3] = (unsigned char)(value & 0xff);

	return quicktime_write_data(file, data, 4);
}

int quicktime_write_int32_le(quicktime_t *file, int value)
{
	unsigned char data[4];

	data[3] = (unsigned char)((value & 0xff000000) >> 24);
	data[2] = (unsigned char)((value & 0xff0000) >> 16);
	data[1] = (unsigned char)((value & 0xff00) >> 8);
	data[0] = (unsigned char) (value & 0xff);

	return quicktime_write_data(file, data, 4);
}

int quicktime_write_char32(quicktime_t *file, const char *string)
{
return quicktime_write_data(file, (uint8_t*)string, 4);
}


float quicktime_read_fixed16(quicktime_t *file)
{
	unsigned char data[2];
	
	quicktime_read_data(file, data, 2);
	if(data[1])
		return (float)data[0] + (float)data[1] / 256;
	else
		return (float)data[0];
}

int quicktime_write_fixed16(quicktime_t *file, float number)
{
	unsigned char data[2];
	int a, b;

	a = (int)number;
	b = (int)((number - a) * 256);
	data[0] = a;
	data[1] = b;

	return quicktime_write_data(file, data, 2);
}

unsigned int quicktime_read_uint32(quicktime_t *file)
{
	unsigned int result;
	unsigned int a, b, c, d;
	uint8_t data[4];

	quicktime_read_data(file, data, 4);
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];

	result = (a << 24) | (b << 16) | (c << 8) | d;
	return result;
}

int quicktime_read_int32(quicktime_t *file)
{
	unsigned int result;
	unsigned int a, b, c, d;
	uint8_t data[4];

	quicktime_read_data(file, data, 4);
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];

	result = (a << 24) | (b << 16) | (c << 8) | d;
	return (int)result;
}

int quicktime_read_int32_le(quicktime_t *file)
{
	unsigned int result;
	unsigned int a, b, c, d;
	uint8_t data[4];

	quicktime_read_data(file, data, 4);
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];

	result = (d << 24) | (c << 16) | (b << 8) | a;
	return (int)result;
}

int64_t quicktime_read_int64(quicktime_t *file)
{
	uint64_t result, a, b, c, d, e, f, g, h;
	uint8_t data[8];

	quicktime_read_data(file, data, 8);
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];
	e = data[4];
	f = data[5];
	g = data[6];
	h = data[7];

	result = (a << 56) | 
		(b << 48) | 
		(c << 40) | 
		(d << 32) | 
		(e << 24) | 
		(f << 16) | 
		(g << 8) | 
		h;
	return (int64_t)result;
}


int64_t quicktime_read_int64_le(quicktime_t *file)
{
        uint64_t result, a, b, c, d, e, f, g, h;
        uint8_t data[8];
                                                                                                                  
        quicktime_read_data(file, data, 8);
        a = data[7];
        b = data[6];
        c = data[5];
        d = data[4];
        e = data[3];
        f = data[2];
        g = data[1];
        h = data[0];
                                                                                                                  
        result = (a << 56) |
                (b << 48) |
                (c << 40) |
                (d << 32) |
                (e << 24) |
                (f << 16) |
                (g << 8) |
                h;
        return (int64_t)result;
}


int quicktime_read_int24(quicktime_t *file)
{
	unsigned int result;
	unsigned int a, b, c;
        uint8_t data[4];
	
	quicktime_read_data(file, data, 3);
	a = data[0];
	b = data[1];
	c = data[2];

	result = (a << 16) | (b << 8) | c;
	return (int)result;
}

int quicktime_write_int24(quicktime_t *file, int number)
{
	unsigned char data[3];
	data[0] = (unsigned char)((number & 0xff0000) >> 16);
	data[1] = (unsigned char)((number & 0xff00) >> 8);
	data[2] = (unsigned char)( number & 0xff);
	
	return quicktime_write_data(file, data, 3);
}

int quicktime_read_int16(quicktime_t *file)
{
	unsigned int result;
	unsigned int a, b;
        uint8_t data[2];
	
	quicktime_read_data(file, data, 2);
	a = data[0];
	b = data[1];

	result = (a << 8) | b;
	return (int)result;
}

int quicktime_read_int16_le(quicktime_t *file)
{
	unsigned int result;
	unsigned int a, b;
	uint8_t data[2];
	
	quicktime_read_data(file, data, 2);
	a = data[0];
	b = data[1];

	result = (b << 8) | a;
	return (int)result;
}

int quicktime_write_int16(quicktime_t *file, int number)
{
	unsigned char data[2];
	data[0] = (number & 0xff00) >> 8;
	data[1] = (number & 0xff);
	
	return quicktime_write_data(file, data, 2);
}

int quicktime_write_int16_le(quicktime_t *file, int number)
{
	unsigned char data[2];
	data[1] = (number & 0xff00) >> 8;
	data[0] = (number & 0xff);
	
	return quicktime_write_data(file, data, 2);
}

int quicktime_read_char(quicktime_t *file)
{
	char output;
	quicktime_read_data(file, (uint8_t*)(&output), 1);
	return output;
}

int quicktime_write_char(quicktime_t *file, char x)
{
return quicktime_write_data(file, (uint8_t*)(&x), 1);
}

void quicktime_read_char32(quicktime_t *file, char *string)
{
quicktime_read_data(file, (uint8_t*)string, 4);
}

int64_t quicktime_position(quicktime_t *file) 
{ 
	return file->file_position; 
}

int quicktime_set_position(quicktime_t *file, int64_t position) 
{
	file->file_position = position;
	return 0;
}

void quicktime_copy_char32(char *output, char *input)
{
	*output++ = *input++;
	*output++ = *input++;
	*output++ = *input++;
	*output = *input;
}


void quicktime_print_chars(const char *desc, uint8_t *input, int len)
{
	int i;
	lqt_dump("%s", desc);
	for(i = 0; i < len; i++) lqt_dump("%02x ", input[i]);
	lqt_dump("\n");
}

unsigned int quicktime_current_time(void)
{
	time_t t;
	time (&t);
	return (unsigned int)(t+(66*31536000ll)+1468800);
}

int quicktime_match_32(void *_input, const void *_output)
{
        uint8_t * input = (uint8_t*)_input;
        uint8_t * output = (uint8_t*)_output;
	if(input[0] == output[0] &&
		input[1] == output[1] &&
		input[2] == output[2] &&
		input[3] == output[3])
		return 1;
	else 
		return 0;
}

int quicktime_match_24(char *input, char *output)
{
	if(input[0] == output[0] &&
		input[1] == output[1] &&
		input[2] == output[2])
		return 1;
	else 
		return 0;
}

static void do_hexdump(uint8_t * data, int len, int linebreak, FILE * f)
  {
  int i;
  int bytes_written = 0;
  int imax;

  while(bytes_written < len)
    {
    imax = (bytes_written + linebreak > len) ? len - bytes_written : linebreak;
    for(i = 0; i < imax; i++)
      fprintf(f, "%02x ", data[bytes_written + i]);
    for(i = imax; i < linebreak; i++)
      fprintf(f, "   ");
    for(i = 0; i < imax; i++)
      {
      if(!(data[bytes_written + i] & 0x80) && (data[bytes_written + i] >= 32))
        fprintf(f, "%c", data[bytes_written + i]);
      else
        fprintf(f, ".");
      }
    bytes_written += imax;
    fprintf(f, "\n");
    }
  }

void lqt_hexdump(uint8_t * data, int len, int linebreak)
  {
  do_hexdump(data, len, linebreak, stderr);  
  }

void lqt_hexdump_stdout(uint8_t * data, int len, int linebreak)
  {
  do_hexdump(data, len, linebreak, stdout);  
  }

#define localtime_r( _clock, _result ) \
	( *(_result) = *localtime( (_clock) ), \
	(_result) )


void lqt_dump_time(uint64_t t)
{
  time_t ti = 0;
  struct tm mytm;
  /*  2082844800 = seconds between 1/1/04 and 1/1/70 */
  ti = t - 2082844800ll;
  struct tm* ltm = localtime(&ti);
  if(ltm)
  {
	  mytm = *ltm;
	  mytm.tm_mon++;
	  mytm.tm_mday++;
	  lqt_dump("%04d-%02d-%02d %02d:%02d:%02d (%"PRId64")",
			 mytm.tm_year+1900, mytm.tm_mon, mytm.tm_mday,
			 mytm.tm_hour, mytm.tm_min, mytm.tm_sec, t);

  }
}
