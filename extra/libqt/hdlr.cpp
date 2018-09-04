/* hdlr.c
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
 * $Id: hdlr.c,v 1.6 2002/10/28 13:36:44 nj_humfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"

#pragma warning(disable:4996)

void oqt_hdlr_init(oqt_hdlr_t *hdlr)
{
	hdlr->version = 0;
	hdlr->flags = 0;
	hdlr->component_type[0] = 'm';
	hdlr->component_type[1] = 'h';
	hdlr->component_type[2] = 'l';
	hdlr->component_type[3] = 'r';
	hdlr->component_subtype[0] = 'x';
	hdlr->component_subtype[1] = 'x';
	hdlr->component_subtype[2] = 'x';
	hdlr->component_subtype[3] = 'x';
	hdlr->component_manufacturer[0] = 'o';
	hdlr->component_manufacturer[1] = 'q';
	hdlr->component_manufacturer[2] = 't';
	hdlr->component_manufacturer[3] = ' ';
	hdlr->component_flags = 0;
	hdlr->component_flag_mask = 0;
	strcpy(hdlr->component_name,"Jetsen QT Media Handler");
}

void oqt_hdlr_init_video(oqt_hdlr_t *hdlr)
{
	hdlr->component_subtype[0] = 'v';
	hdlr->component_subtype[1] = 'i';
	hdlr->component_subtype[2] = 'd';
	hdlr->component_subtype[3] = 'e';
	strcpy(hdlr->component_name,"Jetsen QT Video Media Handler");
}

void oqt_hdlr_init_audio(oqt_hdlr_t *hdlr)
{
	hdlr->component_subtype[0] = 's';
	hdlr->component_subtype[1] = 'o';
	hdlr->component_subtype[2] = 'u';
	hdlr->component_subtype[3] = 'n';
	strcpy(hdlr->component_name,"Jetsen QT Sound Media Handler");
}

void oqt_hdlr_init_data(oqt_hdlr_t *hdlr)
{
	hdlr->component_type[0] = 'd';
	hdlr->component_type[1] = 'h';
	hdlr->component_type[2] = 'l';
	hdlr->component_type[3] = 'r';
	hdlr->component_subtype[0] = 'a';
	hdlr->component_subtype[1] = 'l';
	hdlr->component_subtype[2] = 'i';
	hdlr->component_subtype[3] = 's';
	strcpy(hdlr->component_name,"Jetsen QT Alias Data Handler");
}

void oqt_hdlr_delete(oqt_hdlr_t *hdlr)
{
}

void oqt_hdlr_dump(oqt_hdlr_t *hdlr)
{
	printf("   handler reference (hdlr)\n");
	printf("    version %d\n", hdlr->version);
	printf("    flags %d\n", hdlr->flags);
	printf("    component_type %c%c%c%c\n", hdlr->component_type[0], hdlr->component_type[1], hdlr->component_type[2], hdlr->component_type[3]);
	printf("    component_subtype %c%c%c%c\n", hdlr->component_subtype[0], hdlr->component_subtype[1], hdlr->component_subtype[2], hdlr->component_subtype[3]);
	printf("    component_manufacturer %c%c%c%c\n", hdlr->component_manufacturer[0], hdlr->component_manufacturer[1], hdlr->component_manufacturer[2], hdlr->component_manufacturer[3]);
	printf("    component_name %s\n", hdlr->component_name);
}

void oqt_read_hdlr(oqt_t *file, oqt_hdlr_t *hdlr)
{
	hdlr->version = oqt_read_char(file);
	hdlr->flags = oqt_read_int24(file);
	oqt_read_char32(file, hdlr->component_type);
	oqt_read_char32(file, hdlr->component_subtype);
	oqt_read_char32(file, hdlr->component_manufacturer);
	hdlr->component_flags = oqt_read_int32(file);
	hdlr->component_flag_mask = oqt_read_int32(file);
	oqt_read_pascal(file, hdlr->component_name);
}

void oqt_write_hdlr(oqt_t *file, oqt_hdlr_t *hdlr)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "hdlr");

	oqt_write_char(file, (char)hdlr->version);
	oqt_write_int24(file, hdlr->flags);
	oqt_write_char32(file, hdlr->component_type);
	oqt_write_char32(file, hdlr->component_subtype);
	oqt_write_char32(file, hdlr->component_manufacturer);
	oqt_write_int32(file, hdlr->component_flags);
	oqt_write_int32(file, hdlr->component_flag_mask);
	oqt_write_pascal(file, hdlr->component_name);

	oqt_atom_write_footer(file, &atom);
}
