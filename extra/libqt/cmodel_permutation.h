/* cmodel_permutation.h
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
 * $Id: cmodel_permutation.h,v 1.10 2003/01/21 18:15:36 gordonshumway Exp $
 */

#include "qttype.h"
#include "colormodels.h"

typedef struct {
  int rtoy_tab[0x100], gtoy_tab[0x100], btoy_tab[0x100];
  int rtou_tab[0x100], gtou_tab[0x100], btou_tab[0x100];
  int rtov_tab[0x100], gtov_tab[0x100], btov_tab[0x100];

  int vtor_tab[0x100], vtog_tab[0x100];
  int utog_tab[0x100], utob_tab[0x100];
  int *vtor, *vtog, *utog, *utob;

  int rtoy_tab16[0x10000], gtoy_tab16[0x10000], btoy_tab16[0x10000];
  int rtou_tab16[0x10000], gtou_tab16[0x10000], btou_tab16[0x10000];
  int rtov_tab16[0x10000], gtov_tab16[0x10000], btov_tab16[0x10000];

  int vtor_tab16[0x10000], vtog_tab16[0x10000];
  int utog_tab16[0x10000], utob_tab16[0x10000];
  int *vtor16, *vtog16, *utog16, *utob16;
} cmodel_yuv_t;


extern cmodel_yuv_t *oqt_yuv_table;


#define RECLIP(x, y, z) ((x) = ((x) < (y) ? (y) : ((x) > (z) ? (z) : (x))))

#define RGB_TO_YUV(y, u, v, r, g, b) \
{ \
	y = ((oqt_yuv_table->rtoy_tab[r] + oqt_yuv_table->gtoy_tab[g] + oqt_yuv_table->btoy_tab[b]) >> 16); \
	u = ((oqt_yuv_table->rtou_tab[r] + oqt_yuv_table->gtou_tab[g] + oqt_yuv_table->btou_tab[b]) >> 16); \
	v = ((oqt_yuv_table->rtov_tab[r] + oqt_yuv_table->gtov_tab[g] + oqt_yuv_table->btov_tab[b]) >> 16); \
	RECLIP(y, 0, 0xff); \
	RECLIP(u, 0, 0xff); \
	RECLIP(v, 0, 0xff); \
}

// y -> 24 bits u, v, -> 8 bits r, g, b -> 8 bits
#define YUV_TO_RGB(y, u, v, r, g, b) \
{ \
	(r) = ((y + oqt_yuv_table->vtor_tab[v]) >> 16); \
	(g) = ((y + oqt_yuv_table->utog_tab[u] + oqt_yuv_table->vtog_tab[v]) >> 16); \
	(b) = ((y + oqt_yuv_table->utob_tab[u]) >> 16); \
	RECLIP(r, 0, 0xff); \
	RECLIP(g, 0, 0xff); \
	RECLIP(b, 0, 0xff); \
}

// r, g, b -> 16 bits
#define RGB_TO_YUV16(y, u, v, r, g, b) \
{ \
	y = ((oqt_yuv_table->rtoy_tab16[r] + oqt_yuv_table->gtoy_tab16[g] + oqt_yuv_table->btoy_tab16[b]) >> 8); \
	u = ((oqt_yuv_table->rtou_tab16[r] + oqt_yuv_table->gtou_tab16[g] + oqt_yuv_table->btou_tab16[b]) >> 8); \
	v = ((oqt_yuv_table->rtov_tab16[r] + oqt_yuv_table->gtov_tab16[g] + oqt_yuv_table->btov_tab16[b]) >> 8); \
	RECLIP(y, 0, 0xffff); \
	RECLIP(u, 0, 0xffff); \
	RECLIP(v, 0, 0xffff); \
}

// y -> 24 bits   u, v-> 16 bits
#define YUV_TO_RGB16(y, u, v, r, g, b) \
{ \
	(r) = ((y + oqt_yuv_table->vtor_tab16[v]) >> 8); \
	(g) = ((y + oqt_yuv_table->utog_tab16[u] + oqt_yuv_table->vtog_tab16[v]) >> 8); \
	(b) = ((y + oqt_yuv_table->utob_tab16[u]) >> 8); \
	RECLIP(r, 0, 0xffff); \
	RECLIP(g, 0, 0xffff); \
	RECLIP(b, 0, 0xffff); \
}


#define WRITE_YUV101010(y, u, v) \
{ \
	ULONG output_i = ((y & 0xffc0) << 16) | \
		((u & 0xffc0) << 6) | \
		((v & 0xffc0) >> 4); \
	*(*output)++ = (BYTE)((output_i & 0xff)); \
	*(*output)++ = (BYTE)((output_i & 0xff00) >> 8); \
	*(*output)++ = (BYTE)((output_i & 0xff0000) >> 16); \
	*(*output)++ = (BYTE)((output_i & 0xff000000) >> 24); \
}



// ****************************** Pixel transfers *****************************






// ****************************** ARGB8888 -> *********************************

static inline void transfer_ARGB8888_to_ARGB8888(BYTE *(*output), BYTE *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output)[3] = input[3];
	(*output) += 4;
}

static inline void transfer_ARGB8888_to_RGBA8888(BYTE *(*output), BYTE *input)
{
	(*output)[0] = input[1];
	(*output)[1] = input[2];
	(*output)[2] = input[3];
	(*output)[3] = input[0];
	(*output) += 4;
}


// ******************************** RGB565 -> *********************************

static inline void transfer_RGB565_to_RGB888(BYTE *(*output), BYTE *input)
{
	WORD r, g, b, d = (*input++ << 8); d |= *input++;
	
	r = (d >> 10) & 0x1f; (*output)[0] = (r << 3) | (r >> 2);
	g = (d >>  5) & 0x1f; (*output)[1] = (g << 3) | (g >> 2);
	b =  d & 0x1f;		  (*output)[2] = (b << 3) | (b >> 2);
	(*output) += 3;
}

static inline void transfer_RGB565_to_BGR888(BYTE *(*output), BYTE *input)
{
	WORD r, g, b, d = (*input++ << 8); d |= *input++;
	
	b =  d & 0x1f;		  (*output)[0] = (b << 3) | (b >> 2);
	g = (d >>  5) & 0x1f; (*output)[1] = (g << 3) | (g >> 2);
	r = (d >> 10) & 0x1f; (*output)[2] = (r << 3) | (r >> 2);
	(*output) += 3;
}

static inline void transfer_RGB565_to_RGBA8888(BYTE *(*output), BYTE *input)
{
	WORD r, g, b, d = (*input++ << 8); d |= *input++;
	
	r = (d >> 10) & 0x1f; (*output)[0] = (r << 3) | (r >> 2);
	g = (d >>  5) & 0x1f; (*output)[1] = (g << 3) | (g >> 2);
	b =  d & 0x1f;		  (*output)[2] = (b << 3) | (b >> 2);
	(*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_RGB565_to_ARGB8888(BYTE *(*output), BYTE *input)
{
	WORD r, g, b, d = (*input++ << 8); d |= *input++;
	
	(*output)[0] = 0xff;
	r = (d >> 10) & 0x1f; (*output)[1] = (r << 3) | (r >> 2);
	g = (d >>  5) & 0x1f; (*output)[2] = (g << 3) | (g >> 2);
	b =  d & 0x1f;		  (*output)[3] = (b << 3) | (b >> 2);
	(*output) += 4;
}

static inline void transfer_RGB565_to_ABGR8888(BYTE *(*output), BYTE *input)
{
	WORD r, g, b, d = (*input++ << 8); d |= *input++;
	
	(*output)[0] = 0xff;
	b =  d & 0x1f;		  (*output)[1] = (b << 3) | (b >> 2);
	g = (d >>  5) & 0x1f; (*output)[2] = (g << 3) | (g >> 2);
	r = (d >> 10) & 0x1f; (*output)[3] = (r << 3) | (r >> 2);
	(*output) += 4;
}


// ******************************** RGB888 -> *********************************

static inline void transfer_RGB888_to_RGB8(BYTE *(*output), BYTE *input)
{
  *(*output) = (BYTE)((input[0] & 0xc0) +
			       ((input[1] & 0xe0) >> 2) +
			       ((input[2] & 0xe0) >> 5));
  (*output)++;
}

static inline void transfer_RGB888_to_BGR565(BYTE *(*output), BYTE *input)
{
	WORD r, g, b;
	WORD r_s, g_s, b_s;
	r = *input++;
	g = *input++;
	b = *input;
	
	r_s  = (r & 0x01) << 7;
	r_s |= (r & 0x02) << 5;
	r_s |= (r & 0x04) << 3;
	r_s |= (r & 0x08) << 1;
	r_s |= (r & 0x10) >> 1;
	r_s |= (r & 0x20) >> 3;
	r_s |= (r & 0x40) >> 5;
	r_s |= (r & 0x80) >> 7;

	g_s  = (g & 0x01) << 7;
	g_s |= (g & 0x02) << 5;
	g_s |= (g & 0x04) << 3;
	g_s |= (g & 0x08) << 1;
	g_s |= (g & 0x10) >> 1;
	g_s |= (g & 0x20) >> 3;
	g_s |= (g & 0x40) >> 5;
	g_s |= (g & 0x80) >> 7;

	b_s  = (b & 0x01) << 7;
	b_s |= (b & 0x02) << 5;
	b_s |= (b & 0x04) << 3;
	b_s |= (b & 0x08) << 1;
	b_s |= (b & 0x10) >> 1;
	b_s |= (b & 0x20) >> 3;
	b_s |= (b & 0x40) >> 5;
	b_s |= (b & 0x80) >> 7;

	*(WORD*)(*output) = ((b_s & 0xf8) << 8)
			 + ((g_s & 0xfc) << 3)
			 + ((r_s & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_RGB888_to_RGB565(BYTE *(*output), BYTE *input)
{
  WORD r, g, b;
  r = *input++;
  g = *input++;
  b = *input;
  *(WORD*)(*output) = ((r & 0xf8) << 8)
    + ((g & 0xfc) << 3)
    + ((b & 0xf8) >> 3);
  (*output) += 2;
}

static inline void transfer_RGB888_to_BGR888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[2];
  (*output)[1] = input[1];
  (*output)[2] = input[0];
  (*output) += 3;
}

static inline void transfer_RGB888_to_RGB888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output) += 3;
}

static inline void transfer_RGB888_to_RGBA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output)[3] = 0xff;
  (*output) += 4;
}

static inline void transfer_RGB888_to_RGB161616(WORD *(*output), BYTE *input)
{
	(*output)[0] = (input[0] << 8) | input[0];
	(*output)[1] = (input[1] << 8) | input[1];
	(*output)[2] = (input[2] << 8) | input[2];
	(*output) += 3;
}

static inline void transfer_RGB888_to_RGBA16161616(WORD *(*output), BYTE *input)
{
	(*output)[0] = (input[0] << 8) | input[0];
	(*output)[1] = (input[1] << 8) | input[1];
	(*output)[2] = (input[2] << 8) | input[2];
	(*output)[3] = 0xffff;
	(*output) += 4;
}

