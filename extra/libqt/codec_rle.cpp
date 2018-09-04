/* codec_rle.c
* Copyright (C) 2002 OpenQuicktime Teams
*
* This file is part of OpenQuicktime, a free QuickTime library.
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
* $Id: codec_rle.c,v 1.12 2002/12/28 18:50:59 nj_humfrey Exp $
*/

#include "stdafx.h"
#include "qtcodec.h"

// little endian
#define RGBA_TO_RGBA_NATIVE(r,g,b,a) (a<<24|b<<16|g<<8|r)
#define RGBA_TO_ARGB_NATIVE(r,g,b,a) (b<<24|g<<16|r<<8|a)
#define RGBA_TO_ABGR_NATIVE(r,g,b,a) (r<<24|g<<16|b<<8|a)
#define RGBA_TO_BGRA_NATIVE(r,g,b,a) (a<<24|r<<16|g<<8|b)

typedef struct {
	unsigned char  *mTempData;
	long            mTempSize;
	int             mWarnConversion;
	void (*decode)(unsigned char* input,unsigned long inputsize,int width,int height,unsigned char *output);
} Private;

static void decodeRLE16_BGRA8888(unsigned char* input,unsigned long inputsize,int width,int height,unsigned char *output);
static void decodeRLE24_BGRA8888(unsigned char* input,unsigned long inputsize,int width,int height,unsigned char *output);
static void decodeRLE32_BGRA8888(unsigned char* input,unsigned long inputsize,int width,int height,unsigned char *output);

/* allocate one a temp buffer for RLE */
static int check_temp_buffer_rle(Private* p, long size)
{
	if (!p->mTempData) {
		p->mTempSize=size;
		p->mTempData=(unsigned char*)malloc(p->mTempSize);
	} else if (p->mTempSize<size) {
		p->mTempSize=size;
		p->mTempData=(unsigned char*)realloc(p->mTempData,p->mTempSize);
	}
	if (!p->mTempData) {
		fprintf(stderr,"check_temp_buffer_rle: can't (re-)allocate temp buffer (%ld bytes)\n",size);
		return -1;
	}
	return 0;
}


/* see if row_pointers are consecutive 
- is this acutally a performance gain ?
*/
/*static int rows_consecutive_rle(unsigned char **row_pointers, int w, int h, int depth)
{
int i, result;
int bpl = w*depth;
for(i = 1, result = 1; i < h; i++)
{
if(row_pointers[i] - row_pointers[i - 1] != bpl) result = 0;
}
return result;
}*/

/*************************************************************/
/* Decoding function                                         */
/*************************************************************/
int decode_rle(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch)
{
	Private *p = (Private*)codec;
	int width 	= desc->width;
	int height 	= desc->height;
	int depth   = desc->depth;
	int y, bpl, frame_size;
	BYTE* src;
	/* allocate temp buffer or resize buffer if needed */

	frame_size = oqt_cmodel_calculate_framesize(width, height, BC_BGRA8888, -1);
	if (check_temp_buffer_rle(p, frame_size)) return -1;
	p->decode(input,inputsize,width,height,p->mTempData);
	/* Copy decoded buffer into output buffer */
	bpl = width *4;
	src = p->mTempData;
	for (y=0;y<height;y++,src+=bpl)
		memcpy(output+y*pitch,src,bpl);

	return 0;
}

void* init_rle(ImageDescriptionPtr desc,int* colormodel)
{
	void *p = malloc(sizeof(Private));
	memset(p, 0, sizeof(Private));
	((Private*)p)->mWarnConversion = 1;
	*colormodel =  BC_BGRA8888;
	int depth   = desc->depth;
	switch (depth) 
	{
	case 32:
		((Private*)p)->decode = decodeRLE32_BGRA8888;
		break;
	case 24:
		((Private*)p)->decode = decodeRLE24_BGRA8888;
		break;
	case 16:
		((Private*)p)->decode = decodeRLE16_BGRA8888;
		break;
	default:
		free(p);
		return 0;
	}
	return p;
}

void delete_rle(void* codec)
{
	Private *p = (Private*)codec;
	if(p) 
	{
		if (p->mTempData) free(p->mTempData);
		free(p);
	}
}

