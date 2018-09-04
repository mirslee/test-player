#include "stdafx.h"
#include "qtcodec.h"
#include <emmintrin.h>


void* init_yuv2(ImageDescriptionPtr desc,int* colormodel)
{
	*colormodel = BC_YUV422;
	return (void*)'yuv2';
}
void delete_yuv2(void* code)
{
}


int decode_yuv2(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch)
{
	int width 	= desc->width;
	int height 	= desc->height;
	int spitch = width*2/16;
	int dpitch = pitch/16;
	__m128i addi = 	_mm_set1_epi32(0x80008000);
	__m128i* src = (__m128i*)input;
	__m128i* dst = (__m128i*)output;
	while(height--)
	{
		__m128i* __dst = dst;
		int w = spitch;
		while(w--)
			*__dst++ = _mm_adds_epu8(*src++,addi);
		dst += dpitch;
	}
	_mm_empty();
	return 0;
}
