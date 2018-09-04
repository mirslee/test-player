#include "stdafx.h"
#include "qtcodec.h"

void *decode_cinepak_init(void);
void decode_cinepak(void *context, unsigned char *buf,int size, unsigned char *frame, int width,int height, int bit_per_pixel, int frm_stride);

/* Each instance of the codec should have its own Param structure allocated */
typedef struct {
	void *context;
	char *buffer;
} Param;  


void* init_cvid(ImageDescriptionPtr desc,int* colormodel)
{
	void *p = malloc(sizeof(Param));
	memset(p, 0, sizeof(Param));
	/* Initalise the Codec */
	((Param*)p)->context = decode_cinepak_init();
	((Param*)p)->buffer = (char*)malloc(desc->width * desc->height * 4);
	*colormodel = BC_BGRA8888;
	return p;
}



void delete_cvid(void* codec)
{
	Param *p = (Param*)codec;
	if(p) 
	{
		if (p->context)	free(p->context);
		if (p->buffer)	free(p->buffer);
		free(p);
	}
}



/*************************************************************/
/* Decoding function                                         */
/*************************************************************/
int decode_cvid(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch)
{
	Param *p = (Param*)codec;
	int width 	= desc->width;
	int height 	= desc->height;
	decode_cinepak(p->context, input, inputsize,(BYTE*)p->buffer, width, height, 32, width*4);
	for(int i = 0; i < height; i++)
		memcpy(output+i*pitch, &(p->buffer[i*width*4]), width*4);

	return 0;
}





#define DBUG	0
#define MAX_STRIPS 32


/* ------------------------------------------------------------------------ */
typedef struct
{
	unsigned char y0, y1, y2, y3;
	char u, v;
	unsigned long rgb0, rgb1, rgb2, rgb3;		/* should be a union */
	unsigned char r[4], g[4], b[4];
} cvid_codebook;

typedef struct {
	cvid_codebook *v4_codebook[MAX_STRIPS];
	cvid_codebook *v1_codebook[MAX_STRIPS];
	int strip_num;

	unsigned char *buf;
	unsigned char uiclip[1024];
	unsigned char *uiclp;
} cinepak_context;


/* ------------------------------------------------------------------------ */

#define get_byte(buf) *(buf++)
#define skip_byte(buf) buf++
#define get_word(buf) ((unsigned short)(buf += 2, (buf[-2] << 8 | buf[-1])))
#define get_long(buf) ((unsigned long)(buf += 4, \
	(buf[-4] << 24 | buf[-3] << 16 | buf[-2] << 8 | buf[-1])))


#define le2me_32(x) (x|0xFF000000)

/* ---------------------------------------------------------------------- */
static inline void read_codebook_32(cinepak_context *p, cvid_codebook *c, int mode)
{
	int uvr, uvg, uvb;

	if(mode)		/* black and white */
	{
		c->y0 = get_byte(p->buf);
		c->y1 = get_byte(p->buf);
		c->y2 = get_byte(p->buf);
		c->y3 = get_byte(p->buf);
		c->u = c->v = 0;

		c->rgb0 = (c->y0 << 16) | (c->y0 << 8) | c->y0;
		c->rgb1 = (c->y1 << 16) | (c->y1 << 8) | c->y1;
		c->rgb2 = (c->y2 << 16) | (c->y2 << 8) | c->y2;
		c->rgb3 = (c->y3 << 16) | (c->y3 << 8) | c->y3;
	}
	else			/* colour */
	{
		c->y0 = get_byte(p->buf);  /* luma */
		c->y1 = get_byte(p->buf);
		c->y2 = get_byte(p->buf);
		c->y3 = get_byte(p->buf);
		c->u = get_byte(p->buf); /* chroma */
		c->v = get_byte(p->buf);

		uvr = c->v << 1;
		uvg = -((c->u+1) >> 1) - c->v;
		uvb = c->u << 1;

		c->rgb0 = le2me_32((p->uiclp[c->y0 + uvr] << 16) | (p->uiclp[c->y0 + uvg] << 8) | p->uiclp[c->y0 + uvb]);
		c->rgb1 = le2me_32((p->uiclp[c->y1 + uvr] << 16) | (p->uiclp[c->y1 + uvg] << 8) | p->uiclp[c->y1 + uvb]);
		c->rgb2 = le2me_32((p->uiclp[c->y2 + uvr] << 16) | (p->uiclp[c->y2 + uvg] << 8) | p->uiclp[c->y2 + uvb]);
		c->rgb3 = le2me_32((p->uiclp[c->y3 + uvr] << 16) | (p->uiclp[c->y3 + uvg] << 8) | p->uiclp[c->y3 + uvb]);
	}
}


