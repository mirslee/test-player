/* mdat.c
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
 * $Id: mdat.c,v 1.4 2002/10/28 13:36:45 nj_humfrey Exp $
 */

#include "stdafx.h"

#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


void oqt_mdat_init(oqt_mdat_t *mdat)
{
	mdat->atom.size = 0;
	mdat->atom.start = 0;
}

void oqt_mdat_delete(oqt_mdat_t *mdat)
{
}

void oqt_read_mdat(oqt_t *file, oqt_mdat_t *mdat, oqt_atom_t *parent_atom)
{
	mdat->atom.size = parent_atom->size;
	mdat->atom.start = parent_atom->start;
	oqt_atom_skip(file, parent_atom);
}

void oqt_write_mdat(oqt_t *file, oqt_mdat_t *mdat)
{

}