static void decodeRLE16_BGRA8888(unsigned char* input,unsigned long inputsize,int width,int height,unsigned char *output)
{
	ULONG imagex = width,imagey = height,y,lines,cnt,dsize=inputsize,xskip;
	register BYTE *dp = input;
	register ULONG *iptr;      
	register ULONG r,g,b,d; 

	dp += 4;    /* skip codec size */  
	d = (*dp++) << 8;  d |= *dp++;   /* read code either 0008 or 0000 */ 
	if (dsize < 8) { /* NOP */
		return;
	}

	if (d & 0x0008) { /* Header present */
		y = (*dp++) << 8;  y |= *dp++;           /* start line */
		dp += 2;                                  /* unknown */
		lines = (*dp++) << 8; lines |= *dp++;   /* number of lines */
		dp += 2;                                  /* unknown */
	}
	else {
		y = 0;
		lines = imagey;
	}

	while(lines--) {			/* loop thru lines */
		xskip = *dp++;				/* skip x pixels */
		if (xskip == 0) break;			/* exit */
		cnt = *dp++;				/* RLE code */
		iptr = ( ULONG*)(output + 4*((y * imagex)+(xskip-1)) );
		while(cnt != 0xff) {			/* while not EOL */
			if (cnt == 0x00) {
				xskip = *dp++;
				iptr += 3*(xskip-1);
			}
			else if (cnt < 0x80) {			/* run of data */
				while(cnt--) {
					d = (*dp++ << 8); d |= *dp++;
					r = (d >> 10) & 0x1f;	r = (r << 3) | (r >> 2);
					g = (d >>  5) & 0x1f;	g = (g << 3) | (g >> 2);
					b =  d & 0x1f;			b = (b << 3) | (b >> 2);
					*iptr++ = r;
					*iptr++ = g;
					*iptr++ = b;
					*iptr++ = RGBA_TO_BGRA_NATIVE(r,g,b,0xFF); // for BGRA
				}
			}
			else	{				// repeat data
				cnt = 0x100 - cnt;
				d = (*dp++ << 8); d |= *dp++;
				r = (d >> 10) & 0x1f;	r = (r << 3) | (r >> 2);
				g = (d >>  5) & 0x1f;	g = (g << 3) | (g >> 2);
				b =  d & 0x1f;			b = (b << 3) | (b >> 2);
				d = RGBA_TO_BGRA_NATIVE(r,g,b,0xFF); // for BGRA
				while(cnt--) {
					*iptr++ = d;
				}
			}
			cnt = *dp++;				// get new RLE code
		} // end of line 
		y++;
	}
}


static void decodeRLE24_BGRA8888(unsigned char* input,unsigned long inputsize,int width,int height,unsigned char *output)
{
	ULONG imagex = width,imagey = height,y,lines,cnt,dsize=inputsize,xskip;
	register BYTE *dp = input;
	register ULONG *iptr;      
	register ULONG r,g,b,d; 

	dp += 4;    /* skip codec size */  
	d = (*dp++) << 8;  d |= *dp++;   /* read code either 0008 or 0000 */ 
	if (dsize < 8) { /* NOP */
		return;
	}

	if (d & 0x0008) { /* Header present */
		y = (*dp++) << 8;  y |= *dp++;           /* start line */
		dp += 2;                                  /* unknown */
		lines = (*dp++) << 8; lines |= *dp++;   /* number of lines */
		dp += 2;                                  /* unknown */
	}
	else {
		y = 0;
		lines = imagey;
	}

	while(lines--) {			/* loop thru lines */
		xskip = *dp++;				/* skip x pixels */
		if (xskip == 0) break;			/* exit */
		cnt = *dp++;				/* RLE code */
		iptr = ( ULONG*)(output + 4*((y * imagex)+(xskip-1)) );
		while(cnt != 0xff) {			/* while not EOL */
			if (cnt == 0x00) {
				xskip = *dp++;
				iptr += 3*(xskip-1);
			}
			else if (cnt < 0x80) {			/* run of data */
				while(cnt--) {
					r = *dp++;
					g = *dp++;
					b = *dp++;
					*iptr++ = RGBA_TO_BGRA_NATIVE(r,g,b,0xFF); // for BGRA
				}
			}
			else	{				// repeat data
				cnt = 0x100 - cnt;
				r = *dp++;
				g = *dp++;
				b = *dp++;
				d = RGBA_TO_BGRA_NATIVE(r,g,b,0xFF); // for BGRA
				while(cnt--) {
					*iptr++ = d;
				}
			}
			cnt = *dp++;				// get new RLE code
		} // end of line 
		y++;
	}
}


static void decodeRLE32_BGRA8888(unsigned char* input,unsigned long inputsize,int width,int height,unsigned char *output)
{
	ULONG imagex = width,imagey = height,y,lines,cnt,dsize=inputsize,xskip;
	register BYTE *dp = input;
	register ULONG *iptr;      
	register ULONG r,g,b,a,d; 

	dp += 4;    /* skip codec size */  
	d = (*dp++) << 8;  d |= *dp++;   /* read code either 0008 or 0000 */ 
	if (dsize < 8) { /* NOP */
		return;
	}

	if (d & 0x0008) { /* Header present */
		y = (*dp++) << 8;  y |= *dp++;           /* start line */
		dp += 2;                                  /* unknown */
		lines = (*dp++) << 8; lines |= *dp++;   /* number of lines */
		dp += 2;                                  /* unknown */
	}
	else {
		y = 0;
		lines = imagey;
	}

	while(lines--) {			/* loop thru lines */
		xskip = *dp++;				/* skip x pixels */
		if (xskip == 0) break;			/* exit */
		cnt = *dp++;				/* RLE code */
		iptr = ( ULONG*)(output + 4*((y * imagex)+(xskip-1)) );
		while(cnt != 0xff) {			/* while not EOL */
			if (cnt == 0x00) {
				xskip = *dp++;
				iptr += (xskip-1);
			}
			else if (cnt < 0x80) {			/* run of data */
				while(cnt--) {
					a = *dp++;
					r = *dp++;
					g = *dp++;
					b = *dp++;
					*iptr++ = RGBA_TO_BGRA_NATIVE(r,g,b,a); // for BGRA
				}
			}
			else	{				// repeat data
				cnt = 0x100 - cnt;
				a = *dp++;
				r = *dp++;
				g = *dp++;
				b = *dp++;
				d = RGBA_TO_BGRA_NATIVE(r,g,b,a); // for BGRA
				while(cnt--) {
					*iptr++ = d;
				}
			}
			cnt = *dp++;				// get new RLE code
		} // end of line 
		y++;
	}
}