static inline void transfer_RGB888_to_ARGB8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = 0xff;
  (*output)[1] = input[0];
  (*output)[2] = input[1];
  (*output)[3] = input[2];
  (*output) += 4;
}

static inline void transfer_RGB888_to_ABGR8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = 0xff;
  (*output)[1] = input[2];
  (*output)[2] = input[1];
  (*output)[3] = input[0];
  (*output) += 4;
}

static inline void transfer_RGB888_to_BGR8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[2];
  (*output)[1] = input[1];
  (*output)[2] = input[0];
  (*output) += 4;
}

static inline void transfer_RGB888_to_YUV888(BYTE *(*output), BYTE *input)
{
  int y, u, v;

  RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

  (*output)[0] = y;
  (*output)[1] = u;
  (*output)[2] = v;
  (*output) += 3;
}


static inline void transfer_RGB888_to_YUV101010(BYTE *(*output), BYTE *input)
{
  int r, g, b;
  int y, u, v;

  r = ((WORD)input[0]) << 8;
  g = ((WORD)input[1]) << 8;
  b = ((WORD)input[2]) << 8;
  RGB_TO_YUV16(y, u, v, r, g, b);
  WRITE_YUV101010(y, u, v);
}

static inline void transfer_RGB888_to_VYU888(BYTE *(*output), BYTE *input)
{
  int y, u, v;

  RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

  (*output)[0] = v;
  (*output)[1] = y;
  (*output)[2] = u;
  (*output) += 3;
}

static inline void transfer_RGB888_to_UYVA8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;

  RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

  (*output)[0] = u;
  (*output)[1] = y;
  (*output)[2] = v;
  (*output)[3] = 0xff;
  (*output) += 4;
}



static inline void transfer_RGB888_to_YUVA8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;

  RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

  (*output)[0] = y;
  (*output)[1] = u;
  (*output)[2] = v;
  (*output)[3] = 255;
  (*output) += 4;
}

static inline void transfer_RGB888_to_YUV161616(WORD *(*output), BYTE *input)
{
	int y, u, v, r, g, b;
	
	r = ((int)input[0] << 8) | input[0];
	g = ((int)input[1] << 8) | input[1];
	b = ((int)input[2] << 8) | input[2];

	RGB_TO_YUV16(y, u, v, r, g, b);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output) += 3;
}

static inline void transfer_RGB888_to_YUVA16161616(WORD *(*output), BYTE *input)
{
	int y, u, v, r, g, b;

	r = (((int)input[0]) << 8) | input[0];
	g = (((int)input[1]) << 8) | input[1];
	b = (((int)input[2]) << 8) | input[2];
	RGB_TO_YUV16(y, u, v, r, g, b);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output)[3] = 0xffff;
	(*output) += 4;
}

static inline void transfer_RGB888_to_YUV420P_YUV422P(BYTE *output_y, 
						      BYTE *output_u, 
						      BYTE *output_v, 
						      BYTE *input,
						      int output_column)
{
  int y, u, v;

  RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

  output_y[output_column] = y;
  output_u[output_column / 2] = u;
  output_v[output_column / 2] = v;
}

static inline void transfer_RGB888_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	BYTE *input,
	int output_column)
{
	int y, u, v;

	RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGB888_to_YUV422(BYTE *(*output), 
	BYTE *input,
	int j)
{
	int y, u, v;

	RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

	if(!(j & 1))
	{ 
// Store U and V for even pixels only
		 (*output)[1] = u;
		 (*output)[3] = v;
		 (*output)[0] = y;
	}
	else
	{ 
// Store Y and advance output for odd pixels only
		 (*output)[2] = y;
		 (*output) += 4;
	}

}







// *************************** RGBA8888 -> ************************************

static inline void transfer_RGBA8888_to_TRANSPARENCY(BYTE *(*output), BYTE *input, int (*bit_counter))
{
  if((*bit_counter) == 7) *(*output) = 0;

  if(input[3] < 127) 
    {
      *(*output) |= (BYTE)1 << (7 - (*bit_counter));
    }

  if((*bit_counter) == 0)
    {
      (*output)++;
      (*bit_counter) = 7;
    }
  else
    (*bit_counter)--;
}

// These routines blend in a background color since they should be
// exclusively used for widgets.

static inline void transfer_RGBA8888_to_RGB8bg(BYTE *(*output), BYTE *input, int bg_r, int bg_g, int bg_b)
{
  unsigned int r, g, b, a, anti_a;
  a = input[3];
  anti_a = 255 - a;
	r = ((unsigned int)input[0] * a + bg_r * anti_a) / 0xff;
	g = ((unsigned int)input[1] * a + bg_g * anti_a) / 0xff;
	b = ((unsigned int)input[2] * a + bg_b * anti_a) / 0xff;
	*(*output) = (BYTE)((r & 0xc0) + 
				((g & 0xe0) >> 2) + 
				((b & 0xe0) >> 5));
	(*output)++;
}

static inline void transfer_RGBA8888_to_BGR565bg(BYTE *(*output), BYTE *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;
	r = ((unsigned int)input[0] * a + bg_r * anti_a) / 0xff;
	g = ((unsigned int)input[1] * a + bg_g * anti_a) / 0xff;
	b = ((unsigned int)input[2] * a + bg_b * anti_a) / 0xff;
	*(WORD*)(*output) = (WORD)(((b & 0xf8) << 8) + 
				((g & 0xfc) << 3) + 
				((r & 0xf8) >> 3));
	(*output) += 2;
}

static inline void transfer_RGBA8888_to_RGB565bg(BYTE *(*output), BYTE *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;
	r = ((unsigned int)input[0] * a + bg_r * anti_a) / 0xff;
	g = ((unsigned int)input[1] * a + bg_g * anti_a) / 0xff;
	b = ((unsigned int)input[2] * a + bg_b * anti_a) / 0xff;
	*(WORD*)(*output) = (WORD)(((r & 0xf8) << 8)+ 
				((g & 0xfc) << 3) + 
				((b & 0xf8) >> 3));
	(*output) += 2;
}

static inline void transfer_RGBA8888_to_BGR888bg(BYTE *(*output), BYTE *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;
	r = ((unsigned int)input[0] * a + bg_r * anti_a) / 0xff;
	g = ((unsigned int)input[1] * a + bg_g * anti_a) / 0xff;
	b = ((unsigned int)input[2] * a + bg_b * anti_a) / 0xff;
	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_RGB888bg(BYTE *(*output), BYTE *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;
	r = ((unsigned int)input[0] * a + bg_r * anti_a) / 0xff;
	g = ((unsigned int)input[1] * a + bg_g * anti_a) / 0xff;
	b = ((unsigned int)input[2] * a + bg_b * anti_a) / 0xff;
	(*output)[0] = r;
	(*output)[1] = g;
	(*output)[2] = b;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_BGR8888bg(BYTE *(*output), BYTE *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;

	r = ((unsigned int)input[0] * a + bg_r * anti_a) / 0xff;
	g = ((unsigned int)input[1] * a + bg_g * anti_a) / 0xff;
	b = ((unsigned int)input[2] * a + bg_b * anti_a) / 0xff;

	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 4;
}







// These routines blend in a black background

static inline void transfer_RGBA8888_to_RGB8(BYTE *(*output), BYTE *input)
{
  unsigned int r, g, b, a;
  a = input[3];
  r = (unsigned int)input[0] * a;
  g = (unsigned int)input[1] * a;
  b = (unsigned int)input[2] * a;
  *(*output) = (BYTE)(((r & 0xc000) >> 8) + 
			       ((g & 0xe000) >> 10) + 
			       ((b & 0xe000) >> 13));
  (*output)++;
}

static inline void transfer_RGBA8888_to_BGR565(BYTE *(*output), BYTE *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a) / 0xff;
	g = ((unsigned int)input[1] * a) / 0xff;
	b = ((unsigned int)input[2] * a) / 0xff;
	*(WORD*)(*output) = (WORD)(((b & 0xf8) << 8) + 
				((g & 0xfc) << 3) + 
				((r & 0xf8) >> 3));
	(*output) += 2;
}

static inline void transfer_RGBA8888_to_RGB565(BYTE *(*output), BYTE *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a) / 0xff;
	g = ((unsigned int)input[1] * a) / 0xff;
	b = ((unsigned int)input[2] * a) / 0xff;


	*(WORD*)(*output) = (WORD)(((r & 0xf8) << 8) + 
				((g & 0xfc) << 3) + 
				((b & 0xf8) >> 3));
	(*output) += 2;
}

static inline void transfer_RGBA8888_to_BGR888(BYTE *(*output), BYTE *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a) / 0xff;
	g = ((unsigned int)input[1] * a) / 0xff;
	b = ((unsigned int)input[2] * a) / 0xff;
	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_RGB888(BYTE *(*output), BYTE *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a) / 0xff;
	g = ((unsigned int)input[1] * a) / 0xff;
	b = ((unsigned int)input[2] * a) / 0xff;
	(*output)[0] = r;
	(*output)[1] = g;
	(*output)[2] = b;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_RGBA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output)[3] = input[3];
  (*output) += 4;
}

static inline void transfer_RGBA8888_to_RGB161616(WORD *(*output), BYTE *input)
{
	int opacity;
	
	opacity = input[3];
	(*output)[0] = (((int)input[0] << 8) | input[0]) * opacity / 0xff;
	(*output)[1] = (((int)input[1] << 8) | input[1]) * opacity / 0xff;
	(*output)[2] = (((int)input[2] << 8) | input[2]) * opacity / 0xff;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_RGBA16161616(WORD *(*output), BYTE *input)
{
	(*output)[0] = (((int)input[0]) << 8) | input[0];
	(*output)[1] = (((int)input[1]) << 8) | input[1];
	(*output)[2] = (((int)input[2]) << 8) | input[2];
	(*output)[3] = (((int)input[3]) << 8) | input[3];
	(*output) += 4;
}

static inline void transfer_RGBA8888_to_BGR8888(BYTE *(*output), BYTE *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a) / 0xff;
	g = ((unsigned int)input[1] * a) / 0xff;
	b = ((unsigned int)input[2] * a) / 0xff;
	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 4;
}

static inline void transfer_RGBA8888_to_YUV888(BYTE *(*output), BYTE *input)
{
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a) / 0xff;
	g = (input[1] * a) / 0xff;
	b = (input[2] * a) / 0xff;

	RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_YUVA8888(BYTE *(*output), BYTE *input)
{
	int y, u, v;

	RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output)[3] = input[3];
	(*output) += 4;
}

