#include "stdafx.h"
#include "qtcodec.h"
#include "..\libfaad\faad.h"

/* Each instance of the codec should have its own Param structure allocated */
typedef struct {
	int use_esds;
	int decoder_initialized;
	faacDecHandle hDecoder;
	unsigned long samplerate;
	unsigned char channels;
} Param;


void* init_faac(WAVEFORMATEX* wfx)
{
	Param *p = (Param *)malloc(sizeof(Param));
	memset(p, 0, sizeof(Param));
	return p;
}

void* init_mp4a(WAVEFORMATEX* wfx)
{
	Param *p = (Param *)malloc(sizeof(Param));
	memset(p, 0, sizeof(Param));
	p->use_esds = 1;
	return p;
}

void delete_faac(void* codec)
{
	Param *p = (Param*)codec;

	if (p) {
		if(p->hDecoder) {
			faacDecClose(p->hDecoder);
		}
		free(p);
	}
}

int decode_faac(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize)
{
	Param *p = (Param*)codec;
	int inputpos = 0, outputpos = 0;
	faacDecFrameInfo frameInfo;

	if(!p->decoder_initialized) 
	{
		p->decoder_initialized = 1;
		p->hDecoder = faacDecOpen();

		if(p->use_esds) 
		{
			DWORD* dw = (DWORD*)(wfx+1);
			oqt_esds_t* esds = (oqt_esds_t*)(dw+2);
			if(!dw[1]||!esds->decoderConfigLen) 
			{
				p->samplerate = wfx->nSamplesPerSec;
				p->channels = (BYTE)wfx->nChannels;
			} 
			else 
			{
				if(faacDecInit2(p->hDecoder, esds->decoderConfig,esds->decoderConfigLen,&p->samplerate, &p->channels) < 0) 
				{
					p->samplerate = wfx->nSamplesPerSec;
					p->channels = (BYTE)wfx->nChannels;
				}
			}

			if(wfx->nSamplesPerSec!=p->samplerate ||
				wfx->nChannels!=p->channels) {
					fprintf(stderr, "decode_faac: decoder config does not match movie header\n");
			}
		} 
		else
		{
			int res;

			res = faacDecInit(p->hDecoder, input,inputsize, &p->samplerate, &p->channels);
			if(res < 0) {
				fprintf(stderr, "decode_faac: failed to parse decoder config\n");
			}

			if(wfx->nSamplesPerSec!=p->samplerate ||
				wfx->nChannels!=p->channels) {
					fprintf(stderr, "decode_faac: decoder config does not match movie header\n");
			}

			inputpos = res;
		}
	}

	while(inputpos < (int)inputsize && outputpos < (int)outsize) {
		int len;
		void *sample_buffer;

		/* this seems to help a lot */
		while(inputpos < (int)inputsize && input[inputpos] == 0) {
			++inputpos;
		}

		sample_buffer = faacDecDecode(p->hDecoder, &frameInfo, input+inputpos,inputsize-inputpos);
		if (frameInfo.error > 0) {
			CString str;
			str.Format("decode_faac error: %s\n",faacDecGetErrorMessage(frameInfo.error));
			OutputDebugString(str);

			return outputpos;
		}

		inputpos += frameInfo.bytesconsumed;
		len = p->channels*frameInfo.samples;
		if(outputpos+len > (int)outsize) {
			fprintf(stderr, "decode_faac: would overflow sample buffer!\n");
			return outputpos;
		}
		memcpy(output+outputpos, sample_buffer, len);
		outputpos += len;
	}

	return outputpos;
}
