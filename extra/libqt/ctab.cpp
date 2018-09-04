/* ctab.c
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
 * $Id: ctab.c,v 1.7 2002/12/15 01:51:58 nj_humfrey Exp $
 */

#include <stdio.h>

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"
#include "ctab.h"


int oqt_ctab_init(oqt_ctab_t *ctab)
{
	ctab->seed = 0;
	ctab->flags = 0x8000;
	ctab->size = 0;
	
	ctab->alpha = NULL;
	ctab->red = NULL;
	ctab->green = NULL;
	ctab->blue = NULL;
	return 0;
}

int oqt_ctab_delete(oqt_ctab_t *ctab)
{
	if(ctab->alpha) free(ctab->alpha);
	if(ctab->red) free(ctab->red);
	if(ctab->green) free(ctab->green);
	if(ctab->blue) free(ctab->blue);
	return 0;
}

void oqt_ctab_dump(oqt_ctab_t *ctab)
{
	long i;
	
	if (ctab) {
		printf(" color table\n");
		printf("  seed %d\n", ctab->seed);
		printf("  flags %x\n", ctab->flags);
		printf("  size %d\n", ctab->size);
		
		if ( ctab->size) {
			printf("  colors ");
			for(i = 0; i < ctab->size; i++)
			{
				printf("[%d %d %d %d]", (int)ctab->red[i], (int)ctab->green[i], (int)ctab->blue[i], (int)ctab->alpha[i]);
			}
			printf("\n");
		}
	} else {
		printf("color table is null.\n");
	}
}

static int ctab_allocate_tables(oqt_ctab_t *ctab)
{
	if (ctab->alpha) free(ctab->alpha);
	if (ctab->red) free(ctab->alpha);
	if (ctab->green) free(ctab->alpha);
	if (ctab->blue) free(ctab->alpha);

	ctab->alpha = (short*)malloc(sizeof(short) * ctab->size);
	ctab->red = (short*)malloc(sizeof(short) * ctab->size);
	ctab->green = (short*)malloc(sizeof(short) * ctab->size);
	ctab->blue = (short*)malloc(sizeof(short) * ctab->size);
	
	return 0;
}

int oqt_read_ctab(oqt_t *file, oqt_ctab_t *ctab)
{
	long i;
	
	ctab->seed = oqt_read_int32(file);		// Should be 0
	ctab->flags = oqt_read_int16(file);		// Should be 0x8000
	ctab->size = oqt_read_int16(file) + 1;
	
	ctab_allocate_tables(ctab);
	
	for(i = 0; i < ctab->size; i++)
	{
		ctab->alpha[i] = oqt_read_int16(file);
		ctab->red[i] = oqt_read_int16(file);
		ctab->green[i] = oqt_read_int16(file);
		ctab->blue[i] = oqt_read_int16(file);
	}

	return 0;
}

int oqt_ctab_default(oqt_ctab_t *ctab, int ctab_id)
{
	WORD *src = NULL;
	int i;

	if (ctab_id==1) {
		/* Black and White */
		ctab->size = 2;
		src = oqt_default_ctab_1;
	} else if (ctab_id==2 || ctab_id==34) {
		/* 2bit colour is same as 2bit gray */
		ctab->size = 4;
		src = oqt_default_ctab_2;
	} else if (ctab_id==36) {
		/* 16 Grays */
		ctab->size = 16;
		src = oqt_default_ctab_36;
	} else if (ctab_id==4) {
		/* 16 Colours */
		ctab->size = 16;
		src = oqt_default_ctab_4;
	} else if (ctab_id==40) {
		/* 256 Grays */
		ctab->size = 256;
		src = oqt_default_ctab_40;
	} else if (ctab_id==8) {
		/* 256 Colours */
		ctab->size = 256;
		src = oqt_default_ctab_8;
	}
	
	if (src) {
		// Allocate memory for the table
		ctab_allocate_tables(ctab);
		
		for (i=0; i<ctab->size; i++) {
			ctab->red[i]	= src[(i*4)+0];
			ctab->green[i]	= src[(i*4)+1];
			ctab->blue[i]	= src[(i*4)+2];
			ctab->alpha[i]	= src[(i*4)+3];
		}
	} else {
		fprintf(stderr, "oqt_ctab_default: error, unknown ctab ID %d.\n", ctab_id);
		fprintf(stderr, "Please report this so that the error can be fixed.\n");
	}
	
	return 0;
}


/* *** Untested *** */
int oqt_write_ctab(oqt_t *file, oqt_ctab_t *ctab)
{	
	oqt_atom_t atom;
	long i;

	// Don't bother if size is zero
	if (ctab->size==0) {
		fprintf(stderr, "oqt_write_ctab: aborting - table size is zero.\n");
		return 0;
	}

	oqt_atom_write_header(file, &atom, "ctab");
	oqt_write_int32(file, ctab->seed);
	oqt_write_int16(file, (short)ctab->flags);
	oqt_write_int16(file, (short)ctab->size-1);

	for(i = 0; i < ctab->size; i++)
	{
		oqt_write_int16(file, ctab->alpha[i]);
		oqt_write_int16(file, ctab->red[i]);
		oqt_write_int16(file, ctab->green[i]);
		oqt_write_int16(file, ctab->blue[i]);
	}
	
	oqt_atom_write_footer(file, &atom);
	
	return 0;
}