static inline void transfer_RGBA8888_to_YUV161616(WORD *(*output), BYTE *input)
{
	int y, u, v, opacity, r, g, b;
	
	opacity = input[3];
	r = (((int)input[0] << 8) | input[0]) * opacity / 0xff;
	g = (((int)input[1] << 8) | input[1]) * opacity / 0xff;
	b = (((int)input[2] << 8) | input[2]) * opacity / 0xff;

	RGB_TO_YUV16(y, u, v, r, g, b);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_YUVA16161616(WORD *(*output), BYTE *input)
{
	int y, u, v, r, g, b;

	r = (((int)input[0]) << 8) | input[0];
	g = (((int)input[1]) << 8) | input[1];
	b = (((int)input[2]) << 8) | input[2];
	RGB_TO_YUV16(y, u, v, r, g, b);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output)[3] = (((int)input[3]) << 8) | input[3];
	(*output) += 4;
}

static inline void transfer_RGBA8888_to_YUV101010(BYTE *(*output), BYTE *input)
{
  int r, g, b;
  int y, u, v;

  r = ((WORD)input[0] * input[3]) + 0x1fe;
  g = ((WORD)input[1] * input[3]) + 0x1fe;
  b = ((WORD)input[2] * input[3]) + 0x1fe;
  RGB_TO_YUV16(y, u, v, r, g, b);
  WRITE_YUV101010(y, u, v);
}

static inline void transfer_RGBA8888_to_VYU888(BYTE *(*output), BYTE *input)
{
  int y, u, v, a, r, g, b;
	
  a = input[3];
  r = ((input[0] * a) >> 8) + 1;
  g = ((input[1] * a) >> 8) + 1;
  b = ((input[2] * a) >> 8) + 1;

  RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

  (*output)[0] = v;
  (*output)[1] = y;
  (*output)[2] = u;
  (*output) += 3;
}

static inline void transfer_RGBA8888_to_UYVA8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;

  RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

  (*output)[0] = u;
  (*output)[1] = y;
  (*output)[2] = v;
  (*output)[3] = input[3];
  (*output) += 4;
}

static inline void transfer_RGBA888_to_YUV420P_YUV422P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	BYTE *input,
	int output_column)
{
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a) / 0xff;
	g = (input[1] * a) / 0xff;
	b = (input[2] * a) / 0xff;

	RGB_TO_YUV(y, u, v, r, g, b);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGBA888_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	BYTE *input,
	int output_column)
{
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a) / 0xff;
	g = (input[1] * a) / 0xff;
	b = (input[2] * a) / 0xff;

	RGB_TO_YUV(y, u, v, r, g, b);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGBA888_to_YUV422(BYTE *(*output), 
	BYTE *input,
	int j)
{
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a) / 0xff;
	g = (input[1] * a) / 0xff;
	b = (input[2] * a) / 0xff;

	RGB_TO_YUV(y, u, v, r, g, b);

	if(!(j & 1))
	{ 
// Store U and V for even pixels only
		 (*output)[1] = u;
		 (*output)[3] = v;
		 (*output)[0] = y;
	}
	else
	{ 
// Store Y and advance output for odd pixels only
		 (*output)[2] = y;
		 (*output) += 4;
	}

}













// ******************************** RGB161616 -> *********************************

static inline void transfer_RGB161616_to_RGB8(BYTE *(*output), WORD *input)
{
  *(*output) = (BYTE)(((input[0] & 0xc000) >> 8) +
			       ((input[1] & 0xe000) >> 10) +
			       ((input[2] & 0xe000) >> 13));
  (*output)++;
}

static inline void transfer_RGB161616_to_BGR565(BYTE *(*output), WORD *input)
{
	WORD r, g, b;
	r = *input++;
	g = *input++;
	b = *input;
	*(WORD*)(*output) = (b & 0xf800) |
			 ((g & 0xfc00) >> 5) |
			 ((r & 0xf800) >> 11);
	(*output) += 2;
}

static inline void transfer_RGB161616_to_RGB565(BYTE *(*output), WORD *input)
{
  WORD r, g, b;
  r = *input++;
  g = *input++;
  b = *input;
  *(WORD*)(*output) = (r & 0xf800) |
    ((g & 0xfc00) >> 5) |
    ((b & 0xf800) >> 11);
  (*output) += 2;
}

static inline void transfer_RGB161616_to_BGR888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[2] >> 8;
  (*output)[1] = input[1] >> 8;
  (*output)[2] = input[0] >> 8;
  (*output) += 3;
}

static inline void transfer_RGB161616_to_RGB888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[0] >> 8;
  (*output)[1] = input[1] >> 8;
  (*output)[2] = input[2] >> 8;
  (*output) += 3;
}

static inline void transfer_RGB161616_to_RGBA8888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[0] >> 8;
  (*output)[1] = input[1] >> 8;
  (*output)[2] = input[2] >> 8;
  (*output)[3] = 0xff;
  (*output) += 4;
}

static inline void transfer_RGB161616_to_BGR8888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[2] >> 8;
  (*output)[1] = input[1] >> 8;
  (*output)[2] = input[0] >> 8;
  (*output) += 4;
}

static inline void transfer_RGB161616_to_YUV888(BYTE *(*output), WORD *input)
{
  int y, u, v, r, g, b;
  r = input[0] >> 8;
  g = input[1] >> 8;
  b = input[2] >> 8;

  RGB_TO_YUV(y, u, v, r, g, b);

  (*output)[0] = y;
  (*output)[1] = u;
  (*output)[2] = v;
  (*output) += 3;
}

static inline void transfer_RGB161616_to_YUVA8888(BYTE *(*output), WORD *input)
{
  int y, u, v, r, g, b;

  r = input[0] >> 8;
  g = input[1] >> 8;
  b = input[2] >> 8;

  RGB_TO_YUV(y, u, v, r, g, b);

  (*output)[0] = y;
  (*output)[1] = u;
  (*output)[2] = v;
  (*output)[3] = 255;
  (*output) += 4;
}


static inline void transfer_RGB161616_to_YUV101010(BYTE *(*output), WORD *input)
{
  int r, g, b;
  int y, u, v;

  r = input[0];
  g = input[1];
  b = input[2];
  RGB_TO_YUV16(y, u, v, r, g, b);
  WRITE_YUV101010(y, u, v);
}

static inline void transfer_RGB161616_to_VYU888(BYTE *(*output), WORD *input)
{
  int y, u, v, r, g, b;
  r = input[0] >> 8;
  g = input[1] >> 8;
  b = input[2] >> 8;

  RGB_TO_YUV(y, u, v, r, g, b);

  (*output)[0] = v;
  (*output)[1] = y;
  (*output)[2] = u;
  (*output) += 3;
}

static inline void transfer_RGB161616_to_UYVA8888(BYTE *(*output), WORD *input)
{
  int y, u, v, r, g, b;

  r = input[0] >> 8;
  g = input[1] >> 8;
  b = input[2] >> 8;

  RGB_TO_YUV(y, u, v, r, g, b);

  (*output)[0] = u;
  (*output)[1] = y;
  (*output)[2] = v;
  (*output)[3] = 0xff;
  (*output) += 4;
}


static inline void transfer_RGB161616_to_YUV420P_YUV422P(BYTE *output_y, 
							 BYTE *output_u, 
							 BYTE *output_v, 
							 WORD *input,
							 int output_column)
{
  int y, u, v, r, g, b;
  r = input[0] >> 8;
  g = input[1] >> 8;
  b = input[2] >> 8;

  RGB_TO_YUV(y, u, v, r, g, b);

  output_y[output_column] = y;
  output_u[output_column / 2] = u;
  output_v[output_column / 2] = v;
}

static inline void transfer_RGB161616_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	WORD *input,
	int output_column)
{
	int y, u, v, r, g, b;
	r = input[0] >> 8;
	g = input[1] >> 8;
	b = input[2] >> 8;

	RGB_TO_YUV(y, u, v, r, g, b);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}


// ****************************** RGBA16161616 -> *****************************

static inline void transfer_RGBA16161616_to_RGB8(BYTE *(*output), WORD *input)
{
  unsigned int r, g, b, a;
  a = (input)[3] >> 8;
  r = (unsigned int)(input)[0] * a;
  g = (unsigned int)(input)[1] * a;
  b = (unsigned int)(input)[2] * a;

  *(*output) = (BYTE)(((r & 0xc00000) >> 16) + 
			       ((g & 0xe00000) >> 18) + 
			       ((b & 0xe00000) >> 21));
  (*output)++;
}

static inline void transfer_RGBA16161616_to_BGR565(BYTE *(*output), WORD *input)
{
	unsigned int r, g, b, a;
	a = (input)[3] >> 8;
	r = (unsigned int)(input)[0] * a;
	g = (unsigned int)(input)[1] * a;
	b = (unsigned int)(input)[2] * a;

	*(WORD*)(*output) = (WORD)(((b & 0xf80000) >> 8) + 
				((g & 0xfc0000) >> 13) + 
				((r & 0xf80000) >> 19));
	(*output) += 2;
}

static inline void transfer_RGBA16161616_to_RGB565(BYTE *(*output), WORD *input)
{
  unsigned int r, g, b, a;
  a = (input)[3] >> 8;
  r = (unsigned int)(input)[0] * a;
  g = (unsigned int)(input)[1] * a;
  b = (unsigned int)(input)[2] * a;

  *(WORD*)(*output) = (WORD)(((r & 0xf80000) >> 8) + 
				     ((g & 0xfc0000) >> 13) + 
				     ((b & 0xf80000) >> 19));
  (*output) += 2;
}

static inline void transfer_RGBA16161616_to_BGR888(BYTE *(*output), WORD *input)
{
  unsigned int r, g, b, a;
  a = (input)[3] >> 8;
  r = (unsigned int)(input)[0] * a;
  g = (unsigned int)(input)[1] * a;
  b = (unsigned int)(input)[2] * a;

  (*output)[0] = (BYTE)(b >> 16);
  (*output)[1] = (BYTE)(g >> 16);
  (*output)[2] = (BYTE)(r >> 16);
  (*output) += 3;
}

static inline void transfer_RGBA16161616_to_RGB888(BYTE *(*output), WORD *input)
{
  unsigned int r, g, b, a;
  a = (input)[3] >> 8;
  r = (unsigned int)(input)[0] * a;
  g = (unsigned int)(input)[1] * a;
  b = (unsigned int)(input)[2] * a;

  (*output)[0] = (BYTE)(r >> 16);
  (*output)[1] = (BYTE)(g >> 16);
  (*output)[2] = (BYTE)(b >> 16);
  (*output) += 3;
}


static inline void transfer_RGBA16161616_to_RGBA8888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[0] >> 8;
  (*output)[1] = input[1] >> 8;
  (*output)[2] = input[2] >> 8;
  (*output)[3] = input[3] >> 8;
  (*output) += 4;
}


static inline void transfer_RGBA16161616_to_BGR8888(BYTE *(*output), WORD *input)
{
  unsigned int r, g, b, a;
  a = (input)[3] >> 8;
  r = (input)[0] * a;
  g = (input)[1] * a;
  b = (input)[2] * a;

  (*output)[0] = (BYTE)(b >> 16);
  (*output)[1] = (BYTE)(g >> 16);
  (*output)[2] = (BYTE)(r >> 16);
  (*output) += 4;
}

