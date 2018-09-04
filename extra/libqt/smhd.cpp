/* smhd.c
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
 * $Id: smhd.c,v 1.4 2002/10/28 13:36:45 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"



void oqt_smhd_init(oqt_smhd_t *smhd)
{
	smhd->version = 0;
	smhd->flags = 0;
	smhd->balance = 0;
	smhd->reserved = 0;
}

void oqt_smhd_delete(oqt_smhd_t *smhd)
{
}

void oqt_smhd_dump(oqt_smhd_t *smhd)
{
	printf("    sound media header\n");
	printf("     version %d\n", smhd->version);
	printf("     flags %d\n", smhd->flags);
	printf("     balance %d\n", smhd->balance);
	printf("     reserved %d\n", smhd->reserved);
}

void oqt_read_smhd(oqt_t *file, oqt_smhd_t *smhd)
{
	smhd->version = oqt_read_char(file);
	smhd->flags = oqt_read_int24(file);
	smhd->balance = oqt_read_int16(file);
	smhd->reserved = oqt_read_int16(file);
}

void oqt_write_smhd(oqt_t *file, oqt_smhd_t *smhd)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "smhd");

	oqt_write_char(file, (char)smhd->version);
	oqt_write_int24(file, smhd->flags);
	oqt_write_int16(file, (short)smhd->balance);
	oqt_write_int16(file, (short)smhd->reserved);

	oqt_atom_write_footer(file, &atom);
}