/* ------------------------------------------------------------------------ */
static inline void cvid_v1_32(cinepak_context *p, unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb)
{
	unsigned long *vptr = (unsigned long *)frm, rgb;
	int row_inc = stride/4;

	vptr[0] = rgb = cb->rgb0; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb1; vptr[3] = rgb;
	vptr += row_inc; //if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb0; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb1; vptr[3] = rgb;
	vptr += row_inc; //if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb2; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb3; vptr[3] = rgb;
	vptr += row_inc; //if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb2; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb3; vptr[3] = rgb;
}


/* ------------------------------------------------------------------------ */
static inline void cvid_v4_32(cinepak_context *p, unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb0,
							  cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
	unsigned long *vptr = (unsigned long *)frm;
	int row_inc = stride/4;

	vptr[0] = cb0->rgb0;
	vptr[1] = cb0->rgb1;
	vptr[2] = cb1->rgb0;
	vptr[3] = cb1->rgb1;
	vptr += row_inc; //if(vptr > (unsigned long *)end) return;
	vptr[0] = cb0->rgb2;
	vptr[1] = cb0->rgb3;
	vptr[2] = cb1->rgb2;
	vptr[3] = cb1->rgb3;
	vptr += row_inc; //if(vptr > (unsigned long *)end) return;
	vptr[0] = cb2->rgb0;
	vptr[1] = cb2->rgb1;
	vptr[2] = cb3->rgb0;
	vptr[3] = cb3->rgb1;
	vptr += row_inc; //if(vptr > (unsigned long *)end) return;
	vptr[0] = cb2->rgb2;
	vptr[1] = cb2->rgb3;
	vptr[2] = cb3->rgb2;
	vptr[3] = cb3->rgb3;
}


/* ---------------------------------------------------------------------- */
static inline void read_codebook_24(cinepak_context *p, cvid_codebook *c, int mode)
{
	int uvr, uvg, uvb;

	if(mode)		/* black and white */
	{
		c->y0 = get_byte(p->buf);
		c->y1 = get_byte(p->buf);
		c->y2 = get_byte(p->buf);
		c->y3 = get_byte(p->buf);
		c->u = c->v = 0;

		c->r[0] = c->g[0] = c->b[0] = c->y0;
		c->r[1] = c->g[1] = c->b[1] = c->y1;
		c->r[2] = c->g[2] = c->b[2] = c->y2;
		c->r[3] = c->g[3] = c->b[3] = c->y3;
	}
	else			/* colour */
	{
		c->y0 = get_byte(p->buf);  /* luma */
		c->y1 = get_byte(p->buf);
		c->y2 = get_byte(p->buf);
		c->y3 = get_byte(p->buf);
		c->u = get_byte(p->buf); /* chroma */
		c->v = get_byte(p->buf);

		uvr = c->v << 1;
		uvg = -((c->u+1) >> 1) - c->v;
		uvb = c->u << 1;

		c->r[0] = p->uiclp[c->y0 + uvr]; c->g[0] = p->uiclp[c->y0 + uvg]; c->b[0] = p->uiclp[c->y0 + uvb];
		c->r[1] = p->uiclp[c->y1 + uvr]; c->g[1] = p->uiclp[c->y1 + uvg]; c->b[1] = p->uiclp[c->y1 + uvb];
		c->r[2] = p->uiclp[c->y2 + uvr]; c->g[2] = p->uiclp[c->y2 + uvg]; c->b[2] = p->uiclp[c->y2 + uvb];
		c->r[3] = p->uiclp[c->y3 + uvr]; c->g[3] = p->uiclp[c->y3 + uvg]; c->b[3] = p->uiclp[c->y3 + uvb];
	}
}