static inline void transfer_RGBA16161616_to_YUV101010(BYTE *(*output), WORD *input)
{
  int r, g, b;
  int y, u, v;

  r = (((ULONG)input[0] * input[3]) >> 16) + 0x1;
  g = (((ULONG)input[1] * input[3]) >> 16) + 0x1;
  b = (((ULONG)input[2] * input[3]) >> 16) + 0x1;
  RGB_TO_YUV16(y, u, v, r, g, b);
  WRITE_YUV101010(y, u, v);
}


static inline void transfer_RGBA16161616_to_YUV420P_YUV422P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	WORD *input,
	int output_column)
{
	int y, u, v, r, g, b;
	__int64 a;
	a = input[3];
	r = (int)((__int64)input[0] * a / 0xffffff);
	g = (int)((__int64)input[1] * a / 0xffffff);
	b = (int)((__int64)input[2] * a / 0xffffff);

	RGB_TO_YUV(y, u, v, r, g, b);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGBA16161616_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	WORD *input,
	int output_column)
{
	int y, u, v, r, g, b;
	__int64 a;
	a = input[3];
	r = (int)((__int64)input[0] * a / 0xffffff);
	g = (int)((__int64)input[1] * a / 0xffffff);
	b = (int)((__int64)input[2] * a / 0xffffff);

	RGB_TO_YUV(y, u, v, r, g, b);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

















static inline void transfer_BGR8888_to_RGB888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[2];
  (*output)[1] = input[1];
  (*output)[2] = input[0];
  (*output) += 3;
}

static inline void transfer_BGR8888_to_BGR8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output) += 4;
}

static inline void transfer_BGR888_to_RGB888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[2];
  (*output)[1] = input[1];
  (*output)[2] = input[0];
  (*output) += 3;
}







// ******************************** YUV888 -> *********************************


static inline void transfer_YUV888_to_RGB8(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);

  *(*output) = (BYTE)((r & 0xc0) +
			       ((g & 0xe0) >> 2) +
			       ((b & 0xe0) >> 5));
  (*output)++;
}

static inline void transfer_YUV888_to_BGR565(BYTE *(*output), BYTE *input)
{
	int y, u, v;
	int r, g, b;
	
	y = ((int)input[0]) << 16;
	u = input[1];
	v = input[2];
	YUV_TO_RGB(y, u, v, r, g, b);
	*(WORD*)(*output) = ((b & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((r & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_YUV888_to_RGB565(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
  *(WORD*)(*output) = ((r & 0xf8) << 8)
    + ((g & 0xfc) << 3)
    + ((b & 0xf8) >> 3);
  (*output) += 2;
}

static inline void transfer_YUV888_to_BGR888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);

  (*output)[2] = r;
  (*output)[1] = g;
  (*output)[0] = b;
  (*output) += 3;
}

static inline void transfer_YUV888_to_BGR8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;

  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
  (*output)[2] = r;
  (*output)[1] = g;
  (*output)[0] = b;
  (*output) += 4;
}

static inline void transfer_YUV888_to_RGB888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output) += 3;
}

static inline void transfer_YUV888_to_RGBA8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;

  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output)[3] = 0xff;
  (*output) += 4;
}

static inline void transfer_YUV888_to_YUVA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = (int)input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output)[3] = 0xff;
  (*output) += 4;
}

static inline void transfer_YUV888_to_YUV888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = (int)input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output) += 3;
}


static inline void transfer_YUV888_to_VYU888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[2];
  (*output)[1] = input[0];
  (*output)[2] = input[1];
  (*output) += 3;
}


static inline void transfer_YUV888_to_UYVA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[1];
  (*output)[1] = input[0];
  (*output)[2] = input[2];
  (*output)[3] = 0xff;
  (*output) += 4;
}


static inline void transfer_YUV888_to_YUV101010(BYTE *(*output), BYTE *input)
{
  WORD y_i = ((WORD)input[0]) << 8;
  WORD u_i = ((WORD)input[1]) << 8;
  WORD v_i = ((WORD)input[2]) << 8;
  WRITE_YUV101010(y_i, u_i, v_i);
}

static inline void transfer_YUV888_to_YUV420P_YUV422P(BYTE *output_y, 
						      BYTE *output_u, 
						      BYTE *output_v, 
						      BYTE *input,
						      int output_column)
{
  output_y[output_column] = input[0];
  output_u[output_column / 2] = input[1];
  output_v[output_column / 2] = input[2];
}

static inline void transfer_YUV888_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	BYTE *input,
	int output_column)
{
	output_y[output_column] = input[0];
	output_u[output_column] = input[1];
	output_v[output_column] = input[2];
}

static inline void transfer_YUV888_to_YUV422(BYTE *(*output), 
					     BYTE *input,
					     int j)
{
  // Store U and V for even pixels only
  if(!(j & 1))
    {
      (*output)[1] = input[1];
      (*output)[3] = input[2];
      (*output)[0] = input[0];
    }
  else
    // Store Y and advance output for odd pixels only
    {
      (*output)[2] = input[0];
      (*output) += 4;
    }
}






// ******************************** YUVA8888 -> *******************************




static inline void transfer_YUVA8888_to_RGB8(BYTE *(*output), BYTE *input)
{
  int y, u, v, a;
  int r, g, b;
	
  a = input[3];
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
	
  r *= a;
  g *= a;
  b *= a;

  *(*output) = (BYTE)(((r & 0xc000) >> 8) + 
			       ((g & 0xe000) >> 10) + 
			       ((b & 0xe000) >> 13));
  (*output)++;
}

static inline void transfer_YUVA8888_to_BGR565(BYTE *(*output), BYTE *input)
{
	int y, u, v, a;
	int r, g, b;
	
	a = input[3];
	y = ((int)input[0]) << 16;
	u = input[1];
	v = input[2];
	YUV_TO_RGB(y, u, v, r, g, b);
		
	r *= a;
	g *= a;
	b *= a;

	*(WORD*)(*output) = (WORD)((b & 0xf800) + 
				((g & 0xfc00) >> 5) + 
				((r & 0xf800) >> 11));
	(*output) += 2;
}

static inline void transfer_YUVA8888_to_RGB565(BYTE *(*output), BYTE *input)
{
  int y, u, v, a;
  int r, g, b;
	
  a = input[3];
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
		
  r *= a;
  g *= a;
  b *= a;

  *(WORD*)(*output) = (WORD)((r & 0xf800) + 
				     ((g & 0xfc00) >> 5) + 
				     ((b & 0xf800) >> 11));
  (*output) += 2;
}

static inline void transfer_YUVA8888_to_BGR888(BYTE *(*output), BYTE *input)
{
  int y, u, v, a;
  int r, g, b;
	
  a = input[3];
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];

  YUV_TO_RGB(y, u, v, r, g, b);
		
  r *= a;
  g *= a;
  b *= a;

  (*output)[0] = (b >> 8) + 1;
  (*output)[1] = (g >> 8) + 1;
  (*output)[2] = (r >> 8) + 1;
  (*output) += 3;
}

static inline void transfer_YUVA8888_to_RGB888(BYTE *(*output), BYTE *input)
{
  int y, u, v, a;
  int r, g, b;
	
  a = input[3];
  y = ((int)input[0]) << 16;
  u = input[1];
  v = input[2];

  YUV_TO_RGB(y, u, v, r, g, b);
		
  r *= a;
  g *= a;
  b *= a;

  (*output)[0] = (r >> 8) + 1;
  (*output)[1] = (g >> 8) + 1;
  (*output)[2] = (b >> 8) + 1;
  (*output) += 3;
}

static inline void transfer_YUVA8888_to_RGBA8888(BYTE *(*output), BYTE *input)
{
	int y, u, v, a;
	int r, g, b;

	a = input[3];
	y = ((int)input[0]) << 16;
	u = input[1];
	v = input[2];

	YUV_TO_RGB(y, u, v, r, g, b);
	(*output)[0] = r;
	(*output)[1] = g;
	(*output)[2] = b;
	(*output)[3] = a;
	(*output) += 4;
}

static inline void transfer_YUVA8888_to_BGR8888(BYTE *(*output), BYTE *input)
{
	int y, u, v, a;
	int r, g, b;

	a = input[3];
	y = ((int)input[0]) << 16;
	u = input[1];
	v = input[2];

	YUV_TO_RGB(y, u, v, r, g, b);
	
	r *= a;
	g *= a;
	b *= a;
	(*output)[0] = (b >> 8) + 1;
	(*output)[1] = (g >> 8) + 1;
	(*output)[2] = (r >> 8) + 1;
	(*output) += 4;
}


static inline void transfer_YUVA8888_to_VYU888(BYTE *(*output), BYTE *input)
{
  int y, u, v, a, anti_a;
  a = input[3];
  anti_a = 0xff - a;
  y = ((ULONG)input[0] * a) / 0xff;
  u = ((ULONG)input[1] * a + 0x80 * anti_a) / 0xff;
  v = ((ULONG)input[2] * a + 0x80 * anti_a) / 0xff;
	
  (*output)[0] = v;
  (*output)[1] = y;
  (*output)[2] = u;
  (*output) += 3;
}


static inline void transfer_YUVA8888_to_YUVA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output)[3] = input[3];
  (*output) += 4;
}

static inline void transfer_YUVA8888_to_UYVA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[1];
  (*output)[1] = input[0];
  (*output)[2] = input[2];
  (*output)[3] = input[3];
  (*output) += 4;
}

static inline void transfer_YUVA8888_to_YUV101010(BYTE *(*output), BYTE *input)
{
  WORD y_i = ((WORD)input[0] * input[3]) + 0x1fe;
  WORD u_i = ((WORD)input[1] * input[3]) + 0x1fe;
  WORD v_i = ((WORD)input[2] * input[3]) + 0x1fe;
  WRITE_YUV101010(y_i, u_i, v_i);
}


static inline void transfer_YUVA8888_to_YUV420P_YUV422P(BYTE *output_y, 
							BYTE *output_u, 
							BYTE *output_v, 
							BYTE *input,
							int output_column)
{
	int opacity = input[3];
	int transparency = 0xff - opacity;

	output_y[output_column] =     ((input[0] * opacity) >> 8) + 1;
	output_u[output_column / 2] = ((input[1] * opacity + 0x80 * transparency) >> 8) + 1;
	output_v[output_column / 2] = ((input[2] * opacity + 0x80 * transparency) >> 8) + 1;
}

static inline void transfer_YUVA8888_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	BYTE *input,
	int output_column)
{
	int opacity = input[3];
	int transparency = 0xff - opacity;

	output_y[output_column] =     ((input[0] * opacity) >> 8) + 1;
	output_u[output_column] = ((input[1] * opacity + 0x80 * transparency) >> 8) + 1;
	output_v[output_column] = ((input[2] * opacity + 0x80 * transparency) >> 8) + 1;
}

