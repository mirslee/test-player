#include "stdafx.h"
#include "qtcodec.h"


void* init_8BPS(ImageDescriptionPtr desc,int* colormodel)
{
	*colormodel = BC_BGRA8888;
	return (void*)'8BPS';
}


void delete_8BPS(void* codec)
{
}

int decode_8BPS(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch)
{
	int width = desc->width;
	int height = desc->height;
	int x, y, len;
	BYTE *r_lp, *g_lp, *b_lp;
	BYTE *rp, *gp, *bp;


	// Line Pointers for each Plane 
	r_lp = input;
	g_lp = r_lp + (height << 1);
	b_lp = g_lp + (height << 1);

	/* Start of Red Rows */
	rp = b_lp + (height << 1);

	/* Calculate Size of Compressed Red Plane */
	gp = r_lp; len = 0;
	for(y = 0; y < height; y++) { len += ((gp[0] << 8) | gp[1]); gp += 2; }

	/* Start of Green Rows */
	gp = rp + len;

	/* Calculate Size of Compressed Green Plane */
	bp = g_lp; len = 0;
	for(y = 0; y < height; y++) { len += ((bp[0] << 8) | bp[1]); bp += 2; }

	/* Start of Blue Rows */
	bp = gp + len;
	for(y=0; y<height; y++) 
	{
		BYTE *row = output+y*pitch;

		/* Red PLANE */
		len = r_lp[ (y*2) ] << 8;  len |= r_lp[ (y*2) + 1 ];
		for(x=0; (x < width) && (len > 0);)
		{
			BYTE code = *rp++; len--;
			if (code <= 127) { code++; len -= code;
			while(code--) { row[(x*4)+0] = *rp++; x++; } }
			else if (code > 128) { code = (long)(257) - code;
			while(code--) { row[(x*4)+0] = *rp; x++; } rp++; len--; }
		}

		/* Green PLANE */
		len = g_lp[ (y*2) ] << 8;  len |= g_lp[ (y*2) + 1 ];
		for(x=0; (x < width) && (len > 0);)
		{
			BYTE code = *gp++; len--;
			if (code <= 127) { code++; len -= code;
			while(code--) { row[(x*4)+1] = *gp++; x++; } }
			else if (code > 128) { code = (long)(257) - code;
			while(code--) { row[(x*4)+1] = *gp; x++; } gp++; len--; }
		}

		/* Blue PLANE */
		len = b_lp[ (y*2) ] << 8;  len |= b_lp[ (y*2) + 1 ];
		for(x=0; (x < width) && (len > 0);)
		{
			BYTE code = *bp++; len--;
			if (code <= 127) { code++; len -= code;
			while(code--) { row[(x*4)+2] = *bp++; x++; } }
			else if (code > 128) { code = (long)(257) - code;
			while(code--) { row[(x*4)+2] = *bp; x++; } bp++; len--; }
		}
	}
	return 0;
}