/* ------------------------------------------------------------------------ */
static void cvid_v1_24(cinepak_context *p, unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb)
{
	unsigned char r, g, b;
	int row_inc = stride-4*3;

	*vptr++ = b = cb->b[0]; *vptr++ = g = cb->g[0]; *vptr++ = r = cb->r[0];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[1]; *vptr++ = g = cb->g[1]; *vptr++ = r = cb->r[1];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; //if(vptr > end) return;
	*vptr++ = b = cb->b[0]; *vptr++ = g = cb->g[0]; *vptr++ = r = cb->r[0];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[1]; *vptr++ = g = cb->g[1]; *vptr++ = r = cb->r[1];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; //if(vptr > end) return;
	*vptr++ = b = cb->b[2]; *vptr++ = g = cb->g[2]; *vptr++ = r = cb->r[2];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[3]; *vptr++ = g = cb->g[3]; *vptr++ = r = cb->r[3];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; //if(vptr > end) return;
	*vptr++ = b = cb->b[2]; *vptr++ = g = cb->g[2]; *vptr++ = r = cb->r[2];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[3]; *vptr++ = g = cb->g[3]; *vptr++ = r = cb->r[3];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
}


/* ------------------------------------------------------------------------ */
static void cvid_v4_24(cinepak_context *p, unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb0,
					   cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
	int row_inc = stride-4*3;

	*vptr++ = cb0->b[0]; *vptr++ = cb0->g[0]; *vptr++ = cb0->r[0];
	*vptr++ = cb0->b[1]; *vptr++ = cb0->g[1]; *vptr++ = cb0->r[1];
	*vptr++ = cb1->b[0]; *vptr++ = cb1->g[0]; *vptr++ = cb1->r[0];
	*vptr++ = cb1->b[1]; *vptr++ = cb1->g[1]; *vptr++ = cb1->r[1];
	vptr += row_inc; //if(vptr > end) return;
	*vptr++ = cb0->b[2]; *vptr++ = cb0->g[2]; *vptr++ = cb0->r[2];
	*vptr++ = cb0->b[3]; *vptr++ = cb0->g[3]; *vptr++ = cb0->r[3];
	*vptr++ = cb1->b[2]; *vptr++ = cb1->g[2]; *vptr++ = cb1->r[2];
	*vptr++ = cb1->b[3]; *vptr++ = cb1->g[3]; *vptr++ = cb1->r[3];
	vptr += row_inc; //if(vptr > end) return;
	*vptr++ = cb2->b[0]; *vptr++ = cb2->g[0]; *vptr++ = cb2->r[0];
	*vptr++ = cb2->b[1]; *vptr++ = cb2->g[1]; *vptr++ = cb2->r[1];
	*vptr++ = cb3->b[0]; *vptr++ = cb3->g[0]; *vptr++ = cb3->r[0];
	*vptr++ = cb3->b[1]; *vptr++ = cb3->g[1]; *vptr++ = cb3->r[1];
	vptr += row_inc; //if(vptr > end) return;
	*vptr++ = cb2->b[2]; *vptr++ = cb2->g[2]; *vptr++ = cb2->r[2];
	*vptr++ = cb2->b[3]; *vptr++ = cb2->g[3]; *vptr++ = cb2->r[3];
	*vptr++ = cb3->b[2]; *vptr++ = cb3->g[2]; *vptr++ = cb3->r[2];
	*vptr++ = cb3->b[3]; *vptr++ = cb3->g[3]; *vptr++ = cb3->r[3];
}


