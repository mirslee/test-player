/* atom.c
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
 * $Id: atom.c,v 1.14 2003/04/07 21:02:17 shitowax Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


int oqt_atom_reset(oqt_atom_t *atom)
{
	atom->end = 0;
	atom->type[0] = atom->type[1] = atom->type[2] = atom->type[3] = 0;
	return 0;
}

int oqt_atom_read_type(char *data, char *type)
{
	type[0] = data[4];
	type[1] = data[5];
	type[2] = data[6];
	type[3] = data[7];

	//printf("%c%c%c%c ", type[0], type[1], type[2], type[3]); 
/* need this for oqt_check_sig */
	/* Yann : I change (int) into (unsigned char) cause .Net doesn't like negative number and is asserting all the time*/
	if(	isalpha(/*(int)*/(unsigned char)type[0]) && isalpha(/*(int)*/(unsigned char)type[1]) &&
		isalpha(/*(int)*/(unsigned char)type[2]) && isalpha(/*(int)*/(unsigned char)type[3]))
	return 0;
	else
	return 1;
}

int oqt_atom_read_header(oqt_t *file, oqt_atom_t *atom)
{
        char header[OQT_ATOM_HEAD_LENGTH];
	int result = 0;

	oqt_atom_reset(atom);

	atom->start = oqt_get_position(file);

	if(!oqt_read_data(file, header, OQT_ATOM_HEAD_LENGTH)) return 1;
	result = oqt_atom_read_type(header, (char*)atom->type);
	atom->size = oqt_atom_read_size(header);
	atom->end = atom->start + atom->size;
	atom->use_64 = 0;

	//  printf("oqt_atom_read_header 1 %c%c%c%c start 0x%llx size %lld end 0x%llx ftell 0x%llx 0x%llx\n", 
	// 	atom->type[0], atom->type[1], atom->type[2], atom->type[3],
	//  	atom->start, atom->size, atom->end,
	//  	oqt_get_position(file),
	//  	(__int64)file->oqt_flen(file));
	

/* Skip placeholder atom */
	if(oqt_match_32((char*)atom->type, "wide"))
	{
		atom->start = oqt_get_position(file);
		oqt_atom_reset(atom);
		if(!oqt_read_data(file, header, OQT_ATOM_HEAD_LENGTH)) return 1;
		result = oqt_atom_read_type(header, (char*)atom->type);
		atom->size -= 8;
		if(atom->size <= 0)
		{
/* Wrapper ended.  Get new atom size */
			atom->size = oqt_atom_read_size(header);
		}
		atom->end = atom->start + atom->size;
	}
	else
/* Get extended size */
	if(atom->size == 1)
	{
		if(!oqt_read_data(file, header, OQT_ATOM_HEAD_LENGTH)) return 1;
		atom->size = oqt_atom_read_size64(header);
		atom->end = atom->start + atom->size;
		atom->use_64 = 1;
/*
 * printf("oqt_atom_read_header 2 %c%c%c%c start 0x%llx size %lld end 0x%llx ftell 0x%llx\n", 
 * 	atom->type[0], atom->type[1], atom->type[2], atom->type[3],
 * 	atom->start, atom->size, atom->end,
 * 	oqt_get_position(file));
 */
	}


	return result;
}

int oqt_atom_write_header64(oqt_t *file, oqt_atom_t *atom, char *text)
{
	int result = 0;
	atom->start = oqt_get_position(file);

	result = !oqt_write_int32(file, 1);
	if(!result) result = !oqt_write_char32(file, text);
	if(!result) result = !oqt_write_int64(file, 0);

	atom->use_64 = 1;
	return result;
}

int oqt_atom_write_header(oqt_t *file, oqt_atom_t *atom, char *text)
{
	int result = 0;
	atom->type[0] = text[0]; atom->type[1] = text[1];
	atom->type[2] = text[2]; atom->type[3] = text[3];
	
	atom->start = oqt_get_position(file);
	result = !oqt_write_int32(file, 0);
	if(!result) result = !oqt_write_char32(file, text);
	atom->use_64 = 0;
	return result;
}

void oqt_atom_write_footer(oqt_t *file, oqt_atom_t *atom)
{
	atom->end = oqt_get_position(file);
	atom->size = atom->end - atom->start;
	if(atom->use_64)
	{
		oqt_set_position(file, atom->start + 8);
//printf("oqt_atom_write_footer 0x%llx 0x%llx 0x%llx 0x%llx\n", file->oqt_ftell(file), oqt_get_position(file), atom->start, atom->end);
		oqt_write_int64(file, atom->size);
	}
	else
	{
		oqt_set_position(file, atom->start);
		oqt_write_int32(file, (int)(atom->size));
	}
	
	//printf("%4.4s: start=$%8.8llx end=$%8.8llx len=$%8.8llx\n", &atom->type, atom->start, atom->end, atom->size);

	oqt_set_position(file, atom->end);
}

int oqt_atom_is(oqt_atom_t *atom, char *type)
{
	if(atom->type[0] == type[0] &&
		atom->type[1] == type[1] &&
		atom->type[2] == type[2] &&
		atom->type[3] == type[3])
	return 1;
	else
	return 0;
}

int oqt_atom_is_null(oqt_atom_t *atom)
{
	if(atom->type[0] == 0 &&
		atom->type[1] == 0 &&
		atom->type[2] == 0 &&
		atom->type[3] == 0)
	return 1;
	else
	return 0;
}

ULONG oqt_atom_read_size(char *data)
{
	ULONG result;
	ULONG a, b, c, d;
	
	a = (unsigned char)data[0];
	b = (unsigned char)data[1];
	c = (unsigned char)data[2];
	d = (unsigned char)data[3];

	result = (a << 24) | (b << 16) | (c << 8) | d;

// extended header is size 1
//	if(result < OQT_ATOM_HEAD_LENGTH) result = OQT_ATOM_HEAD_LENGTH;
	return result;
}

__int64 oqt_atom_read_size64(char *data)
{
	__int64 result, a, b, c, d, e, f, g, h;

	a = (unsigned char)data[0];
	b = (unsigned char)data[1];
	c = (unsigned char)data[2];
	d = (unsigned char)data[3];
	e = (unsigned char)data[4];
	f = (unsigned char)data[5];
	g = (unsigned char)data[6];
	h = (unsigned char)data[7];

	result = (a << 56) | 
		(b << 48) | 
		(c << 40) | 
		(d << 32) | 
		(e << 24) | 
		(f << 16) | 
		(g << 8) | 
		h;

	if(result < OQT_ATOM_HEAD_LENGTH) result = OQT_ATOM_HEAD_LENGTH;
	return (__int64)result;
}

int oqt_atom_skip(oqt_t *file, oqt_atom_t *atom)
{
	__int64 pos = atom->end;
	if(atom->start == atom->end) pos++;
	return oqt_set_position(file, pos);
}
