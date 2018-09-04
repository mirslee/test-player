/* colormodels.c
 * Copyright (C) 2002 QT4Linux and OpenQuicktime Teams
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
 * $Id: colormodels.c,v 1.9 2003/04/07 21:02:21 shitowax Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"
#include "cmodel_permutation.h"



const char*
oqt_get_cmodel_name(int colormodel)
{
	switch(colormodel) {
		case BC_TRANSPARENCY:	return "TRANSPARENCY";
		case BC_COMPRESSED:		return "COMPRESSED";
		case BC_RGB8:			return "RGB8";
		case BC_RGB565:			return "RGB565";
		case BC_BGR565:			return "BGR565";
		case BC_BGR888:			return "BGR888";
		case BC_BGR8888:		return "BGR8888";
		
		case BC_RGB888:			return "RGB888";
		case BC_RGBA8888:		return "RGBA8888";
		case BC_ARGB8888:		return "ARGB8888";
		case BC_ABGR8888:		return "ABGR8888";
		case BC_RGB161616:		return "RGB161616";
		case BC_RGBA16161616:	return "RGBA16161616";
		case BC_YUV888:			return "YUV888";
		case BC_YUVA8888:		return "YUVA8888";
		case BC_YUV161616:		return "YUV161616";
		case BC_YUVA16161616:	return "YUVA16161616";
		case BC_YUV422:			return "YUV422";
		
		case BC_A8:				return "A8";
		case BC_A16:			return "A16";
		case BC_YUV101010:		return "YUV101010";
		case BC_VYU888:			return "VYU888";
		case BC_UYVA8888:		return "UYVA8888";

		case BC_YUV420P:		return "YUV420P";
		case BC_YUV422P:		return "YUV422P";
		case BC_YUV411P:		return "YUV411P";
		case BC_YUV444P:		return "YUV444P";

		case BC_M8:				return "M8";
		case BC_M4:				return "M4";
		case BC_M2:				return "M2";
		case BC_M1:				return "M1";

		case BC_YUV420PACKED:	return "YUV420PACKED";
		case BC_NONE:			return "NONE (Frame Drop)";
		case BC_UNSUPPORTED:	return "UNSUPPORTED";

		default:				return "Unknown";
	}
}


int
oqt_cmodel_is_rgb(int colormodel)
{
	switch(colormodel) {
		case BC_RGB8:			return 1;
		case BC_RGB565:			return 1;
		case BC_BGR565:			return 1;
		case BC_BGR888:			return 1;
		case BC_BGR8888:		return 1;
		
		case BC_RGB888:			return 1;
		case BC_RGBA8888:		return 1;
		case BC_ARGB8888:		return 1;
		case BC_ABGR8888:		return 1;
		case BC_RGB161616:		return 1;
		case BC_RGBA16161616:	return 1;
	}
	return 0;
}

int
oqt_cmodel_is_mapped(int colormodel)
{
	switch(colormodel) {
		case BC_M1:			return 1;
		case BC_M2:			return 1;
		case BC_M4:			return 1;
		case BC_M8:			return 1;
	}
	return 0;
}


int
oqt_cmodel_is_planar(int colormodel)
{
	switch(colormodel) {
		case BC_YUV420P:      return 1; break;
		case BC_YUV422P:      return 1; break;
		case BC_YUV444P:      return 1; break;
		case BC_YUV411P:      return 1; break;
	}
	return 0;
}


int
oqt_cmodel_is_yuv(int colormodel)
{
	switch(colormodel)
	{
		case BC_YUV888:
		case BC_YUVA8888:
		case BC_YUV161616:
		case BC_YUVA16161616:
		case BC_YUV422:
		case BC_YUV420P:
		case BC_YUV422P:
		case BC_YUV444P:
		case BC_YUV411P:
			return 1;
			break;
		
		default:
			return 0;
			break;
	}
}

int
oqt_cmodel_components(int colormodel)
{
	switch(colormodel) {
		case BC_A8:           return 1; break;
		case BC_A16:          return 1; break;
		case BC_RGB8:   	  return 3; break;
		case BC_RGB565:   	  return 3; break;
		case BC_BGR565:   	  return 3; break;
		case BC_RGB888:       return 3; break;
		case BC_BGR888:       return 3; break;
		case BC_RGBA8888:     return 4; break;
		case BC_ARGB8888:     return 4; break;
		case BC_ABGR8888:     return 4; break;
		case BC_RGB161616:    return 3; break;
		case BC_RGBA16161616: return 4; break;
		case BC_YUV888:       return 3; break;
		case BC_YUVA8888:     return 4; break;
		case BC_YUV161616:    return 3; break;
		case BC_YUVA16161616: return 4; break;
		case BC_YUV101010:    return 3; break;
	}
	fprintf(stderr,"oqt_cmodel_components: unknown cmodel %d\n", colormodel);
	return -1;
}

int
oqt_cmodel_calculate_pixelsize(int colormodel)
{
	switch(colormodel) {
		case BC_A8:           return 1; break;
		case BC_A16:          return 2; break;
		case BC_TRANSPARENCY: return 1; break;
		case BC_COMPRESSED:   return 1; break;
		case BC_RGB8:         return 1; break;
		case BC_RGB565:       return 2; break;
		case BC_BGR565:       return 2; break;
		case BC_BGR888:       return 3; break;
		case BC_BGR8888:      return 4; break;
		// Working bitmaps are packed to simplify processing
		case BC_RGB888:       return 3; break;
		case BC_ARGB8888:     return 4; break;
		case BC_ABGR8888:     return 4; break;
		case BC_RGBA8888:     return 4; break;
		case BC_RGB161616:    return 6; break;
		case BC_RGBA16161616: return 8; break;
		case BC_YUV888:       return 3; break;
		case BC_YUVA8888:     return 4; break;
		case BC_YUV161616:    return 6; break;
		case BC_YUVA16161616: return 8; break;
		case BC_YUV101010:    return 4; break;
		case BC_VYU888:       return 3; break;
		case BC_UYVA8888:     return 4; break;
		// Planar
		case BC_YUV420P:      return 1; break;
		case BC_YUV422P:      return 1; break;
		case BC_YUV444P:      return 1; break;
		case BC_YUV422:       return 2; break;
		case BC_YUV411P:      return 1; break;
	}
	fprintf(stderr,"oqt_cmodel_calculate_pixelsize: unknown cmodel %d\n", colormodel);
	return -1;
}

int
oqt_cmodel_calculate_max(int colormodel)
{
	switch(colormodel) {
		// Working bitmaps are packed to simplify processing
		case BC_A8:           return 0xff; break;
		case BC_A16:          return 0xffff; break;
		case BC_RGB888:       return 0xff; break;
		case BC_BGR888:       return 0xff; break;
		case BC_RGBA8888:     return 0xff; break;
		case BC_ARGB8888:     return 0xff; break;
		case BC_ABGR8888:     return 0xff; break;
		case BC_RGB161616:    return 0xffff; break;
		case BC_RGBA16161616: return 0xffff; break;
		case BC_YUV888:       return 0xff; break;
		case BC_YUVA8888:     return 0xff; break;
		case BC_YUV161616:    return 0xffff; break;
		case BC_YUVA16161616: return 0xffff; break;
	}
	fprintf(stderr,"oqt_cmodel_calculate_max unknown cmodel %d\n", colormodel);
	return -1;
}

int
oqt_cmodel_calculate_framesize(int w, int h, int color_model, int bytes_per_line)
{
	if(bytes_per_line <= 0) bytes_per_line = w * oqt_cmodel_calculate_pixelsize(color_model);
	switch(color_model) {
		case BC_YUV420P:
		case BC_YUV411P:
			return w * h + w * h / 2 + 4;
		break;
		
		case BC_YUV422P:
			return w * h * 2 + 4;
		break;
		
		case BC_YUV444P:
			return w * h * 3 + 4;
			break;

		default:
			return h * bytes_per_line + 4;
		break;
	}
	return -1;
}





cmodel_yuv_t *oqt_yuv_table = NULL;

static void oqt_cmodel_init_yuv(cmodel_yuv_t *yuv_table) {
	int i;
	for(i = 0; i < 0x100; i++)
	{
		/* compression */
		yuv_table->rtoy_tab[i] = (int)( 0.2990 * 0x10000 * i);
		yuv_table->rtou_tab[i] = (int)(-0.1687 * 0x10000 * i);
		yuv_table->rtov_tab[i] = (int)( 0.5000 * 0x10000 * i);
		
		yuv_table->gtoy_tab[i] = (int)( 0.5870 * 0x10000 * i);
		yuv_table->gtou_tab[i] = (int)(-0.3320 * 0x10000 * i);
		yuv_table->gtov_tab[i] = (int)(-0.4187 * 0x10000 * i);
		
		yuv_table->btoy_tab[i] = (int)( 0.1140 * 0x10000 * i);
		yuv_table->btou_tab[i] = (int)( 0.5000 * 0x10000 * i) + 0x800000;
		yuv_table->btov_tab[i] = (int)(-0.0813 * 0x10000 * i) + 0x800000;
	}
	
	yuv_table->vtor = &(yuv_table->vtor_tab[0x80]);
	yuv_table->vtog = &(yuv_table->vtog_tab[0x80]);
	yuv_table->utog = &(yuv_table->utog_tab[0x80]);
	yuv_table->utob = &(yuv_table->utob_tab[0x80]);
	for(i = -0x80; i < 0x80; i++)
	{
		/* decompression */
		yuv_table->vtor[i] = (int)( 1.4020 * 0x10000 * i);
		yuv_table->vtog[i] = (int)(-0.7141 * 0x10000 * i);
		
		yuv_table->utog[i] = (int)(-0.3441 * 0x10000 * i);
		yuv_table->utob[i] = (int)( 1.7720 * 0x10000 * i);
	}
	
	for(i = 0; i < 0x10000; i++)
	{
		/* compression */
		yuv_table->rtoy_tab16[i] = (int)( 0.2990 * 0x100 * i);
		yuv_table->rtou_tab16[i] = (int)(-0.1687 * 0x100 * i);
		yuv_table->rtov_tab16[i] = (int)( 0.5000 * 0x100 * i);
		
		yuv_table->gtoy_tab16[i] = (int)( 0.5870 * 0x100 * i);
		yuv_table->gtou_tab16[i] = (int)(-0.3320 * 0x100 * i);
		yuv_table->gtov_tab16[i] = (int)(-0.4187 * 0x100 * i);
		
		yuv_table->btoy_tab16[i] = (int)( 0.1140 * 0x100 * i);
		yuv_table->btou_tab16[i] = (int)( 0.5000 * 0x100 * i) + 0x800000;
		yuv_table->btov_tab16[i] = (int)(-0.0813 * 0x100 * i) + 0x800000;
	}
	
	yuv_table->vtor16 = &(yuv_table->vtor_tab16[0x8000]);
	yuv_table->vtog16 = &(yuv_table->vtog_tab16[0x8000]);
	yuv_table->utog16 = &(yuv_table->utog_tab16[0x8000]);
	yuv_table->utob16 = &(yuv_table->utob_tab16[0x8000]);
	for(i = -0x8000; i < 0x8000; i++)
	{
		/* decompression */
		yuv_table->vtor16[i] = (int)( 1.4020 * 0x100 * i);
		yuv_table->vtog16[i] = (int)(-0.7141 * 0x100 * i);
		
		yuv_table->utog16[i] = (int)(-0.3441 * 0x100 * i);
		yuv_table->utob16[i] = (int)( 1.7720 * 0x100 * i);
	}
}

