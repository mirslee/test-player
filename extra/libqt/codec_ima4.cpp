#include "stdafx.h"
#include "qtcodec.h"

// Known by divine revelation 
#define BLOCK_SIZE 0x22
#define SAMPLES_PER_BLOCK 0x40

static int ima4_step[89] = 
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int ima4_index[16] = 
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

typedef struct
{
	short *work_buffer;
	int *last_samples, *last_indexes;
} Param;


void* init_ima4(WAVEFORMATEX* wfx)
{
	int i;
	int channels = wfx->nChannels;
	Param *p = (Param*)malloc(sizeof(Param));
	p->work_buffer = (short*)malloc(SAMPLES_PER_BLOCK * sizeof(short*));
	p->last_samples = (int*)malloc(sizeof(int) * channels);
	for(i = 0; i < channels; i++) p->last_samples[i] = 0;

	// Allocate last index buffer
	p->last_indexes = (int*)malloc(sizeof(int) * channels);
	for(i = 0; i < channels; i++) p->last_indexes[i] = 0;
	return p;
}


void delete_ima4(void* codec)
{
	Param *p = (Param*)codec;
	if (p) 
	{
		if(p->work_buffer) free(p->work_buffer);
		if(p->last_samples) free(p->last_samples);
		if(p->last_indexes) free(p->last_indexes);
		free(p);
	}
}

static void ima4_decode_sample(int *predictor, int *nibble, int *index, int *step)
{
	int difference, sign;

	// Get new index value 
	*index += ima4_index[*nibble];

	if(*index < 0) *index = 0; 
	else 
		if(*index > 88) *index = 88;

	// Get sign and magnitude from *nibble 
	sign = *nibble & 8;
	*nibble = *nibble & 7;

	// Get difference 
	difference = *step >> 3;
	if(*nibble & 4) difference += *step;
	if(*nibble & 2) difference += *step >> 1;
	if(*nibble & 1) difference += *step >> 2;

	// Predict value 
	if(sign) 
		*predictor -= difference;
	else 
		*predictor += difference;

	if(*predictor > 32767) *predictor = 32767;
	else
		if(*predictor < -32768) *predictor = -32768;

	// Update the step value 
	*step = ima4_step[*index];
}

static void ima4_decode_block(short *output, BYTE *input)
{
	int predictor, index, step;
	int nibble, nibble_count;
	BYTE *input_end = input + BLOCK_SIZE;

	// Get the chunk header 
	predictor = *input++ << 8;
	predictor |= *input++;

	index = predictor & 0x7f;
	if(index > 88) index = 88;

	predictor &= 0xff80;
	if(predictor & 0x8000) predictor -= 0x10000;
	step = ima4_step[index];

	// Read the input buffer sequentially, one nibble at a time 
	nibble_count = 0;
	while(input < input_end)
	{
		nibble = nibble_count ? (*input++  >> 4) & 0x0f : *input & 0x0f;

		ima4_decode_sample(&predictor, &nibble, &index, &step);
		*output++ = predictor;
		nibble_count ^= 1;
	}
}

int decode_ima4(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize)
{
	Param *p = (Param*)codec;
	int channels = wfx->nChannels;
	int samples =  (inputsize * SAMPLES_PER_BLOCK) / BLOCK_SIZE;
	short *output_ptr = (short*)output;
	unsigned char* block_ptr = input;
	int i,j,k;

	for(i = 0; i < samples; i += (SAMPLES_PER_BLOCK*channels))
	{
		for(j=0; j < channels; j++) 
		{
			ima4_decode_block(p->work_buffer, block_ptr);
			block_ptr += BLOCK_SIZE;

			for(k=0; k<SAMPLES_PER_BLOCK; k++) 
			{
				output_ptr[i+(k*channels)+j] = p->work_buffer[k];
			}
		}
	}
	return i*(wfx->nChannels/8);
}