static inline void transfer_YUVA8888_to_YUV422(BYTE *(*output), 
					       BYTE *input,
					       int j)
{
	int opacity = input[3];
	int transparency = 0xff - opacity;
// Store U and V for even pixels only
	if(!(j & 1))
	{
		(*output)[0] = ((input[0] * opacity) >> 8) + 1;
		(*output)[1] = ((input[1] * opacity + 0x80 * transparency) >> 8) + 1;
		(*output)[3] = ((input[2] * opacity + 0x80 * transparency) >> 8) + 1;
	}
	else
// Store Y and advance output for odd pixels only
	{
		(*output)[2] = ((input[0] * opacity) >> 8) + 1;
		(*output) += 4;
	}
}











// ********************************* YUV101010 -> *****************************

#define READ_YUV101010 \
	unsigned int y, u, v; \
	ULONG input_i = input[0] | \
		(input[1] << 8) | \
		(input[2] << 16) | \
		(input[3] << 24); \
 \
	y = (input_i & 0xffc00000) >> 16; \
	u = (input_i & 0x3ff000) >> 6; \
	v = (input_i & 0xffc) << 4;

static inline void transfer_YUV101010_to_RGB8(BYTE *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;

  YUV_TO_RGB16(y, u, v, r, g, b);

  *(*output)++ = (BYTE)(((input[0] & 0xc000) >> 8) +
				 ((input[1] & 0xe000) >> 10) +
				 ((input[2] & 0xe000) >> 13));
}

static inline void transfer_YUV101010_to_BGR565(BYTE *(*output), BYTE *input)
{
	int r, g, b;

	READ_YUV101010

	y <<= 8;

	YUV_TO_RGB16(y, u, v, r, g, b);

	*(WORD*)(*output) = (b & 0xf800) |
			 ((g & 0xfc00) >> 5) |
			 ((r & 0xf800) >> 11);
	(*output) += 2;
}

static inline void transfer_YUV101010_to_RGB565(BYTE *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;

  YUV_TO_RGB16(y, u, v, r, g, b);

  *(WORD*)(*output) = (r & 0xf800) |
    ((g & 0xfc00) >> 5) |
    ((b & 0xf800) >> 11);
  (*output) += 2;
}

static inline void transfer_YUV101010_to_BGR888(BYTE *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;

  YUV_TO_RGB16(y, u, v, r, g, b);

  *(*output)++ = b >> 8;
  *(*output)++ = g >> 8;
  *(*output)++ = r >> 8;
}

static inline void transfer_YUV101010_to_BGR8888(BYTE *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;

  YUV_TO_RGB16(y, u, v, r, g, b);

  *(*output)++ = b >> 8;
  *(*output)++ = g >> 8;
  *(*output)++ = r >> 8;
  (*output)++;
}

static inline void transfer_YUV101010_to_YUV888(BYTE *(*output), BYTE *input)
{
  READ_YUV101010
	 
    *(*output)++ = y >> 8;
  *(*output)++ = u >> 8;
  *(*output)++ = v >> 8;
}

static inline void transfer_YUV101010_to_YUVA8888(BYTE *(*output), BYTE *input)
{
  READ_YUV101010
	 
    *(*output)++ = y >> 8;
  *(*output)++ = u >> 8;
  *(*output)++ = v >> 8;
  *(*output)++ = 0xff;
}

static inline void transfer_YUV101010_to_YUV161616(WORD *(*output), BYTE *input)
{
  READ_YUV101010
	 
    *(*output)++ = y;
  *(*output)++ = u;
  *(*output)++ = v;
}

static inline void transfer_YUV101010_to_YUVA16161616(WORD *(*output), BYTE *input)
{
  READ_YUV101010
	 
    *(*output)++ = y;
  *(*output)++ = u;
  *(*output)++ = v;
  *(*output)++ = 0xffff;
}

static inline void transfer_YUV101010_to_RGB888(BYTE *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;

  YUV_TO_RGB16(y, u, v, r, g, b);

  *(*output)++ = r >> 8;
  *(*output)++ = g >> 8;
  *(*output)++ = b >> 8;
}

static inline void transfer_YUV101010_to_RGBA8888(BYTE *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;
  
  YUV_TO_RGB16(y, u, v, r, g, b);

  *(*output)++ = r >> 8;
  *(*output)++ = g >> 8;
  *(*output)++ = b >> 8;
  *(*output)++ = 0xff;
}

static inline void transfer_YUV101010_to_RGB161616(WORD *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;

  YUV_TO_RGB16(y, u, v, r, g, b);

  *(*output)++ = r;
  *(*output)++ = g;
  *(*output)++ = b;
}

static inline void transfer_YUV101010_to_RGBA16161616(WORD *(*output), BYTE *input)
{
  int r, g, b;

  READ_YUV101010

    y <<= 8;

  YUV_TO_RGB16(y, u, v, r, g, b);

  *(*output)++ = r;
  *(*output)++ = g;
  *(*output)++ = b;
  *(*output)++ = 0xffff;
}









// ******************************** YUV161616 -> ******************************


static inline void transfer_YUV161616_to_RGB8(BYTE *(*output), WORD *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 8;
  u = input[1] >> 8;
  v = input[2] >> 8;
  YUV_TO_RGB(y, u, v, r, g, b);

  *(*output) = (BYTE)((r & 0xc0) +
			       ((g & 0xe0) >> 2) +
			       ((b & 0xe0) >> 5));
  (*output)++;
}

static inline void transfer_YUV161616_to_BGR565(BYTE *(*output), WORD *input)
{
	int y, u, v;
	int r, g, b;
	
	y = ((int)input[0]) << 8;
	u = input[1] >> 8;
	v = input[2] >> 8;
	YUV_TO_RGB(y, u, v, r, g, b);
	*(WORD*)(*output) = ((b & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((r & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_YUV161616_to_RGB565(BYTE *(*output), WORD *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 8;
  u = input[1] >> 8;
  v = input[2] >> 8;
  YUV_TO_RGB(y, u, v, r, g, b);
  *(WORD*)(*output) = ((r & 0xf8) << 8)
    + ((g & 0xfc) << 3)
    + ((b & 0xf8) >> 3);
  (*output) += 2;
}

static inline void transfer_YUV161616_to_BGR888(BYTE *(*output), WORD *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 8;
  u = input[1];
  v = input[2];
  YUV_TO_RGB16(y, u, v, r, g, b);

  (*output)[2] = r >> 8;
  (*output)[1] = g >> 8;
  (*output)[0] = b >> 8;
  (*output) += 3;
}

static inline void transfer_YUV161616_to_RGB888(BYTE *(*output), WORD *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 8;
  u = input[1];
  v = input[2];
  YUV_TO_RGB16(y, u, v, r, g, b);

  (*output)[0] = r >> 8;
  (*output)[1] = g >> 8;
  (*output)[2] = b >> 8;
  (*output) += 3;
}

static inline void transfer_YUV161616_to_RGBA8888(BYTE *(*output), WORD *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[0]) << 8;
  u = input[1];
  v = input[2];
  YUV_TO_RGB16(y, u, v, r, g, b);

  (*output)[0] = r >> 8;
  (*output)[1] = g >> 8;
  (*output)[2] = b >> 8;
  (*output)[3] = 0xff;
  (*output) += 4;
}

static inline void transfer_YUV161616_to_BGR8888(BYTE *(*output), WORD *input)
{
  int y, u, v;
  int r, g, b;

  y = ((int)input[0]) << 8;
  u = input[1] >> 8;
  v = input[2] >> 8;
  YUV_TO_RGB(y, u, v, r, g, b);
  (*output)[2] = r;
  (*output)[1] = g;
  (*output)[0] = b;
  (*output) += 4;
}

static inline void transfer_YUV161616_to_YUV161616(WORD *(*output), WORD *input)
{
  (*output)[0] = input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output) += 3;
}

static inline void transfer_YUV161616_to_YUVA8888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[0] >> 8;
  (*output)[1] = input[1] >> 8;
  (*output)[2] = input[2] >> 8;
  (*output)[3] = 255;
  (*output) += 4;
}


static inline void transfer_YUV161616_to_VYU888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[2] >> 8;
  (*output)[1] = input[0] >> 8;
  (*output)[2] = input[1] >> 8;
  (*output) += 3;
}


static inline void transfer_YUV161616_to_UYVA8888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[1] >> 8;
  (*output)[1] = input[0] >> 8;
  (*output)[2] = input[2] >> 8;
  (*output)[3] = input[3] >> 8;
  (*output) += 4;
}

static inline void transfer_YUV161616_to_YUV101010(BYTE *(*output), WORD *input)
{
  WORD y_i = input[0];
  WORD u_i = input[1];
  WORD v_i = input[2];
  WRITE_YUV101010(y_i, u_i, v_i);
}

static inline void transfer_YUV161616_to_YUV420P_YUV422P(BYTE *output_y, 
							 BYTE *output_u, 
							 BYTE *output_v, 
							 WORD *input,
							 int output_column)
{
  output_y[output_column] = input[0] >> 8;
  output_u[output_column / 2] = input[1] >> 8;
  output_v[output_column / 2] = input[2] >> 8;
}

static inline void transfer_YUV161616_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	WORD *input,
	int output_column)
{
	output_y[output_column] = input[0] >> 8;
	output_u[output_column] = input[1] >> 8;
	output_v[output_column] = input[2] >> 8;
}

static inline void transfer_YUV161616_to_YUV422(BYTE *(*output), 
						WORD *input,
						int j)
{
  // Store U and V for even pixels only
  if(!(j & 1))
    {
      (*output)[1] = input[1] >> 8;
      (*output)[3] = input[2] >> 8;
      (*output)[0] = input[0] >> 8;
    }
  else
    // Store Y and advance output for odd pixels only
    {
      (*output)[2] = input[0] >> 8;
      (*output) += 4;
    }
}











// ******************************** YUVA16161616 -> ***************************




static inline void transfer_YUVA16161616_to_RGB8(BYTE *(*output), WORD *input)
{
  int y, u, v, a;
  int r, g, b;
	
  a = input[3];
  y = ((int)input[0]) << 8;
  u = input[1] >> 8;
  v = input[2] >> 8;
  YUV_TO_RGB(y, u, v, r, g, b);
	
  r *= a;
  g *= a;
  b *= a;

  *(*output) = (BYTE)(((r & 0xc000) >> 8) + 
			       ((g & 0xe000) >> 10) + 
			       ((b & 0xe000) >> 13));
  (*output)++;
}

static inline void transfer_YUVA16161616_to_BGR565(BYTE *(*output), WORD *input)
{
	int y, u, v, a;
	int r, g, b;
	
	a = input[3] >> 8;
	y = ((int)input[0]) << 8;
	u = input[1] >> 8;
	v = input[2] >> 8;
	YUV_TO_RGB(y, u, v, r, g, b);
		
	r *= a;
	g *= a;
	b *= a;

	*(WORD*)(*output) = (WORD)((b & 0xf800) + 
				((g & 0xfc00) >> 5) + 
				((r & 0xf800) >> 11));
	(*output) += 2;
}