static void oqt_cmodel_get_scale_tables(int **column_table, 
			     int **row_table, 
			     int in_x1, 
			     int in_y1, 
			     int in_x2, 
			     int in_y2,
			     int out_x1, 
			     int out_y1, 
			     int out_x2, 
			     int out_y2)
{
	int i;
	float w_in = (float)in_x2 - in_x1;
	float h_in = (float)in_y2 - in_y1;
	int w_out = out_x2 - out_x1;
	int h_out = out_y2 - out_y1;
	
	float hscale = w_in / w_out;
	float vscale = h_in / h_out;
	
	(*column_table) = (int*)malloc(sizeof(int) * w_out);
	(*row_table) = (int*)malloc(sizeof(int) * h_out);
	for(i = 0; i < w_out; i++)
	{
		(*column_table)[i] = (int)(hscale * i) + in_x1;
	}
	
	for(i = 0; i < h_out; i++)
	{
		(*row_table)[i] = (int)(vscale * i) + in_y1;
	}
}

void oqt_cmodel_transfer(BYTE **output_rows, 
		     BYTE **input_rows,
		     BYTE *out_y_plane,
		     BYTE *out_u_plane,
		     BYTE *out_v_plane,
		     BYTE *in_y_plane,
		     BYTE *in_u_plane,
		     BYTE *in_v_plane,
		     int in_x, 
		     int in_y, 
		     int in_w, 
		     int in_h,
		     int out_x, 
		     int out_y, 
		     int out_w, 
		     int out_h,
		     int in_colormodel, 
		     int out_colormodel,
		     int bg_color,
		     int in_rowspan,
		     int out_rowspan)
{
	int *column_table;
	int *row_table;
	int scale;
	int bg_r, bg_g, bg_b;
	int in_pixelsize = oqt_cmodel_calculate_pixelsize(in_colormodel);
	int out_pixelsize = oqt_cmodel_calculate_pixelsize(out_colormodel);
	
	bg_r = (bg_color & 0xff0000) >> 16;
	bg_g = (bg_color & 0xff00) >> 8;
	bg_b = (bg_color & 0xff);
	
	// Initialize tables
	if(oqt_yuv_table == NULL) {
		oqt_yuv_table = (cmodel_yuv_t*)calloc(1, sizeof(cmodel_yuv_t));
		oqt_cmodel_init_yuv(oqt_yuv_table);
	}
	
	// Get scaling
	scale = (out_w != in_w) || (in_x != 0);
	oqt_cmodel_get_scale_tables(&column_table, &row_table, 
		in_x, in_y, in_x + in_w, in_y + in_h,
		out_x, out_y, out_x + out_w, out_y + out_h);
	
	// Handle planar cmodels separately
	switch(in_colormodel)
	{
		case BC_YUV420P:
		case BC_YUV422P:
			oqt_cmodel_transfer_yuv420p(output_rows,  \
				input_rows, \
				out_y_plane, \
				out_u_plane, \
				out_v_plane, \
				in_y_plane, \
				in_u_plane, \
				in_v_plane, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_x,  \
				out_y,  \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				bg_color, \
				in_rowspan, \
				out_rowspan, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table, \
				bg_r, \
				bg_g, \
				bg_b);
			break;

		case BC_YUV444P:
			oqt_cmodel_transfer_yuv444p(output_rows,  \
				input_rows, \
				out_y_plane, \
				out_u_plane, \
				out_v_plane, \
				in_y_plane, \
				in_u_plane, \
				in_v_plane, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_x,  \
				out_y,  \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				bg_color, \
				in_rowspan, \
				out_rowspan, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table, \
				bg_r, \
				bg_g, \
				bg_b);
			break;

		case BC_YUV422:
			oqt_cmodel_transfer_yuv422(output_rows,  \
				input_rows, \
				out_y_plane, \
				out_u_plane, \
				out_v_plane, \
				in_y_plane, \
				in_u_plane, \
				in_v_plane, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_x,  \
				out_y,  \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				bg_color, \
				in_rowspan, \
				out_rowspan, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table, \
				bg_r, \
				bg_g, \
				bg_b);
			break;

		default:
			oqt_cmodel_transfer_default(output_rows,  \
				input_rows, \
				out_y_plane, \
				out_u_plane, \
				out_v_plane, \
				in_y_plane, \
				in_u_plane, \
				in_v_plane, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_x,  \
				out_y,  \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				bg_color, \
				in_rowspan, \
				out_rowspan, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table, \
				bg_r, \
				bg_g, \
				bg_b);
			break;
	}

	free(column_table);
	free(row_table);
}
