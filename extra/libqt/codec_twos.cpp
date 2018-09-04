#include "stdafx.h"
#include "qtcodec.h"

void* init_twos(WAVEFORMATEX* wfx){return (void*)'twos';}
void delete_twos(void* code){}


__forceinline int swap_bytes(BYTE *buffer_in, BYTE *buffer_out, long samples, int bits)
{
	long i = 0;

	switch(bits)
	{
	case 8:
		for(i=0;i < samples; i+=1)
		{
			buffer_out[i] = buffer_in[1];
		}
		break;

	case 16:
		for(i=0; i < samples * 2; i+=2)
		{
			buffer_out[i+1] = buffer_in[i];
			buffer_out[i] = buffer_in[i+1];
		}
		break;

	case 24:
		for(i=0; i < samples * 3; i+=3)
		{
			buffer_out[i+2] = buffer_in[i];
			buffer_out[i+1] = buffer_in[i+1];
			buffer_out[i] = buffer_in[i+2];
		}
		break;

	default:
		break;
	}
	return 0;
}

int decode_twos(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize)
{
	int bits = wfx->wBitsPerSample;
	swap_bytes(input, output, inputsize/(bits/8),bits);
	return inputsize;
}


int decode_sowt(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize)
{
	memcpy(output,input,inputsize);
	return inputsize;
}

static short alaw_decode [128] =
{	 -5504,  -5248,  -6016,  -5760,  -4480,  -4224,  -4992,  -4736, 
	 -7552,  -7296,  -8064,  -7808,  -6528,  -6272,  -7040,  -6784, 
	 -2752,  -2624,  -3008,  -2880,  -2240,  -2112,  -2496,  -2368, 
	 -3776,  -3648,  -4032,  -3904,  -3264,  -3136,  -3520,  -3392, 
	-22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944, 
	-30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136, 
	-11008, -10496, -12032, -11520,  -8960,  -8448,  -9984,  -9472, 
	-15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568, 
	  -344,   -328,   -376,   -360,   -280,   -264,   -312,   -296, 
	  -472,   -456,   -504,   -488,   -408,   -392,   -440,   -424, 
	   -88,    -72,   -120,   -104,    -24,     -8,    -56,    -40, 
	  -216,   -200,   -248,   -232,   -152,   -136,   -184,   -168, 
	 -1376,  -1312,  -1504,  -1440,  -1120,  -1056,  -1248,  -1184, 
	 -1888,  -1824,  -2016,  -1952,  -1632,  -1568,  -1760,  -1696, 
	  -688,   -656,   -752,   -720,   -560,   -528,   -624,   -592, 
	  -944,   -912,  -1008,   -976,   -816,   -784,   -880,   -848
} ; /* alaw_decode */

__forceinline void	alaw2s_array (BYTE *buffer, ULONG count, short *ptr, ULONG index)
{
	for (ULONG k = 0 ; k < count ; k++)
	{	
		if (buffer [k] & 0x80)
			ptr [index] = -1 * alaw_decode [((int) buffer [k]) & 0x7F] ;
		else
			ptr [index] = alaw_decode [((int) buffer [k]) & 0x7F] ;
		index ++ ;
	}
} /* alaw2s_array */

int decode_alaw(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize)
{
	if(wfx->wBitsPerSample!=16) return -1;
	alaw2s_array(input, inputsize, (short*)output, 0);
	return inputsize*2;
}



int decode_raw(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize)
{
	for(ULONG i = 0; i < inputsize; i++)
		output[i] = (BYTE)input[i] - 0x80;
	return inputsize;
}



#define uBIAS 0x84
#define uCLIP 32635

typedef struct
{
	short *ulawtoint16_table;
	short *ulawtoint16_ptr;
} Param;



void* init_ulaw(WAVEFORMATEX* wfx)
{
	Param *p = (Param *)malloc(sizeof(Param));
	p->ulawtoint16_table=0;
	p->ulawtoint16_ptr=0;
	// Initalise tables

	if(!p->ulawtoint16_table)
	{
		static int exp_lut[8] = { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };
		int sign, exponent, mantissa, sample;
		unsigned char ulawbyte;

		p->ulawtoint16_table = (short*)malloc(sizeof(short) * 256);
		p->ulawtoint16_ptr = p->ulawtoint16_table;
		for(int i = 0; i < 256; i++)
		{
			ulawbyte = (unsigned char)i;
			ulawbyte = ~ulawbyte;
			sign = (ulawbyte & 0x80);
			exponent = (ulawbyte >> 4) & 0x07;
			mantissa = ulawbyte & 0x0F;
			sample = exp_lut[exponent] + (mantissa << (exponent + 3));
			if(sign != 0) sample = -sample;
			p->ulawtoint16_ptr[i] = sample;
		}
	}
	return p;
}

void delete_ulaw(void* codec)
{
	Param *p = (Param*)codec;

	if (p) 
	{
		if(p->ulawtoint16_table) free(p->ulawtoint16_table); 
		free(p);
	}
}


int decode_ulaw(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize)
{
	Param *p = (Param*)codec;
	short *output_ptr = (short*)output;
	for(ULONG i=0;i<inputsize;i++) 
		output_ptr[i] = p->ulawtoint16_ptr[input[i]];
	return inputsize*2;
}


