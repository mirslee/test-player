/* matrix.c
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
 * $Id: matrix.c,v 1.3 2002/10/28 13:36:45 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"

void oqt_matrix_init(oqt_matrix_t *matrix)
{
	int i;
	for(i = 0; i < 9; i++) matrix->values[i] = 0;
	matrix->values[0] = matrix->values[4] = 1;
	matrix->values[8] = 16384;
}

void oqt_matrix_delete(oqt_matrix_t *matrix)
{
}

void oqt_read_matrix(oqt_t *file, oqt_matrix_t *matrix)
{
	int i = 0;
	for(i = 0; i < 9; i++)
	{
		matrix->values[i] = oqt_read_fixed32(file);
	}
}

void oqt_matrix_dump(oqt_matrix_t *matrix)
{
	int i;
	printf("   matrix");
	for(i = 0; i < 9; i++) printf(" %f", (double)matrix->values[i]);
	printf("\n");
}

void oqt_write_matrix(oqt_t *file, oqt_matrix_t *matrix)
{
	int i;
	for(i = 0; i < 9; i++)
	{
		oqt_write_fixed32(file, matrix->values[i]);
	}
}