/* ------------------------------------------------------------------------
* Call this function once at the start of the sequence and save the
* returned context for calls to decode_cinepak().
*/
void *decode_cinepak_init(void)
{
	cinepak_context *context;
	int i;

	if((context = (cinepak_context*)malloc(sizeof(cinepak_context))) == NULL) return NULL;
	memset(context, 0, sizeof(cinepak_context));
	context->strip_num = 0;
	context->buf = NULL;

	// Initalise the record
	context->uiclp = context->uiclip+512;
	for(i = -512; i < 512; i++) {
		context->uiclp[i] = (i < 0 ? 0 : (i > 255 ? 255 : i));
	}

	return (void *)context;
}


/* ------------------------------------------------------------------------
* This function decodes a buffer containing a Cinepak encoded frame.
*
* context - the context created by decode_cinepak_init().
* buf - the input buffer to be decoded
* size - the size of the input buffer
* frame - the output frame buffer
* width - the width of the output frame
* height - the height of the output frame
* bit_per_pixel - the number of bits per pixel allocated to the output
*   frame (only 24 or 32 bpp are supported)
*/
void decode_cinepak(void *context, unsigned char *buf, int size, unsigned char *frame, int width, int height, int bit_per_pixel,
					int frm_stride)
{
	cinepak_context *p = (cinepak_context *)context;
	cvid_codebook *v4_codebook, *v1_codebook, *codebook = NULL;
	unsigned long x, y, y_bottom, frame_flags,cv_width, cv_height, 
		strip_id, chunk_id, x0, y0, x1, y1, ci, flag, mask;
	int cnum,strips;
	long len, top_size, chunk_size;
	int i, cur_strip, d0, d1, d2, d3, bpp = 3, modulo;
	unsigned char *frm_ptr, *frm_end;
	void (*read_codebook)(cinepak_context *p, cvid_codebook *c, int mode) = read_codebook_24;
	void (*cvid_v1)(cinepak_context *p, unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb) = cvid_v1_24;
	void (*cvid_v4)(cinepak_context *p,unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb0,
		cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3) = cvid_v4_24;

	x = y = 0;
	y_bottom = 0;
	p->buf = buf;

	frame_flags = get_byte(p->buf);
	len = get_byte(p->buf) << 16;
	len |= get_byte(p->buf) << 8;
	len |= get_byte(p->buf);

	switch(bit_per_pixel)
	{
	case 24:
		bpp = 3;
		read_codebook = read_codebook_24;
		cvid_v1 = cvid_v1_24;
		cvid_v4 = cvid_v4_24;
		break;
	case 32:
		bpp = 4;
		read_codebook = read_codebook_32;
		cvid_v1 = cvid_v1_32;
		cvid_v4 = cvid_v4_32;
		break;
	default:
		fprintf(stderr, "CVID: unsupported bit depth: %d\n", bit_per_pixel);
		return;
		break;
	}

	/* frm_stride = width * bpp; */
	frm_ptr = frame;
	frm_end = frm_ptr + width * height * bpp;

	if(len != size)
	{
		if(len & 0x01) len++; /* AVIs tend to have a size mismatch */
		if(len != size)
		{
			fprintf(stderr, "CVID: corruption %d (QT/AVI) != %ld (CV)\n", size, len);
			// return;
		}
	}

	cv_width = get_word(p->buf);
	cv_height = get_word(p->buf);
	strips = get_word(p->buf);

	if(strips > p->strip_num)
	{
		if(strips >= MAX_STRIPS) 
		{
			fprintf(stderr, "CVID: strip overflow (more than %d)\n", MAX_STRIPS);
			return;
		}

		for(i = p->strip_num; i < strips; i++)
		{
			if((p->v4_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 260)) == NULL)
			{
				fprintf(stderr, "CVID: codebook v4 alloc err\n");
				return;
			}

			if((p->v1_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 260)) == NULL)
			{
				fprintf(stderr, "CVID: codebook v1 alloc err\n");
				return;
			}
		}
	}
	p->strip_num = strips;