static inline void transfer_YUVA16161616_to_RGB565(BYTE *(*output), WORD *input)
{
	int y, u, v, a;
	int r, g, b;
	
	a = input[3] >> 8;
	y = ((int)input[0]) << 8;
	u = input[1] >> 8;
	v = input[2] >> 8;
	YUV_TO_RGB(y, u, v, r, g, b);
		
	r *= a;
	g *= a;
	b *= a;

	*(WORD*)(*output) = (WORD)((r & 0xf800) + 
				((g & 0xfc00) >> 5) + 
				((b & 0xf800) >> 11));
	(*output) += 2;
}

static inline void transfer_YUVA16161616_to_BGR888(BYTE *(*output), WORD *input)
{
	int y, u, v, a;
	int r, g, b;
	
	a = input[3];
	y = ((int)input[0]) << 8;
	u = input[1];
	v = input[2];

	YUV_TO_RGB16(y, u, v, r, g, b);
		
	r *= a;
	g *= a;
	b *= a;

	(*output)[0] = b / 0xffff00;
	(*output)[1] = g / 0xffff00;
	(*output)[2] = r / 0xffff00;
	(*output) += 3;
}

static inline void transfer_YUVA16161616_to_RGB888(BYTE *(*output), WORD *input)
{
	unsigned int y, u, v, a;
	unsigned int r, g, b;
	
	a = input[3];
	y = ((int)input[0]) << 8;
	u = input[1];
	v = input[2];

	YUV_TO_RGB16(y, u, v, r, g, b);
		
	r *= a;
	g *= a;
	b *= a;

	(*output)[0] = r / 0xffff00;
	(*output)[1] = g / 0xffff00;
	(*output)[2] = b / 0xffff00;
	(*output) += 3;
}

static inline void transfer_YUVA16161616_to_RGBA8888(BYTE *(*output), WORD *input)
{
	unsigned int y, u, v;
	unsigned int r, g, b;
	
	y = ((int)input[0]) << 8;
	u = input[1];
	v = input[2];

	YUV_TO_RGB16(y, u, v, r, g, b);

	(*output)[0] = (r >> 8);
	(*output)[1] = (g >> 8);
	(*output)[2] = (b >> 8);
	(*output)[3] = (BYTE)input[3];
	(*output) += 4;
}

static inline void transfer_YUVA16161616_to_BGR8888(BYTE *(*output), WORD *input)
{
	int y, u, v, a;
	__int64 r, g, b;

	a = input[3];
	y = ((int)input[0]) << 8;
	u = input[1] >> 8;
	v = input[2] >> 8;

	YUV_TO_RGB(y, u, v, r, g, b);

	r *= a;
	g *= a;
	b *= a;

	(*output)[0] = (BYTE)(b / 0xffff);
	(*output)[1] = (BYTE)(g / 0xffff);
	(*output)[2] = (BYTE)(r / 0xffff);
	(*output) += 4;
}


static inline void transfer_YUVA16161616_to_VYU888(BYTE *(*output), WORD *input)
{
	int y, u, v, a, anti_a;
	a = input[3];
	anti_a = 0xffff - a;
	y = ((ULONG)input[0] * a) / 0xffff00;
	u = ((ULONG)input[1] * a + 0x8000 * anti_a) / 0xffff00;
	v = ((ULONG)input[2] * a + 0x8000 * anti_a) / 0xffff00;

	(*output)[0] = v;
	(*output)[1] = y;
	(*output)[2] = u;
	(*output) += 3;
}


static inline void transfer_YUVA16161616_to_YUVA16161616(WORD *(*output), WORD *input)
{
  (*output)[0] = input[0];
  (*output)[1] = input[1];
  (*output)[2] = input[2];
  (*output)[3] = input[3];
  (*output) += 4;
}

static inline void transfer_YUVA16161616_to_UYVA8888(BYTE *(*output), WORD *input)
{
  (*output)[0] = input[1] >> 8;
  (*output)[1] = input[0] >> 8;
  (*output)[2] = input[2] >> 8;
  (*output)[3] = input[3] >> 8;
  (*output) += 4;
}


static inline void transfer_YUVA16161616_to_YUV101010(BYTE *(*output), WORD *input)
{
	__int64 opacity = input[3];
	__int64 transparency = 0xffff - opacity;
	WORD y_i = (WORD)(((__int64)input[0] * opacity + 0x8000 * transparency) / 0xffff00);
	WORD u_i = (WORD)(((__int64)input[1] * opacity + 0x8000 * transparency) / 0xffff00);
	WORD v_i = (WORD)(((__int64)input[2] * opacity + 0x8000 * transparency) / 0xffff00);
	WRITE_YUV101010(y_i, u_i, v_i);
}

static inline void transfer_YUVA16161616_to_YUV420P_YUV422P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	WORD *input,
	int output_column)
{
	__int64 opacity = input[3];
	__int64 transparency = 0xffff - opacity;

	output_y[output_column] =     (BYTE)(((__int64)input[0] * opacity) / 0xffff00);
	output_u[output_column / 2] = (BYTE)(((__int64)input[1] * opacity + 0x8000 * transparency) / 0xffff00);
	output_v[output_column / 2] = (BYTE)(((__int64)input[2] * opacity + 0x8000 * transparency) / 0xffff00);
}

static inline void transfer_YUVA16161616_to_YUV444P(BYTE *output_y, 
	BYTE *output_u, 
	BYTE *output_v, 
	WORD *input,
	int output_column)
{
	__int64 opacity = input[3];
	__int64 transparency = 0xffff - opacity;

	output_y[output_column] = (BYTE)(((__int64)input[0] * opacity) / 0xffff00);
	output_u[output_column] = (BYTE)(((__int64)input[1] * opacity + 0x8000 * transparency) / 0xffff00);
	output_v[output_column] = (BYTE)(((__int64)input[2] * opacity + 0x8000 * transparency) / 0xffff00);
}

static inline void transfer_YUVA16161616_to_YUV422(BYTE *(*output), 
						   WORD *input,
						   int j)
{
	__int64 opacity = input[3];
	__int64 transparency = 0xffff - opacity;

// Store U and V for even pixels only
	if(!(j & 1))
	{
		(*output)[0] = (BYTE)(((__int64)input[0] * opacity) / 0xffff00);
		(*output)[1] = (BYTE)(((__int64)input[1] * opacity + 0x8000 * transparency) / 0xffff00);
		(*output)[3] = (BYTE)(((__int64)input[2] * opacity + 0x8000 * transparency) / 0xffff00);
	}
	else
// Store Y and advance output for odd pixels only
	{
		(*output)[2] = (BYTE)((input[0] * opacity) / 0xffff00);
		(*output) += 4;
	}
}
















// ******************************** VYU888 -> *********************************


static inline void transfer_VYU888_to_RGB8(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[2];
  v = input[0];
  YUV_TO_RGB(y, u, v, r, g, b);

  *(*output) = (BYTE)((r & 0xc0) +
			       ((g & 0xe0) >> 2) +
			       ((b & 0xe0) >> 5));
  (*output)++;
}

static inline void transfer_VYU888_to_BGR565(BYTE *(*output), BYTE *input)
{
	int y, u, v;
	int r, g, b;
	
	y = ((int)input[1]) << 16;
	u = input[2];
	v = input[0];
	YUV_TO_RGB(y, u, v, r, g, b);
	*(WORD*)(*output) = ((b & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((r & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_VYU888_to_RGB565(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[2];
  v = input[0];
  YUV_TO_RGB(y, u, v, r, g, b);
  *(WORD*)(*output) = ((r & 0xf8) << 8)
    + ((g & 0xfc) << 3)
    + ((b & 0xf8) >> 3);
  (*output) += 2;
}

static inline void transfer_VYU888_to_BGR888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[2];
  v = input[0];
  YUV_TO_RGB(y, u, v, r, g, b);

  (*output)[2] = r;
  (*output)[1] = g;
  (*output)[0] = b;
  (*output) += 3;
}

static inline void transfer_VYU888_to_BGR8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;

  y = ((int)input[1]) << 16;
  u = input[2];
  v = input[0];
  YUV_TO_RGB(y, u, v, r, g, b);
  (*output)[2] = r;
  (*output)[1] = g;
  (*output)[0] = b;
  (*output) += 4;
}


static inline void transfer_VYU888_to_RGB888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[2];
  v = input[0];
  YUV_TO_RGB(y, u, v, r, g, b);

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output) += 3;
}

static inline void transfer_VYU888_to_RGBA8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[2];
  v = input[0];
  YUV_TO_RGB(y, u, v, r, g, b);

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output)[3] = 0xff;
  (*output) += 4;
}


static inline void transfer_VYU888_to_RGB161616(WORD *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[2] << 8;
  v = input[0] << 8;
  YUV_TO_RGB16(y, u, v, r, g, b);

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output) += 3;
}

static inline void transfer_VYU888_to_RGBA16161616(WORD *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;

  y = ((int)input[1]) << 16;
  u = input[2] << 8;
  v = input[0] << 8;
  YUV_TO_RGB16(y, u, v, r, g, b);

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output)[3] = 0xffff;
  (*output) += 3;
}


static inline void transfer_VYU888_to_YUV888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[1];
  (*output)[1] = input[2];
  (*output)[2] = input[0];
  (*output) += 3;
}

static inline void transfer_VYU888_to_YUVA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[1];
  (*output)[1] = input[2];
  (*output)[2] = input[0];
  (*output)[3] = 0xff;
  (*output) += 4;
}


static inline void transfer_VYU888_to_YUV161616(WORD *(*output), BYTE *input)
{
  (*output)[0] = ((int)input[1]) << 8;
  (*output)[1] = ((int)input[2]) << 8;
  (*output)[2] = ((int)input[0]) << 8;
  (*output) += 3;
}

static inline void transfer_VYU888_to_YUVA16161616(WORD *(*output), BYTE *input)
{
  (*output)[0] = ((int)input[1]) << 8;
  (*output)[1] = ((int)input[2]) << 8;
  (*output)[2] = ((int)input[0]) << 8;
  (*output)[3] = 0xff;
  (*output) += 4;
}














// ******************************** UYVA8888 -> *********************************


static inline void transfer_UYVA8888_to_RGB8(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;

  y = ((int)input[1]) << 16;
  u = input[0];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);

  r = r * input[3] / 0xff;
  g = g * input[3] / 0xff;
  b = b * input[3] / 0xff;
  *(*output) = (BYTE)((r & 0xc0) +
			       ((g & 0xe0) >> 2) +
			       ((b & 0xe0) >> 5));
  (*output)++;
}

