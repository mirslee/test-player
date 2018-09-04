/* udta.c
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
* $Id: udta.c,v 1.19 2003/04/07 21:02:27 shitowax Exp $
*/

#include "stdafx.h"
#include "qttype.h"
#include "libqt.h"
#include "qtfuncs.h"


#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "1.0"
#endif

#ifndef PACKAGE_STRING
# define PACKAGE_STRING "Jetsen Mov Demultiplexer 1.0"
#endif

#ifndef OQT_HOST
# define OQT_HOST "Jetsen Mov Demultiplexer"
#endif

#define OQT_DEFAULT_INFO "Jetsen Mov Demultiplexer v" PACKAGE_VERSION
#define OQT_INFO_NAME_COUNT		38

struct code_to_name_s {
	const char*	code;
	const char*	name;
};

static struct code_to_name_s code_to_name[OQT_INFO_NAME_COUNT] =
{
	{ OQT_INFO_COPYRIGHT, "Copyright" },
	{ OQT_INFO_NAME, "Name" },
	{ OQT_INFO_INFORMATION, "Information" },
	{ OQT_INFO_ARTIST, "Artist" },
	{ OQT_INFO_ALBUMN, "Albumn" },
	{ OQT_INFO_ENCODED_BY, "Encoded By" },
	{ OQT_INFO_TRACK, "Track" },
	{ OQT_INFO_CREATION_DATE, "Creation Date" },
	{ OQT_INFO_COMMENT, "Comment" },
	{ OQT_INFO_AUTHOR, "Author" },
	{ OQT_INFO_COMPOSER, "Composer" },
	{ OQT_INFO_DESCRIPTION, "Description" },
	{ OQT_INFO_DIRECTOR, "Director" },
	{ OQT_INFO_DISCLAIMER, "Disclaimer" },
	{ OQT_INFO_GENRE, "Genre" },
	{ OQT_INFO_HOST_COMPUTER, "Host Computer" },
	{ OQT_INFO_MAKE, "Make" },
	{ OQT_INFO_MODEL, "Model" },
	{ OQT_INFO_ORIG_ARTIST, "Original Artist" },
	{ OQT_INFO_ORIG_FORMAT, "Original Format" },
	{ OQT_INFO_ORIG_SOURCE, "Original Source" },
	{ OQT_INFO_PERFORMERS, "Performers" },
	{ OQT_INFO_PRODUCER, "Producer" },
	{ OQT_INFO_PRODUCT, "Product" },
	{ OQT_INFO_SOFTWARE, "Software" },
	{ OQT_INFO_SPECL_PLAY_REQ, "Special Playback Requirements" },
	{ OQT_INFO_WARNING, "Warning" },
	{ OQT_INFO_WRITER, "Writer" },
	{ OQT_INFO_URL_LINK, "URL Link" },
	{ OQT_INFO_EDIT_DATE_1, "Edit Date 1" },
	{ OQT_INFO_EDIT_DATE_2, "Edit Date 2" },
	{ OQT_INFO_EDIT_DATE_3, "Edit Date 3" },
	{ OQT_INFO_EDIT_DATE_4, "Edit Date 4" },
	{ OQT_INFO_EDIT_DATE_5, "Edit Date 5" },
	{ OQT_INFO_EDIT_DATE_6, "Edit Date 6" },
	{ OQT_INFO_EDIT_DATE_7, "Edit Date 7" },
	{ OQT_INFO_EDIT_DATE_8, "Edit Date 8" },
	{ OQT_INFO_EDIT_DATE_9, "Edit Date 9" }
};




int oqt_udta_init(oqt_moov_t *moov)
{
	moov->udta = NULL;
	moov->udta_count = 0;
	moov->window_x = 0;
	moov->window_y = 0;
	return 0;
}

int oqt_udta_delete(oqt_moov_t *moov)
{  
	int i;

	if(moov->udta)
	{
		for(i=0;i<moov->udta_count;++i) {
			if (moov->udta[i].value && moov->udta[i].value_len)
				free(moov->udta[i].value);
		}
		free(moov->udta);
	}

	oqt_udta_init(moov);

	return 0;
}

void oqt_udta_dump(oqt_moov_t *moov)
{
	int i;
	printf(" user data (udta)\n");

	for(i=0;i<moov->udta_count;++i) {
		const char *name = oqt_get_info_name(moov->udta[i].code);
		if (name) {
			printf("  %s -> ", name);
		} else {
			printf("  Unknown [%.4s] -> ", moov->udta[i].code);
		}

		if (moov->udta[i].value && moov->udta[i].value_len) {
			printf("%s\n", moov->udta[i].value);
		} else printf("NULL\n");
	}

	printf("  window loc -> %d,%d\n", moov->window_x, moov->window_y);
}