#if DBUG
	fprintf(stderr, "CVID: <%ld,%ld> strips %ld\n", cv_width, cv_height, strips);
#endif

	for(cur_strip = 0; cur_strip < strips; cur_strip++)
	{
		v4_codebook = p->v4_codebook[cur_strip];
		v1_codebook = p->v1_codebook[cur_strip];

		if((cur_strip > 0) && (!(frame_flags & 0x01)))
		{
			memcpy(p->v4_codebook[cur_strip], p->v4_codebook[cur_strip-1], 260 * sizeof(cvid_codebook));
			memcpy(p->v1_codebook[cur_strip], p->v1_codebook[cur_strip-1], 260 * sizeof(cvid_codebook));
		}

		strip_id = get_word(p->buf);		/* 1000 = key strip, 1100 = iter strip */
		top_size = get_word(p->buf);
		y0 = get_word(p->buf);		/* FIXME: most of these are ignored at the moment */
		x0 = get_word(p->buf);
		y1 = get_word(p->buf);
		x1 = get_word(p->buf);

		y_bottom += y1;
		top_size -= 12;
		x = 0;
		//		if(x1 != width) 
		//			fprintf(stderr, "CVID: Warning x1 (%ld) != width (%d)\n", x1, width);

#if DBUG
		fprintf(stderr, "   %d) %04lx %04ld <%ld,%ld> <%ld,%ld> yt %ld  %d\n",
			cur_strip, strip_id, top_size, x0, y0, x1, y1, y_bottom);
#endif

		while(top_size > 0)
		{
			chunk_id  = get_word(p->buf);
			chunk_size = get_word(p->buf);

#if DBUG
			fprintf(stderr, "        %04lx %04ld\n", chunk_id, chunk_size);
#endif
			top_size -= chunk_size;
			chunk_size -= 4;

			switch(chunk_id)
			{
				/* -------------------- Codebook Entries -------------------- */
			case 0x2000:
			case 0x2200:
				modulo = chunk_size % 6;
				codebook = (chunk_id == 0x2200 ? v1_codebook : v4_codebook);
				cnum = (chunk_size - modulo) / 6;
				for(i = 0; i < cnum; i++) read_codebook(p, codebook+i, 0);
				while (modulo--)
					p->buf++;
				break;

			case 0x2400:
			case 0x2600:		/* 8 bit per pixel */
				codebook = (chunk_id == 0x2600 ? v1_codebook : v4_codebook);
				cnum = chunk_size/4;  
				for(i = 0; i < cnum; i++) read_codebook(p, codebook+i, 1);
				break;

			case 0x2100:
			case 0x2300:
				codebook = (chunk_id == 0x2300 ? v1_codebook : v4_codebook);

				ci = 0;
				while(chunk_size > 3)
				{
					flag = get_long(p->buf);
					chunk_size -= 4;

					for(i = 0; i < 32; i++)
					{
						if(flag & 0x80000000)
						{
							chunk_size -= 6;
							read_codebook(p, codebook+ci, 0);
						}

						ci++;
						flag <<= 1;
					}
				}
				while(chunk_size > 0) { skip_byte(p->buf); chunk_size--; }
				break;

			case 0x2500:
			case 0x2700:		/* 8 bit per pixel */
				codebook = (chunk_id == 0x2700 ? v1_codebook : v4_codebook);

				ci = 0;
				while(chunk_size > 0)
				{
					flag = get_long(p->buf);
					chunk_size -= 4;

					for(i = 0; i < 32; i++)
					{
						if(flag & 0x80000000)
						{
							chunk_size -= 4;
							read_codebook(p, codebook+ci, 1);
						}

						ci++;
						flag <<= 1;
					}
				}
				while(chunk_size > 0) { skip_byte(p->buf); chunk_size--; }
				break;

				/* -------------------- Frame -------------------- */
			case 0x3000: 
				while((chunk_size > 0) && (y < y_bottom))
				{
					flag = get_long(p->buf);
					chunk_size -= 4;

					for(i = 0; i < 32; i++)
					{ 
						if(y >= y_bottom) break;
						if(flag & 0x80000000)	/* 4 bytes per block */
						{
							d0 = get_byte(p->buf);
							d1 = get_byte(p->buf);
							d2 = get_byte(p->buf);
							d3 = get_byte(p->buf);
							chunk_size -= 4;
							cvid_v4(p, frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v4_codebook+d0, v4_codebook+d1, v4_codebook+d2, v4_codebook+d3);
						}
						else		/* 1 byte per block */
						{
							cvid_v1(p, frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte(p->buf));
							chunk_size--;
						}

						x += 4;
						if(x >= (unsigned int)x1)
						{
							x = 0;
							y += 4;
						}
						flag <<= 1;
					}
				}
				while(chunk_size > 0) { skip_byte(p->buf); chunk_size--; }
				break;

			case 0x3100:
				while((chunk_size > 0) && (y < y_bottom))
				{
					/* ---- flag bits: 0 = SKIP, 10 = V1, 11 = V4 ---- */
					flag = (unsigned long)get_long(p->buf);
					chunk_size -= 4;
					mask = 0x80000000;

					while((mask) && (y < y_bottom))
					{
						if(flag & mask)
						{
							if(mask == 1)
							{
								if(chunk_size < 0) break;
								flag = (unsigned long)get_long(p->buf);
								chunk_size -= 4;
								mask = 0x80000000;
							}
							else mask >>= 1;

							if(flag & mask)		/* V4 */
							{
								d0 = get_byte(p->buf);
								d1 = get_byte(p->buf);
								d2 = get_byte(p->buf);
								d3 = get_byte(p->buf);
								chunk_size -= 4;
								cvid_v4(p, frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v4_codebook+d0, v4_codebook+d1, v4_codebook+d2, v4_codebook+d3);
							}
							else		/* V1 */
							{
								chunk_size--;
								cvid_v1(p, frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte(p->buf));
							}
						}		/* else SKIP */

						mask >>= 1;
						x += 4;
						if(x >= (unsigned int)x1)
						{
							x = 0;
							y += 4;
						}
					}
				}

				while(chunk_size > 0) { skip_byte(p->buf); chunk_size--; }
				break;

			case 0x3200:		/* each byte is a V1 codebook */
				while((chunk_size > 0) && (y < y_bottom))
				{
					cvid_v1(p, frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte(p->buf));
					chunk_size--;
					x += 4;
					if(x >= (unsigned int)x1)
					{
						x = 0;
						y += 4;
					}
				}
				while(chunk_size > 0) { skip_byte(p->buf); chunk_size--; }
				break;

			default:
				fprintf(stderr, "CVID: unknown chunk_id %08lx\n", chunk_id);
				while(chunk_size > 0) { skip_byte(p->buf); chunk_size--; }
				break;
			}
		}
	}

	if(len != size)
	{
		if(len & 0x01) len++; /* AVIs tend to have a size mismatch */
		if(len != size)
		{
			long xlen;
			skip_byte(p->buf);
			xlen = get_byte(p->buf) << 16;
			xlen |= get_byte(p->buf) << 8;
			xlen |= get_byte(p->buf); /* Read Len */
			fprintf(stderr, "CVID: END INFO chunk size %d cvid size1 %ld cvid size2 %ld\n", size, len, xlen);
		}
	}
}