static inline void transfer_UYVA8888_to_BGR565(BYTE *(*output), BYTE *input)
{
	int y, u, v;
	int r, g, b;
	
	y = ((int)input[1]) << 16;
	u = input[0];
	v = input[2];
	YUV_TO_RGB(y, u, v, r, g, b);
	r = r * input[3] / 0xff;
	g = g * input[3] / 0xff;
	b = b * input[3] / 0xff;
	*(WORD*)(*output) = ((b & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((r & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_UYVA8888_to_RGB565(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[0];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
  r = r * input[3] / 0xff;
  g = g * input[3] / 0xff;
  b = b * input[3] / 0xff;
  *(WORD*)(*output) = ((r & 0xf8) << 8)
    + ((g & 0xfc) << 3)
    + ((b & 0xf8) >> 3);
  (*output) += 2;
}

static inline void transfer_UYVA8888_to_BGR888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[0];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
  r = r * input[3] / 0xff;
  g = g * input[3] / 0xff;
  b = b * input[3] / 0xff;

  (*output)[2] = r;
  (*output)[1] = g;
  (*output)[0] = b;
  (*output) += 3;
}

static inline void transfer_UYVA8888_to_BGR8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[0];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
  r = r * input[3] / 0xff;
  g = g * input[3] / 0xff;
  b = b * input[3] / 0xff;

  (*output)[2] = r;
  (*output)[1] = g;
  (*output)[0] = b;
  (*output) += 4;
}


static inline void transfer_UYVA8888_to_RGB888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[0];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);
  r = r * input[3] / 0xff;
  g = g * input[3] / 0xff;
  b = b * input[3] / 0xff;

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output) += 3;
}

static inline void transfer_UYVA8888_to_RGBA8888(BYTE *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[0];
  v = input[2];
  YUV_TO_RGB(y, u, v, r, g, b);

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output)[3] = input[3];
  (*output) += 4;
}


static inline void transfer_UYVA8888_to_RGB161616(WORD *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;
	
  y = ((int)input[1]) << 16;
  u = input[0] << 8;
  v = input[2] << 8;
  YUV_TO_RGB16(y, u, v, r, g, b);
  r = r * input[3] / 0xff;
  g = g * input[3] / 0xff;
  b = b * input[3] / 0xff;

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output) += 3;
}

static inline void transfer_UYVA8888_to_RGBA16161616(WORD *(*output), BYTE *input)
{
  int y, u, v;
  int r, g, b;

  y = ((int)input[1]) << 16;
  u = input[0] << 8;
  v = input[2] << 8;
  YUV_TO_RGB16(y, u, v, r, g, b);

  (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output)[3] = input[3] << 8;
  (*output) += 4;
}


static inline void transfer_UYVA8888_to_YUV888(BYTE *(*output), BYTE *input)
{
  int a, anti_a;
  a = input[3];
  anti_a = 0xff - a;

  (*output)[0] = (a * input[1]) / 0xff;
  (*output)[1] = (a * input[0] + anti_a * 0x80) / 0xff;
  (*output)[2] = (a * input[2] + anti_a * 0x80) / 0xff;
  (*output) += 3;
}

static inline void transfer_UYVA8888_to_YUVA8888(BYTE *(*output), BYTE *input)
{
  (*output)[0] = input[1];
  (*output)[1] = input[0];
  (*output)[2] = input[2];
  (*output)[3] = input[3];
  (*output) += 4;
}


static inline void transfer_UYVA8888_to_YUV161616(WORD *(*output), BYTE *input)
{
  int a, anti_a;
  a = input[3];
  anti_a = 0xff - a;

  (*output)[0] = a * input[1];
  (*output)[1] = a * input[0] + anti_a * 0x80;
  (*output)[2] = a * input[2] + anti_a * 0x80;
  (*output) += 3;
}

static inline void transfer_UYVA8888_to_YUVA16161616(WORD *(*output), BYTE *input)
{
  (*output)[0] = input[1] << 8;
  (*output)[1] = input[0] << 8;
  (*output)[2] = input[2] << 8;
  (*output)[3] = input[3] << 8;
  (*output) += 4;
}















// ******************************** YUV422P -> ********************************

static inline void transfer_YUV422P_to_RGB8(BYTE *(*output), 
					    BYTE *input_y,
					    BYTE *input_u,
					    BYTE *input_v)
{
  int y, u, v;
  int r, g, b;
	
  y = (int)(*input_y) << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    *(*output) = (BYTE)((r & 0xc0) +
				 ((g & 0xe0) >> 2) +
				 ((b & 0xe0) >> 5));
  (*output)++;
}

