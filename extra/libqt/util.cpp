/* util.c
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
 * $Id: util.c,v 1.11 2003/02/08 04:20:21 jhatala Exp $
 */

#include <time.h>
#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"



void oqt_copy_char32(char *output, char *input)
{
	*output++ = *input++;
	*output++ = *input++;
	*output++ = *input++;
	*output = *input;
}


void oqt_print_chars(char *desc, char *input, int len)
{
	int i;
	printf("%s", desc);
	for(i = 0; i < len; i++) printf("%c", input[i]);
	printf("\n");
}


ULONG oqt_current_time(void)
{
	time_t t;
	time (&t);
	return (ULONG)(t+(66*31536000)+1468800);
}


int oqt_match_32(const char *input, const char *output)
{
	if(*(DWORD*)input==*(DWORD*)output)
		return 1;
	else 
		return 0;
}


int oqt_read_mp4_descr_length(oqt_t *file)
{
	char b;
	unsigned char numBytes = 0;
	unsigned int length = 0;

	do {
		b = oqt_read_char(file);
		numBytes++;
		length = (length << 7) | (b & 0x7F);
	} while ((b & 0x80) && numBytes < 4);

	return length;
}

int oqt_write_mp4_descr_length(oqt_t *file, int length, int compact)
{
	BYTE b;
	int i,numBytes;

	if (compact) {
		if (length <= 0x7F) {
			numBytes = 1;
		} else if (length <= 0x3FFF) {
			numBytes = 2;
		} else if (length <= 0x1FFFFF) {
			numBytes = 3;
		} else {
			numBytes = 4;
		}
	} else {
		numBytes = 4;
	}

	for (i = numBytes-1; i >= 0; i--) {
		b = (length >> (i * 7)) & 0x7F;
		if (i != 0) {
			b |= 0x80;
		}
		oqt_write_char(file, b);
	}

	return numBytes; 
}


int oqt_get_timescale(float frame_rate)
{
	int timescale = 600;
/* Encode the 29.97, 23.976, 59.94 framerates */
	if(frame_rate - (int)frame_rate != 0) 
		timescale = (int)(frame_rate * 1001 + 0.5);
	else
	if((600 / frame_rate) - (int)(600 / frame_rate) != 0) 
			timescale = (int)(frame_rate * 100 + 0.5);
//printf("oqt_get_timescale %f %d\n", 600.0 / (double)frame_rate, (int)(600.0 / frame_rate));
	return timescale;
}

void oqt_hexdump(const BYTE *data, int size)
{
	int i, j;

	for(j = 0; j < (size+15)/16; ++j) {
		for(i = 0; i < 16; ++i) {
			if(16*j+i < size) {
				printf(" %02x", (int)data[16*j+i]);
			} else {
				printf("   ");
			}
		}
		printf("        ");
		for(i = 0; i < 16; ++i) {
			if(16*j+i < size) {
				unsigned char c = data[16*j+i];
				if(c >= 32 && c < 127) {
					printf("%c", c);
				} else {
					printf(".");
				}
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
}
