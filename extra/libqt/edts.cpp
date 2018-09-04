/* edts.c
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
 * $Id: edts.c,v 1.4 2002/10/28 13:36:44 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_edts_init(oqt_edts_t *edts)
{
	oqt_elst_init(&(edts->elst));
}

void oqt_edts_delete(oqt_edts_t *edts)
{
	oqt_elst_delete(&(edts->elst));
}

void oqt_edts_init_table(oqt_edts_t *edts)
{
	oqt_elst_init_all(&(edts->elst));
}

void oqt_read_edts(oqt_t *file, oqt_edts_t *edts, oqt_atom_t *edts_atom)
{
	oqt_atom_t leaf_atom;

	do
	{
		oqt_atom_read_header(file, &leaf_atom);
//printf("oqt_read_edts %llx %llx\n", oqt_get_position(file), leaf_atom.end);
		if(oqt_atom_is(&leaf_atom, "elst"))
		{ oqt_read_elst(file, &(edts->elst)); }
		else
			oqt_atom_skip(file, &leaf_atom);
	}while(oqt_get_position(file) < edts_atom->end);
}

void oqt_edts_dump(oqt_edts_t *edts)
{
	printf("  edit atom (edts)\n");
	oqt_elst_dump(&(edts->elst));
}

void oqt_write_edts(oqt_t *file, oqt_edts_t *edts, long duration)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "edts");
	oqt_write_elst(file, &(edts->elst), duration);
	oqt_atom_write_footer(file, &atom);
}