static inline void transfer_YUV422P_to_BGR565(BYTE *(*output), 
	BYTE *input_y,
	BYTE *input_u,
	BYTE *input_v)
{
	int y, u, v;
	int r, g, b;
	
	y = (int)(*input_y) << 16;
	u = *input_u;
	v = *input_v;
	YUV_TO_RGB(y, u, v, r, g, b)

	*(WORD*)(*output) = ((b & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((r & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_YUV422P_to_RGB565(BYTE *(*output), 
					      BYTE *input_y,
					      BYTE *input_u,
					      BYTE *input_v)
{
  int y, u, v;
  int r, g, b;
	
  y = (int)(*input_y) << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    *(WORD*)(*output) = ((r & 0xf8) << 8)
    + ((g & 0xfc) << 3)
    + ((b & 0xf8) >> 3);
  (*output) += 2;
}

static inline void transfer_YUV422P_to_BGR888(BYTE *(*output), 
					      BYTE *input_y,
					      BYTE *input_u,
					      BYTE *input_v)
{
  int y, u, v;
  int r, g, b;
	
  y = (int)(*input_y) << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = b;
  (*output)[1] = g;
  (*output)[2] = r;
  (*output) += 3;
}

static inline void transfer_YUV422P_to_BGR8888(BYTE *(*output), 
					       BYTE *input_y,
					       BYTE *input_u,
					       BYTE *input_v)
{
  int y, u, v;
  int r, g, b;

  y = (int)(*input_y) << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = b;
  (*output)[1] = g;
  (*output)[2] = r;
  (*output) += 4;
}

static inline void transfer_YUV422P_to_RGB888(BYTE *(*output), 
					      BYTE *input_y,
					      BYTE *input_u,
					      BYTE *input_v)
{
  // Signedness is important
  int y, u, v;
  int r, g, b;

  y = *input_y << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output) += 3;
}

static inline void transfer_YUV422P_to_ARGB8888(BYTE *(*output), 
						BYTE *input_y,
						BYTE *input_u,
						BYTE *input_v)
{
  // Signedness is important
  int y, u, v;
  int r, g, b;

  y = *input_y << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = 0xff;
  (*output)[1] = r;
  (*output)[2] = g;
  (*output)[3] = b;
  (*output) += 4;
}

static inline void transfer_YUV422P_to_ABGR8888(BYTE *(*output), 
						BYTE *input_y,
						BYTE *input_u,
						BYTE *input_v)
{
  // Signedness is important
  int y, u, v;
  int r, g, b;

  y = *input_y << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = 0xff;
  (*output)[3] = r;
  (*output)[2] = g;
  (*output)[1] = b;
  (*output) += 4;
}

static inline void transfer_YUV422P_to_RGBA8888(BYTE *(*output), 
						BYTE *input_y,
						BYTE *input_u,
						BYTE *input_v)
{
  // Signedness is important
  int y, u, v;
  int r, g, b;

  y = *input_y << 16;
  u = *input_u;
  v = *input_v;
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output)[3] = 0xff;
  (*output) += 4;
}

static inline void transfer_YUV422P_to_RGB161616(WORD *(*output), 
						 BYTE *input_y,
						 BYTE *input_u,
						 BYTE *input_v)
{
  // Signedness is important
  int y, u, v;
  int r, g, b;
  y = *input_y << 16;
  u = *input_u << 8;
  v = *input_v << 8;
  YUV_TO_RGB16(y, u, v, r, g, b)

    (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;

  (*output) += 3;
}


static inline void transfer_YUV422P_to_RGBA16161616(WORD *(*output), 
						    BYTE *input_y,
						    BYTE *input_u,
						    BYTE *input_v)
{
  // Signedness is important
  int y, u, v;
  int r, g, b;
  y = *input_y << 16;
  u = *input_u << 8;
  v = *input_v << 8;
  YUV_TO_RGB16(y, u, v, r, g, b)

    (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output)[3] = 0xffff;

  (*output) += 4;
}



static inline void transfer_YUV422P_to_YUV888(BYTE *(*output), 
					      BYTE *input_y,
					      BYTE *input_u,
					      BYTE *input_v)
{
  (*output)[0] = *input_y;
  (*output)[1] = *input_u;
  (*output)[2] = *input_v;
  (*output) += 3;
}

static inline void transfer_YUV422P_to_YUV161616(WORD *(*output), 
						 BYTE *input_y,
						 BYTE *input_u,
						 BYTE *input_v)
{
  (*output)[0] = *input_y << 8;
  (*output)[1] = *input_u << 8;
  (*output)[2] = *input_v << 8;
  (*output) += 3;
}

static inline void transfer_YUV422P_to_YUVA8888(BYTE *(*output), 
						BYTE *input_y,
						BYTE *input_u,
						BYTE *input_v)
{
  (*output)[0] = *input_y;
  (*output)[1] = *input_u;
  (*output)[2] = *input_v;
  (*output)[3] = 0xff;
  (*output) += 4;
}

static inline void transfer_YUV422P_to_YUVA16161616(WORD *(*output), 
						    BYTE *input_y,
						    BYTE *input_u,
						    BYTE *input_v)
{
	(*output)[0] = ((WORD)*input_y) << 8;
	(*output)[1] = ((WORD)*input_u) << 8;
	(*output)[2] = ((WORD)*input_v) << 8;


	(*output)[3] = 0xffff;
	(*output) += 4;
}

static inline void transfer_YUV422P_to_YUV420P(BYTE *input_y,
					       BYTE *input_u,
					       BYTE *input_v,
					       BYTE *output_y,
					       BYTE *output_u,
					       BYTE *output_v,
					       int j)
{
  output_y[j] = *input_y;
  output_u[j / 2] = *input_u;
  output_v[j / 2] = *input_v;
}

static inline void transfer_YUV422P_to_YUV444P(BYTE *input_y,
	BYTE *input_u,
	BYTE *input_v,
	BYTE *output_y,
	BYTE *output_u,
	BYTE *output_v,
	int j)
{
	output_y[j] = *input_y;
	output_u[j] = *input_u;
	output_v[j] = *input_v;
}

static inline void transfer_YUV422P_to_YUV422(BYTE *(*output), 
					      BYTE *input_y,
					      BYTE *input_u,
					      BYTE *input_v,
					      int j)
{
  // Store U and V for even pixels only
  if(!(j & 1))
    {
      (*output)[1] = *input_u;
      (*output)[3] = *input_v;
      (*output)[0] = *input_y;
    }
  else
    // Store Y and advance output for odd pixels only
    {
      (*output)[2] = *input_y;
      (*output) += 4;
    }
}















// ******************************** YUV444P -> ********************************

static inline void transfer_YUV444P_to_YUV444P(BYTE *input_y,
	BYTE *input_u,
	BYTE *input_v,
	BYTE *output_y,
	BYTE *output_u,
	BYTE *output_v,
	int j)
{
	output_y[j] = *input_y;
	output_u[j] = *input_u;
	output_v[j] = *input_v;
}










// ******************************** YUV422 -> *********************************

static inline void transfer_YUV422_to_RGB8(BYTE *(*output), 
					   BYTE *input, 
					   int column)
{
  int y, u, v;
  int r, g, b;

  // Even pixel
  if(!(column & 1))
    y = (int)(input[0]) << 16;
  else
    // Odd pixel
    y = (int)(input[2]) << 16;

  u = input[1];
  v = input[3];
  YUV_TO_RGB(y, u, v, r, g, b)

    *(*output) = (BYTE)((r & 0xc0) +
				 ((g & 0xe0) >> 2) +
				 ((b & 0xe0) >> 5));
  (*output)++;
}

static inline void transfer_YUV422_to_BGR565(BYTE *(*output), 
	BYTE *input, 
	int column)
{
	int y, u, v;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = (int)(input[0]) << 16;
	else
// Odd pixel
		y = (int)(input[2]) << 16;
	u = input[1];
	v = input[3];
	YUV_TO_RGB(y, u, v, r, g, b)

	*(WORD*)(*output) = ((b & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((r & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_YUV422_to_RGB565(BYTE *(*output), 
					     BYTE *input, 
					     int column)
{
  int y, u, v;
  int r, g, b;

  // Even pixel
  if(!(column & 1))
    y = (int)(input[0]) << 16;
  else
    // Odd pixel
    y = (int)(input[2]) << 16;
  u = input[1];
  v = input[3];
  YUV_TO_RGB(y, u, v, r, g, b)

    *(WORD*)(*output) = ((r & 0xf8) << 8)
    + ((g & 0xfc) << 3)
    + ((b & 0xf8) >> 3);
  (*output) += 2;
}

static inline void transfer_YUV422_to_BGR888(BYTE *(*output), 
					     BYTE *input, 
					     int column)
{
  int y, u, v;
  int r, g, b;

  // Even pixel
  if(!(column & 1))
    y = (int)(input[0]) << 16;
  else
    // Odd pixel
    y = (int)(input[2]) << 16;
  u = input[1];
  v = input[3];
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = b;
  (*output)[1] = g;
  (*output)[2] = r;
  (*output) += 3;
}

static inline void transfer_YUV422_to_RGB888(BYTE *(*output), 
					     BYTE *input, 
					     int column)
{
  int y, u, v;
  int r, g, b;

  // Even pixel
  if(!(column & 1))
    y = (int)(input[0]) << 16;
  else
    // Odd pixel
    y = (int)(input[2]) << 16;
  u = input[1];
  v = input[3];
  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = r;
  (*output)[1] = g;
  (*output)[2] = b;
  (*output) += 3;
}

static inline void transfer_YUV422_to_YUV888(BYTE *(*output), 
					     BYTE *input, 
					     int column)
{
  // Even pixel
  if(!(column & 1))
    (*output)[0] = input[0];
  else
    // Odd pixel
    (*output)[0] = input[2];

  (*output)[1] = input[1];
  (*output)[2] = input[3];
  (*output) += 3;
}

static inline void transfer_YUV422_to_YUVA8888(BYTE *(*output), 
					       BYTE *input, 
					       int column)
{
  // Even pixel
  if(!(column & 1))
    (*output)[0] = input[0];
  else
    // Odd pixel
    (*output)[0] = input[2];

  (*output)[1] = input[1];
  (*output)[2] = input[3];
  (*output)[3] = 255;
  (*output) += 4;
}

static inline void transfer_YUV422_to_YUV161616(WORD *(*output), 
						BYTE *input, 
						int column)
{
  // Even pixel
  if(!(column & 1))
    (*output)[0] = input[0] << 8;
  else
    // Odd pixel
    (*output)[0] = input[2] << 8;

  (*output)[1] = input[1] << 8;
  (*output)[2] = input[3] << 8;
  (*output) += 3;
}

static inline void transfer_YUV422_to_YUVA16161616(WORD *(*output), 
						   BYTE *input, 
						   int column)
{
// Even pixel
	if(!(column & 1))
		(*output)[0] = input[0] << 8;
	else
// Odd pixel
		(*output)[0] = input[2] << 8;

	(*output)[1] = input[1] << 8;
	(*output)[2] = input[3] << 8;
	(*output)[3] = 0xffff;
	(*output) += 4;
}

static inline void transfer_YUV422_to_BGR8888(BYTE *(*output), 
					      BYTE *input, 
					      int column)
{
  int y, u, v;
  int r, g, b;

  // Even pixel
  if(!(column & 1))
    y = (int)(input[0]) << 16;
  else
    // Odd pixel
    y = (int)(input[2]) << 16;
  u = input[1];
  v = input[3];

  YUV_TO_RGB(y, u, v, r, g, b)

    (*output)[0] = b;
  (*output)[1] = g;
  (*output)[2] = r;
  (*output) += 4;
}


static inline void transfer_YUV422_to_YUV422P(BYTE *output_y, 
					      BYTE *output_u, 
					      BYTE *output_v, 
					      BYTE *input,
					      int output_column)
{
  // Store U and V for even pixels only
  if(!(output_column & 1))
    {
      output_y[output_column] = input[0];
      output_u[output_column / 2] = input[1];
      output_v[output_column / 2] = input[3];
    }
  else
    // Store Y and advance output for odd pixels only
    {
      output_y[output_column] = input[2];
    }
}

static inline void transfer_YUV422_to_YUV420P(BYTE *output_y, 
					      BYTE *output_u, 
					      BYTE *output_v, 
					      BYTE *input,
					      int output_column,
					      int output_row)
{
  // Even column
  if(!(output_column & 1))
    {
      output_y[output_column] = input[0];
      // Store U and V for even columns and even rows only
      if(!(output_row & 1))
	{
	  output_u[output_column / 2] = input[1];
	  output_v[output_column / 2] = input[3];
	}
    }
  else
    // Odd column
    {
      output_y[output_column] = input[2];
    }
}

static inline void transfer_YUV422_to_YUV422(BYTE *(*output), 
					     BYTE *input,
					     int j)
{
  // Store U and V for even pixels only
  if(!(j & 1))
    {
      (*output)[0] = input[0];
      (*output)[1] = input[1];
      (*output)[3] = input[3];
    }
  else
    // Store Y and advance output for odd pixels only
    {
      (*output)[2] = input[2];
      (*output) += 4;
    }
}








// ******************************** Loops *************************************

#define TRANSFER_FRAME_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *output_row = output_rows[i + out_y] + out_x * out_pixelsize; \
		BYTE *input_row = input_rows[row_table[i]]; \
		int bit_counter = 7; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_FRAME_TAIL \
		} \
	}

#define TRANSFER_YUV420P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *input_row = input_rows[row_table[i]]; \
		BYTE *output_y = out_y_plane + i * total_out_w + out_x; \
		BYTE *output_u = out_u_plane + i / 2 * total_out_w / 2 + out_x / 2; \
		BYTE *output_v = out_v_plane + i / 2 * total_out_w / 2 + out_x / 2; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *input_row = input_rows[row_table[i]]; \
		BYTE *output_y = out_y_plane + i * total_out_w + out_x; \
		BYTE *output_u = out_u_plane + i * total_out_w / 2 + out_x / 2; \
		BYTE *output_v = out_v_plane + i * total_out_w / 2 + out_x / 2; \
		for(j = 0; j < out_w; j++) \
		{
		  
#define TRANSFER_YUV444P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *input_row = input_rows[row_table[i]]; \
		BYTE *output_y = out_y_plane + i * total_out_w + out_x; \
		BYTE *output_u = out_u_plane + i * total_out_w + out_x; \
		BYTE *output_v = out_v_plane + i * total_out_w + out_x; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV420P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *output_row = output_rows[i + out_y] + out_x * out_pixelsize; \
		BYTE *input_y = in_y_plane + row_table[i] * total_in_w; \
		BYTE *input_u = in_u_plane + (row_table[i] / 2) * (total_in_w / 2); \
		BYTE *input_v = in_v_plane + (row_table[i] / 2) * (total_in_w / 2); \
		for(j = 0; j < out_w; j++) \
		{


#define TRANSFER_YUV422P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *output_row = output_rows[i + out_y] + out_x * out_pixelsize; \
		BYTE *input_y = in_y_plane + row_table[i] * total_in_w; \
		BYTE *input_u = in_u_plane + row_table[i] * (total_in_w / 2); \
		BYTE *input_v = in_v_plane + row_table[i] * (total_in_w / 2); \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV444P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *output_row = output_rows[i + out_y] + out_x * out_pixelsize; \
		BYTE *input_y = in_y_plane + row_table[i] * total_in_w; \
		BYTE *input_u = in_u_plane + row_table[i] * total_in_w; \
		BYTE *input_v = in_v_plane + row_table[i] * total_in_w; \
		for(j = 0; j < out_w; j++) \
		{


#define TRANSFER_YUV422_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		BYTE *output_row = output_rows[i + out_y] + ((out_x * out_pixelsize) & 0xfffffffc); \
		BYTE *input_y = in_y_plane + row_table[i] * total_in_w; \
		BYTE *input_u = in_u_plane + row_table[i] * (total_in_w / 2); \
		BYTE *input_v = in_v_plane + row_table[i] * (total_in_w / 2); \
		for(j = 0; j < out_w; j++) \
		{


#if 0
		} } /* for emacs */
#endif

#define TRANSFER_FAIL \
	fprintf(stderr, "oqt_cmodel_transfer: warning, transfer from [%d] to [%d] is not supported.\n", \
	in_colormodel, out_colormodel);
							  
 
// ******************************** Permutation *******************************
 


#define PERMUTATION_ARGS \
	BYTE **output_rows,  \
	BYTE **input_rows, \
	BYTE *out_y_plane, \
	BYTE *out_u_plane, \
	BYTE *out_v_plane, \
	BYTE *in_y_plane, \
	BYTE *in_u_plane, \
	BYTE *in_v_plane, \
	int in_x,  \
	int in_y,  \
	int in_w,  \
	int in_h, \
	int out_x,  \
	int out_y,  \
	int out_w,  \
	int out_h, \
	int in_colormodel,  \
	int out_colormodel, \
	int bg_color, \
	int total_in_w, \
	int total_out_w, \
	int scale, \
	int out_pixelsize, \
	int in_pixelsize, \
	int *row_table, \
	int *column_table, \
	int bg_r, \
	int bg_g, \
	int bg_b


extern void oqt_cmodel_transfer_yuv422(PERMUTATION_ARGS);
extern void oqt_cmodel_transfer_default(PERMUTATION_ARGS);
extern void oqt_cmodel_transfer_yuv420p(PERMUTATION_ARGS);
extern void oqt_cmodel_transfer_yuv444p(PERMUTATION_ARGS);