int oqt_set_udta_value(oqt_moov_t* moov, char* code, char *value, int size)
{
	int i, foundIt = 0;

	/* Does it already exist in the table ? */
	for(i=0;i<moov->udta_count;++i) {
		if (memcmp(moov->udta[i].code, code, 4)==0) {
			foundIt=1; break;
		}
	}

	/* Re-allocate table if not found */
	if (!foundIt) {
		i = moov->udta_count++;
		moov->udta = (oqt_udta_t*)realloc(moov->udta, (sizeof(oqt_udta_t)*moov->udta_count));
		memcpy(moov->udta[i].code, code, 4);
	} else {
		/* Free up the old memory */
		if (moov->udta[i].value && moov->udta[i].value_len) {
			free(moov->udta[i].value);
			moov->udta[i].value_len = 0;
		}
	}

	/* Set the new value */
	if (size>0) {
		moov->udta[i].value_len = size;
		moov->udta[i].value = (char*)malloc(size+1);
		memcpy(moov->udta[i].value, value, size);
		moov->udta[i].value[size] = 0;
	} else {
		moov->udta[i].value_len = 0;
		moov->udta[i].value = NULL;
	}

	return 0;
}

int oqt_read_udta(oqt_t *file, oqt_moov_t *moov, oqt_atom_t *udta_atom)
{
	oqt_atom_t leaf_atom;
	int result = 0;

	do
	{
		oqt_atom_read_header(file, &leaf_atom);

		/* If it starts with a '? then it (might) be a string... */
		if (leaf_atom.type[0] == '?') {
			char *string = NULL;
			int len = 0;

			result += oqt_read_udta_string(file, &string, &len);
			oqt_set_udta_value(moov, (char*)leaf_atom.type, string, len);

			free(string);
		}
		else if(oqt_atom_is(&leaf_atom, "WLOC"))
		{
			/* Location of the movie's window */
			moov->window_x = oqt_read_int16(file);
			moov->window_y = oqt_read_int16(file);
		}
		else
			oqt_atom_skip(file, &leaf_atom);

	} while(oqt_get_position(file) < udta_atom->end);

	return result;
}



void oqt_write_udta(oqt_t *file, oqt_moov_t *moov)
{
	int i;
	oqt_atom_t atom, subatom;

	oqt_atom_write_header(file, &atom, "udta");

#ifndef NO_GRATUTATOUS	
	/* Add Gratuatus Information field if none exists */
	if (!oqt_get_info_value(file, OQT_INFO_INFORMATION)) {
		oqt_set_info_value(file, OQT_INFO_INFORMATION, OQT_DEFAULT_INFO);
	}	  

	/* Set the Software and Host Computer infos if none exists */
	if(!oqt_get_info_value(file, OQT_INFO_SOFTWARE)) {
		oqt_set_info_value(file, OQT_INFO_SOFTWARE, PACKAGE_STRING);
	}
	if(!oqt_get_info_value(file, OQT_INFO_HOST_COMPUTER)) {
		oqt_set_info_value(file, OQT_INFO_HOST_COMPUTER, OQT_HOST);
	}
#endif

	for(i=0;i<moov->udta_count;++i) {
		if (moov->udta[i].value && moov->udta[i].value_len) {
			oqt_atom_write_header(file, &subatom, moov->udta[i].code);
			oqt_write_udta_string(file, moov->udta[i].value, moov->udta[i].value_len);
			oqt_atom_write_footer(file, &subatom);
		}
	}

	/* Write the Window Location */
	oqt_atom_write_header(file, &subatom, "WLOC");
	oqt_write_int16(file, moov->window_x);
	oqt_write_int16(file, moov->window_y);
	oqt_atom_write_footer(file, &subatom);

	oqt_atom_write_footer(file, &atom);
}


int oqt_read_udta_string(oqt_t *file, char **string, int *size)
{
	int result;

	//if(*size) free(*string);
	*size = oqt_read_int16(file);  /* Size of string */
	oqt_read_int16(file);  /* Discard language code */
	*string = (char*)realloc(*string, *size + 1);
	result = oqt_read_data(file, *string, *size);
	(*string)[*size] = 0;
	return !result;
}

int oqt_write_udta_string(oqt_t *file, char *string, int size)
{
	int new_size = (int)strlen(string);
	int result;

	oqt_write_int16(file, new_size);    /* String size */
	oqt_write_int16(file, 0);    /* Language code */
	result = oqt_write_data(file, string, new_size);
	return !result;
}



const char* oqt_get_info_value(oqt_t *file, char *code)
{
	int i;

	for(i=0;i<file->moov.udta_count;++i) {
		if (memcmp(file->moov.udta[i].code, code, 4)==0) {
			return (const char*)file->moov.udta[i].value;
		}
	}

	return NULL;
}


void
oqt_set_info_value(oqt_t *file, char *code, char *value)
{
	if (value==NULL) 
		oqt_set_udta_value(&(file->moov), code, NULL, 0);
	else	oqt_set_udta_value(&(file->moov), code, value,(int)strlen(value));
}


const char*
oqt_get_info_name(char *code)
{
	int i;

	for(i=0;i<OQT_INFO_NAME_COUNT;++i) {
		if (memcmp(code_to_name[i].code, code, 4)==0) {
			return (const char*)code_to_name[i].name;
		}
	}

	return NULL;
}


int
oqt_get_info_count(oqt_t *file)
{
	return file->moov.udta_count;
}


oqt_udta_t*
oqt_get_info_list(oqt_t *file)
{
	return file->moov.udta;
}


void
oqt_get_window_loc(oqt_t *file, short *x, short *y)
{
	*x = file->moov.window_x;
	*y = file->moov.window_y;
}

void
oqt_set_window_loc(oqt_t *file, short x, short y)
{
	file->moov.window_x = x;
	file->moov.window_y = y;
}
