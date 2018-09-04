/* dinf.c
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
 * $Id: dinf.c,v 1.4 2002/10/28 13:36:44 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_dinf_init(oqt_dinf_t *dinf)
{
	oqt_dref_init(&(dinf->dref));
}

void oqt_dinf_delete(oqt_dinf_t *dinf)
{
	oqt_dref_delete(&(dinf->dref));
}

void oqt_dinf_init_all(oqt_dinf_t *dinf)
{
	oqt_dref_init_all(&(dinf->dref));
}

void oqt_dinf_dump(oqt_dinf_t *dinf)
{
	printf("    data information (dinf)\n");
	oqt_dref_dump(&(dinf->dref));
}

void oqt_read_dinf(oqt_t *file, oqt_dinf_t *dinf, oqt_atom_t *dinf_atom)
{
	oqt_atom_t leaf_atom;

	do
	{
		oqt_atom_read_header(file, &leaf_atom);
		if(oqt_atom_is(&leaf_atom, "dref"))
			{ oqt_read_dref(file, &(dinf->dref)); }
		else
			oqt_atom_skip(file, &leaf_atom);
	}while(oqt_get_position(file) < dinf_atom->end);
}

void oqt_write_dinf(oqt_t *file, oqt_dinf_t *dinf)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "dinf");
	oqt_write_dref(file, &(dinf->dref));
	oqt_atom_write_footer(file, &atom);
}
