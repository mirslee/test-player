/* vmhd.c
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
 * $Id: vmhd.c,v 1.4 2002/10/28 13:36:45 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_vmhd_init(oqt_vmhd_t *vmhd)
{
	vmhd->version = 0;
	vmhd->flags = 1;
	vmhd->graphics_mode = 64;
	vmhd->opcolor[0] = 32768;
	vmhd->opcolor[1] = 32768;
	vmhd->opcolor[2] = 32768;
}

void oqt_vmhd_init_video(oqt_t *file, 
								oqt_vmhd_t *vmhd, 
								int frame_w,
								int frame_h, 
								float frame_rate)
{
}

void oqt_vmhd_delete(oqt_vmhd_t *vmhd)
{
}

void oqt_vmhd_dump(oqt_vmhd_t *vmhd)
{
	printf("    video media header\n");
	printf("     version %d\n", vmhd->version);
	printf("     flags %d\n", vmhd->flags);
	printf("     graphics_mode %d\n", vmhd->graphics_mode);
	printf("     opcolor %d %d %d\n", vmhd->opcolor[0], vmhd->opcolor[1], vmhd->opcolor[2]);
}

void oqt_read_vmhd(oqt_t *file, oqt_vmhd_t *vmhd)
{
	int i;
	vmhd->version = oqt_read_char(file);
	vmhd->flags = oqt_read_int24(file);
	vmhd->graphics_mode = oqt_read_int16(file);
	for(i = 0; i < 3; i++)
		vmhd->opcolor[i] = oqt_read_int16(file);
}

void oqt_write_vmhd(oqt_t *file, oqt_vmhd_t *vmhd)
{
	oqt_atom_t atom;
	int i;
	oqt_atom_write_header(file, &atom, "vmhd");

	oqt_write_char(file, (char)vmhd->version);
	oqt_write_int24(file, vmhd->flags);
	oqt_write_int16(file, (short)vmhd->graphics_mode);
	
	for(i = 0; i < 3; i++)
		oqt_write_int16(file, (short)vmhd->opcolor[i]);

	oqt_atom_write_footer(file, &atom);
}

