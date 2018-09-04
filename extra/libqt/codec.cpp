/* codecs.c
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
* $Id: codecs.c,v 1.60 2003/04/20 00:32:58 nhumfrey Exp $
*/
#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"
#include "colormodels.h"


static int total_vcodecs = 0;
static int total_acodecs = 0;
static oqt_video_codec_t **vcodecs = NULL; 
static oqt_audio_codec_t **acodecs = NULL; 




const oqt_codec_info_t**
oqt_list_audio_codecs()
{
	oqt_load_all_plugins();
	return (const oqt_codec_info_t**)acodecs;
}


const oqt_codec_info_t**
oqt_list_video_codecs()
{
	oqt_load_all_plugins();
	return (const oqt_codec_info_t**)vcodecs;
}

int
oqt_count_video_codecs()
{
	oqt_load_all_plugins();
	return total_vcodecs;
}

int
oqt_count_audio_codecs()
{
	oqt_load_all_plugins();
	return total_acodecs;
}





static int oqt_get_vcodec_index(const char *code)
{
	int i;

	for(i = 0; i < total_vcodecs; i++)
	{
		if(oqt_match_32(code, vcodecs[i]->info.fourcc))
			return i;
	}
	return -1;
}

static int oqt_get_acodec_index(const char *code)
{
	int i;

	for(i = 0; i < total_acodecs; i++)
	{
		if(oqt_match_32(code, acodecs[i]->info.fourcc))
			return i;
	}
	return -1;
}


/*
handle should be null for internal codecs
or handle to opened libary/module for external codecs
*/
int oqt_register_codec( oqt_codec_info_t* info)
{
	int index;

	// Make sure the info is non-null
	if(info==NULL) {
		fprintf(stderr,"oqt_register_codec: codec information pointer is NULL.\n");
		return -1;
	}

	// Make sure that the id code is valid
	if (info->fourcc[0] == 0 &&
		info->fourcc[1] == 0 &&
		info->fourcc[2] == 0 &&
		info->fourcc[3] == 0 )
	{
		fprintf(stderr,"oqt_register_codec: codec's four character code is not valid.\n");
		return -1;
	}	

	// Is it a video or an audio codec ?
	if (info->type == OQT_CODEC_TYPE_AUDIO) {
		index = oqt_get_acodec_index(info->fourcc);
		if (index>=0) {
			// already present
			if (info->merit > acodecs[index]->info.merit) {
				// replace previous low merit codec
				fprintf(stderr, "oqt_register_codec: codec [%.4s] merit %d replacing merit %d\n", info->fourcc, info->merit, acodecs[index]->info.merit);
				// XXX: memory leak
			} else if (info->merit < acodecs[index]->info.merit) {
				// ignore this registration
				fprintf(stderr, "oqt_register_codec: codec [%.4s] merit %d ignored because merit %d is higher\n", info->fourcc, info->merit, acodecs[index]->info.merit);
				// XXX: memory leak
				return 0;
			} else {
				fprintf(stderr, "oqt_register_codec: codec [%.4s] is already registered with the same merit %d\n", info->fourcc, info->merit);
				return -1;
			}
		} else {
			// Increment the number of codecs and make space for it
			index=total_acodecs++;
			acodecs = (oqt_audio_codec_t **)realloc(acodecs,
				total_acodecs*sizeof(oqt_audio_codec_t*));
			if (!acodecs) {
				perror("Error reallocating memory for acodecs.\n");
				return -1;
			}
		}

		// Cast the information pointer to an audio codec pointer
		acodecs[index] = (oqt_audio_codec_t *)info;

	} else if (info->type == OQT_CODEC_TYPE_VIDEO) {
		index = oqt_get_vcodec_index(info->fourcc);
		if (index>=0) {
			// already present
			if (info->merit > vcodecs[index]->info.merit) {
				// replace previous low merit codec
				fprintf(stderr, "oqt_register_codec: codec [%.4s] merit %d replacing merit %d\n", info->fourcc, info->merit, vcodecs[index]->info.merit);
				// XXX: memory leak
			} else if (info->merit < vcodecs[index]->info.merit) {
				// ignore this registration
				fprintf(stderr, "oqt_register_codec: codec [%.4s] merit %d ignored because merit %d is higher\n", info->fourcc, info->merit, vcodecs[index]->info.merit);
				// XXX: memory leak
				return 0;
			} else {
				fprintf(stderr, "oqt_register_codec: codec [%.4s] is already registered with the same merit %d\n", info->fourcc, info->merit);
				return -1;
			}
		} else {
			// Increment the number of codecs and make space for it
			index=total_vcodecs++;
			vcodecs = (oqt_video_codec_t **)realloc(vcodecs,
				total_vcodecs*sizeof(oqt_video_codec_t*));
			if (!vcodecs) {
				perror("Error reallocating memory for vcodecs.\n");
				return -1;
			}
		}

		// Cast the information pointer to an video codec pointer
		vcodecs[index] = (oqt_video_codec_t *)info;

	} else {
		fprintf(stderr,"oqt_register_codec: codec type is not valid.\n");
		return -1;
	}

	return 0;
}





static oqt_video_codec_t* 
find_video_codec(char *compressor)
{
	int index = -1;

	// Try already loaded codec
	index = oqt_get_vcodec_index(compressor);

	// Try loading all the codecs
	if(index < 0)
	{
		oqt_load_all_plugins();
		index = oqt_get_vcodec_index(compressor);
	}

	if(index < 0) {
#ifndef __MACOS__
		fprintf(stderr, "Error: didn't find codec in find_video_codec.\n");
#endif
		return NULL;
	}
	return vcodecs[index];
}



static oqt_audio_codec_t* 
find_audio_codec(char *compressor)
{
	int index = -1;

	// Try already loaded codec
	index = oqt_get_acodec_index(compressor);

	// Try loading all the codecs
	if(index < 0)
	{
		oqt_load_all_plugins();
		index = oqt_get_acodec_index(compressor);
	}

	if(index < 0) {
#ifndef __MACOS__
		fprintf(stderr, "Error: didn't find codec in find_audio_codec.\n");
#endif
		return NULL;
	}
	return acodecs[index];
}


const oqt_codec_param_t*
oqt_list_codec_params (
								   const oqt_codec_info_t *codec,
								   int *o_numparams)
{
	/* Is it audio or video ?*/
	if (codec->type == OQT_CODEC_TYPE_VIDEO) {
		oqt_video_codec_t* vcodec = (oqt_video_codec_t*)codec;
		return vcodec->list_params(o_numparams);
	} else if (codec->type == OQT_CODEC_TYPE_AUDIO) {
		oqt_audio_codec_t* acodec = (oqt_audio_codec_t*)codec;
		return acodec->list_params(o_numparams);
	} else {
		fprintf(stderr, "oqt_list_codec_params: Unknown codec type (%d).\n", codec->type);
	}

	return NULL;
}


int oqt_init_video_map(oqt_t *file, int track, oqt_trak_t *trak)
{
	oqt_video_map_t *vtrack = &file->vtracks[track];
	char *compressor = trak->mdia.minf.stbl.stsd.table[0].format;
	oqt_video_codec_t* codec = NULL;
	int error=0;

	/* Initialise the map */
	memset(vtrack, 0, sizeof(*vtrack));
	vtrack->track = trak;
	vtrack->current_frame = 0;
	vtrack->current_chunk = 1;

	/* Look to see if the codec is available */
	codec = find_video_codec(compressor);
	if (!codec) return -1;
	vtrack->codec = codec;

	/* Try and intitalise the codec's private data */
	if ((error = codec->init_codec(file, track)) < 1) {
		// Init should return a number greater than zero
		fprintf(stderr, "Error in oqt_init_video_map: %d.\n", error);
	}

	/* Store the name of the video compressor */
	strncpy(trak->mdia.minf.stbl.stsd.table[0].compressor_name, 
		vtrack->codec->info.name, 32);	// Max Length is 32

	return 0;
}


int oqt_init_audio_map(oqt_t *file, int track, oqt_trak_t *trak)
{
	char *compressor = trak->mdia.minf.stbl.stsd.table[0].format;
	oqt_audio_map_t *atrack = &file->atracks[track];
	oqt_audio_codec_t* codec = NULL;
	int error=0;

	// Initialise the map
	memset(atrack, 0, sizeof(*atrack));
	atrack->track = trak;
	atrack->current_sample = 0;
	atrack->current_chunk = 1;

	// Initalise the audio buffers
	atrack->decoded_buf_ptr=NULL;
	atrack->decoded_buf_size=0;
	atrack->decoded_buf_used=0;

	atrack->encoded_buf_ptr = NULL;
	atrack->encoded_buf_size = 0;
	atrack->encoded_buf_size = 0;

	// Not done any encoding yet
	atrack->encoder_preped=0;

	// Look to see if the codec is available
	codec = find_audio_codec(compressor);
	if (!codec) return -1;
	atrack->codec = codec;

	// Try and intitalise the codec's private data
	if ((error = codec->init_codec(file, track)) < 1) {
		// Init should return a number greater than zero
		fprintf(stderr, "Error in oqt_init_audio_map: %d.\n", error);
	}

	// Desactivate the Sample Size Table
	// if the sample size is known and constant
	if(atrack->codec->frames_to_bytes(file, track, 1) > 0) {
		atrack->track->mdia.minf.stbl.stsz.sample_size = atrack->codec->frames_to_bytes(file, track, 1);
	}

	return 0;
}

int oqt_delete_video_map(oqt_t *file, int track)
{
	oqt_video_map_t *vtrack = &file->vtracks[track];

	// Release the codec if neeed
	if( vtrack->codec )
	{
		int usecounter = vtrack->codec->delete_codec(file, track);

#ifdef DEBUG
		fprintf(stderr,"oqt_delete_video_map (%d)\n", usecounter);
#endif

		// *** Do we actually ever want to do this ??
		//     they don't take up much memory....
		//     and we can use it for listing codecs...
		//    There could be problems with unloading plugins
		//    shared by more than one codec... ***
		// Unload the plug-in module
		//oqt_close_plugin(vcodecs[index].plugin_handle);

		// Mark the codec as released
		vtrack->codec = NULL;

		// Did the codec release the private memory ?
		if (vtrack->codec_private)
			fprintf(stderr, "Warning: codec failed to release private memory.\n");
	}


	return 0;
}


int oqt_delete_audio_map(oqt_t *file, int track)
{
	oqt_audio_map_t *atrack = &file->atracks[track];

	// Free up work buffers
	if(atrack->decoded_buf_ptr)
		free(atrack->decoded_buf_ptr);

	if(atrack->encoded_buf_ptr)
		free(atrack->encoded_buf_ptr);

	if( atrack->codec )
	{
		int usecounter = atrack->codec->delete_codec(file, track);

#ifdef DEBUG
		fprintf(stderr,"oqt_delete_audio_map (%d)\n", usecounter);
#endif

		// *** Do we actually ever want to do this ??
		//     they don't take up much memory....
		//     and we can use it for listing codecs...
		//    There could be problems with unloading plugins
		//    shared by more than one codec... ***
		// Unload the plug-in module
		//oqt_close_plugin(vcodecs[index].plugin_handle);

		// Mark the codec as released
		atrack->codec = NULL;

		// Did the codec release the private memory ?
		if (atrack->codec_private)
			fprintf(stderr, "Warning: codec failed to release private memory.\n");
	}


	return 0;
}

int
oqt_supported_video(
								oqt_t *file, int track)
{
	char *compressor = oqt_get_video_compressor(file, track);
	if((oqt_get_vcodec_index(compressor)) < 0) return 0;
	return 1;
}


int
oqt_supported_audio(
								oqt_t *file, int track)
{
	char *compressor = oqt_get_audio_compressor(file, track);
	if((oqt_get_acodec_index(compressor)) < 0) return 0;
	return 1;
}


static int
oqt_decode_video_noconv(
						oqt_t		*file,
						int			track,
						int			color_model, 
						BYTE	**row_pointers)
{
	oqt_video_map_t	*vtrack = &(file->vtracks[track]);
	BYTE	*input = NULL;
	int			error = -1;
	int		bytes;


	// Get the size of the frame
	bytes = oqt_get_video_framesize(file, track, vtrack->current_frame);
	if(bytes<=0) {
		fprintf(stderr, "oqt_decode_video_noconv: frame size equal %u\n", bytes);
		return -1;
	}

	// This sets the correct file->data_file too
	oqt_set_video_position(file, track, vtrack->current_frame);

	input=(BYTE*)malloc(bytes);
	if(input) {
		// printf("Position in the file: %lli Frame Number %li\n", oqt_get_position(file), vtrack->current_frame);

		if(oqt_read_data(file->data_file, (char*)input, bytes))
		{ 
			error = vtrack->codec->decode(file, track, bytes, input, row_pointers, color_model); 	  
		} else  {
			fprintf(stderr, "oqt_decode_video_noconv: can't read data from file\n");
		}

		free(input);
	} else 
		fprintf(stderr, "oqt_decode_video_noconv: Can't allocate decoding buffer");

	vtrack->current_frame++;

	return error;
}

int
oqt_write_video_data(
								 oqt_t		*file, 
								 int		track,
								 BYTE	*output,
								 int		bytes,
								 int		IsAKeyFrame)
{
	int error;
	oqt_video_map_t *vtrack = &(file->vtracks[track]);
	__int64 offset = oqt_get_position(file);

	// printf("Writing %u bytes\n", bytes);
	error = oqt_write_data(file, (char*)output, bytes);

	if( IsAKeyFrame ) {
		oqt_insert_video_keyframe(file, track, vtrack->current_chunk);
	}

	oqt_update_tables(file,
		vtrack->track,
		NULL,
		offset,
		vtrack->current_chunk,
		vtrack->current_frame,
		1,
		bytes,
		1,
		NULL);

	/* Increment frame and chunk */
	vtrack->current_chunk++;
	vtrack->current_frame++;

	return error;    
}

static int
oqt_encode_video_noconv(
						oqt_t		*file, 
						int		track,
						int		color_model, 
						BYTE	**row_pointers)
{
	BYTE *output; 
	int error = -1;
	oqt_video_map_t *vtrack = &(file->vtracks[track]);
	int width, height,bytes, out_size;
	int IsAKeyFrame;

	// Initializing some local variables
	width  = oqt_get_video_width(file, track);
	height = oqt_get_video_height(file, track);

	// FIXME: A guess at the largest size an encoded frame could be
	// - might be a better way of doing this ?
	out_size = width * height * 4;

	output = (BYTE*)malloc(out_size);
	if(output) {
		bytes = vtrack->codec->encode(file, track, row_pointers, out_size, output, &IsAKeyFrame, color_model); 

		if(bytes > 0) {
			error = !oqt_write_video_data(file, track, output, bytes, IsAKeyFrame);
		} else  {
			fprintf(stderr, "oqt_encode_video_noconv: Error in external encoding function.\n");
		}

		free(output);
	} else {
		fprintf(stderr, "oqt_encode_video_noconv: Can't allocate %d byte encoding buffer.\n", out_size);
	}

	return error;    
}


int
oqt_decode_video(
							 oqt_t		*file,
							 int			track,
							 int			color_model, 
							 BYTE	**row_pointers
							 )
{
	int preferred = oqt_preferred_colormodel(file, track);
	int result;

	// Check that codec supports it - rather than segfault
	if (!file->vtracks[track].codec->decode) {
		fprintf(stderr, "oqt_decode_video: Error codec does not support decoding.\n");
		return -1;
	}

	// Maybe an unsupported bit depth or something ?
	if (preferred == BC_UNSUPPORTED) {
		fprintf(stderr, "oqt_decode_video: error codec is not able to decode this track.\n");
		return -1;
	}

	if(color_model == BC_NONE || oqt_reads_colormodel(file, track, color_model)) {
		return oqt_decode_video_noconv(file, track, color_model, row_pointers);

	} else if (!oqt_reads_colormodel(file, track, preferred)) {
		fprintf(stderr, "oqt_decode_video: Error codec does not support decoding of its prefered colour model (%d).\n",
			preferred);
		fprintf(stderr, "                  [This is a bug in the codec.]\n");
		return -1;

	} else {
		int width = oqt_get_video_width(file, track);
		int height = oqt_get_video_height(file, track);

		if(preferred == BC_YUV420P || preferred == BC_YUV422P ||
			preferred == BC_YUV444P || preferred == BC_YUV411P) {
				// planar yuv, buffers are large enough for YUV444P
				BYTE *buffer_y = (BYTE*)malloc(width*height);
				BYTE *buffer_u = (BYTE*)malloc(width*height);
				BYTE *buffer_v = (BYTE*)malloc(width*height);
				BYTE *tmp_row_pointers[3];

				tmp_row_pointers[0] = buffer_y;
				tmp_row_pointers[1] = buffer_u;
				tmp_row_pointers[2] = buffer_v;

				result = oqt_decode_video_noconv(file, track, preferred, tmp_row_pointers);

				oqt_cmodel_transfer(row_pointers, NULL,
					row_pointers[0], row_pointers[1],
					row_pointers[2], 
					tmp_row_pointers[0], tmp_row_pointers[1],
					tmp_row_pointers[2], 
					0, 0, width, height,
					0, 0, width, height,
					preferred, color_model, 0,
					width,
					width * oqt_cmodel_calculate_pixelsize(color_model));

				// Free the memory
				free(buffer_y);
				free(buffer_u);
				free(buffer_v);

				return result;

		} else if(oqt_cmodel_is_rgb(preferred) || preferred == BC_YUV422) {
			// rgb or packed YUV422
			int i;
			int size = oqt_cmodel_calculate_framesize(width, height, preferred, -1);
			int pixelsize = oqt_cmodel_calculate_pixelsize(preferred);
			BYTE *buffer = (BYTE*)malloc(size);
			BYTE **tmp_row_pointers = (BYTE**)malloc(height*sizeof(BYTE*));

			for(i = 0; i < height; ++i) {
				tmp_row_pointers[i] = buffer + (pixelsize * width * i);
			} 
			result = oqt_decode_video_noconv(file, track, preferred, tmp_row_pointers);

			oqt_cmodel_transfer(row_pointers, tmp_row_pointers,
				row_pointers[0], row_pointers[1],
				row_pointers[2], 
				NULL, NULL, NULL,
				0, 0, width, height,
				0, 0, width, height,
				preferred, color_model, 0,
				width * pixelsize,
				width * oqt_cmodel_calculate_pixelsize(color_model));

			// Free the memory
			free(tmp_row_pointers);
			free(buffer);

			return result;
		} else {
			fprintf(stderr, "oqt_decode_video: unexpected color conversion (%s -> %s).\n",
				oqt_get_cmodel_name(preferred), oqt_get_cmodel_name(color_model));
			return -1;
		}
	}
}

int
oqt_encode_video(
							 oqt_t		*file, 
							 int			track,
							 int			color_model, 
							 BYTE	**row_pointers)
{
	int preferred = oqt_preferred_colormodel(file, track);
	int result;

	// Check that codec supports it - rather than segfault
	if (!file->vtracks[track].codec->encode) {
		fprintf(stderr, "oqt_encode_video: Error codec does not support encoding.\n");
		return -1;
	}

	// Maybe an unsupported bit depth or something ?
	if (preferred == BC_UNSUPPORTED) {
		fprintf(stderr, "oqt_encode_video: error codec is not able to encode this track.\n");
		return -1;
	}

	if(oqt_writes_colormodel(file, track, color_model)) {
		return oqt_encode_video_noconv(file, track, color_model, row_pointers);

	} else if (!oqt_writes_colormodel(file, track, preferred)) {
		fprintf(stderr, "oqt_decode_video: Error codec does not support encoding of its prefered colour model (%d).\n",
			preferred);
		fprintf(stderr, "                  [This is a bug in the codec.]\n");
		return -1;

	} else {
		int width = oqt_get_video_width(file, track);
		int height = oqt_get_video_height(file, track);

		if(preferred == BC_YUV420P || preferred == BC_YUV422P ||
			preferred == BC_YUV444P || preferred == BC_YUV411P) {
				// planar yuv, buffers are large enough for YUV444P
				BYTE *buffer_y = (BYTE*)malloc(width*height);
				BYTE *buffer_u = (BYTE*)malloc(width*height);
				BYTE *buffer_v = (BYTE*)malloc(width*height);
				BYTE *tmp_row_pointers[3];
				tmp_row_pointers[0] = buffer_y;
				tmp_row_pointers[1] = buffer_u;
				tmp_row_pointers[2] = buffer_v;


				oqt_cmodel_transfer(tmp_row_pointers, row_pointers,
					tmp_row_pointers[0], tmp_row_pointers[1],
					tmp_row_pointers[2], 
					row_pointers[0], row_pointers[1],
					row_pointers[2], 
					0, 0, width, height,
					0, 0, width, height,
					color_model, preferred, 0,
					width * oqt_cmodel_calculate_pixelsize(color_model),
					width);

				result = oqt_encode_video_noconv(file, track, preferred,
					tmp_row_pointers);

				// Free the memory
				free(buffer_y);
				free(buffer_u);
				free(buffer_v);			

				return result;

		} else if(oqt_cmodel_is_rgb(preferred) || preferred == BC_YUV422) {
			// rgb or packed YUV422
			int i;
			int size = oqt_cmodel_calculate_framesize(width, height, preferred, -1);
			int pixelsize = oqt_cmodel_calculate_pixelsize(preferred);
			BYTE *buffer = (BYTE*)malloc(size);
			BYTE **tmp_row_pointers = (BYTE**)malloc(height*sizeof(BYTE*));

			for(i = 0; i < height; ++i) {
				tmp_row_pointers[i] = buffer + (pixelsize * width * i);
			} 

			oqt_cmodel_transfer(tmp_row_pointers, row_pointers,
				tmp_row_pointers[0], tmp_row_pointers[1], tmp_row_pointers[2],
				row_pointers[0], row_pointers[1], row_pointers[2], 
				0, 0, width, height,
				0, 0, width, height,
				color_model, preferred, 0,
				width * oqt_cmodel_calculate_pixelsize(color_model),
				width * pixelsize);

			result = oqt_encode_video_noconv(file, track, preferred,
				tmp_row_pointers);

			// Free the memory
			free(tmp_row_pointers);
			free(buffer);

			return result;
		} else {
			fprintf(stderr, "oqt_encode_video: unexpected color conversion (%s -> %s).\n",
				oqt_get_cmodel_name(preferred), oqt_get_cmodel_name(color_model));
			return -1;
		}
	}
}




static int decode_audio_chunk(
							  oqt_t *file,
							  int track,
							  long chunk,
							  long sd_id,
							  oqt_audio_codec_t *codec)
{
	int result = 0;
	long chunk_bytes;
	oqt_audio_map_t *atrack = &(file->atracks[track]);
	__int64 chunk_samples, offset=0;
	int channels = oqt_get_audio_channels(file, track);
	int frame_size = ((oqt_get_audio_bits(file, track)/8)*channels);


	// Get the byte count to read. 
	chunk_samples = oqt_chunk_samples(atrack->track, chunk);
	//fprintf(stderr, "%d samples in chunk number %lld (%d bytes).\n", 
	//	chunk_samples, chunk, frame_size*chunk_samples);

	// Is the the chunk already in the work buffer ?
	if(chunk == atrack->current_chunk && atrack->decoded_buf_ptr) {
		//fprintf(stderr, "Reusing old chunk %llu.\n", chunk);
		return result;
	}
	//	else fprintf(stderr, "Decoding chunk %llu.\n", chunk);


	// Get the length of the chunk in bytes
	chunk_bytes =  oqt_get_audio_chunksize(file, track, chunk);
	if (chunk_bytes==0) {
		fprintf(stderr, "decode_audio_chunk: warning chunk %d has no length.\n", chunk);
	}
	//fprintf(stderr, "Length of chunk %lld is %ld.\n", chunk, chunk_bytes);

	// Get the buffer to read into. 
	atrack->encoded_buf_size = chunk_bytes;
	atrack->encoded_buf_ptr = (char*)realloc(atrack->encoded_buf_ptr, atrack->encoded_buf_size);

	// Get the buffer to decode into
	atrack->decoded_buf_size = (int)chunk_samples*frame_size;
	atrack->decoded_buf_ptr = (char*)realloc(atrack->decoded_buf_ptr, atrack->decoded_buf_size);


	//  printf("before oqt_read_chunk\n");
	offset = oqt_chunk_to_offset(atrack->track, chunk);
	if (offset<0) {
		result = 1;
	} else {
		file->data_file = oqt_file_from_sd_id(file, atrack->track, sd_id);

		if(!file->data_file) {
			result = 1;
		} else {
			oqt_set_position(file->data_file, offset);
			result = !oqt_read_data(file->data_file, atrack->encoded_buf_ptr, chunk_bytes);
		}
	}


	// Now decode the chunk, one block at a time, until the total samples in the chunk 
	// is reached.

	if(result)	{
		fprintf(stderr, "decode_audio_chunk: failed to read chunk: %d\n", chunk);
	} else {
		//printf("chunk %d ", chunk);
		result = atrack->codec->decode(file, track, chunk_bytes, 
			(BYTE*)atrack->encoded_buf_ptr, atrack->decoded_buf_size,
			(BYTE*)atrack->decoded_buf_ptr);

		// 'result' the the number of bytes decoded
		if (result<=0) {
			fprintf(stderr, "decode_audio_chunk: failed to decode chunk: %d\n", chunk);
		} else result=0;
	}

	// Store the number of the chunk that is in the buffers
	atrack->current_chunk = chunk;

	return result;
}



long oqt_decode_audio(oqt_t *file,
															  int track, BYTE *output, long outputsize)
{
	int result = 0;
	oqt_audio_map_t *atrack = &(file->atracks[track]);
	int channels = oqt_get_audio_channels(file, track);
	int frame_size = ((oqt_get_audio_bits(file, track)/8)*channels);
	__int64 chunk_first_sample, total_length = oqt_get_audio_length(file, track);
	__int64 chunk_samples, chunk_remain;
	long chunk, block_len;
	long sd_id;
	char *block_start;
	char *buf = (char*)output;
	int buf_remain = outputsize;


	// Check that codec supports it - rather than segfault
	if (!atrack->codec->decode) {
		fprintf(stderr, "oqt_decode_audio: Error codec does not support decoding.\n");
		return -1;
	}

	// Make sure the buffer size is sensible
	if (outputsize <= 0) {
		fprintf(stderr, "Warning: output buffer is %d in oqt_decode_audio.\n", outputsize);
	}

	// Get the number of the current chunk based on our position
	// and number of the first sample in that chunk
	oqt_chunk_of_sample(&chunk_first_sample, &chunk, 
		atrack->track, &sd_id, atrack->current_sample);

	//fprintf(stderr, "chunk=%lld, chunk_first_sample=%lld, current_sample=%lld\n",
	//	   chunk, chunk_first_sample, atrack->current_sample);


	// Read chunks and extract ranges of samples until the output is full.
	// or we reach the end of the track.
	while(buf_remain > 0 && atrack->current_sample < total_length)
	{

		// Read and decode a new chunk if necessary
		result = decode_audio_chunk(file, track, chunk, sd_id, atrack->codec);
		if (result || !atrack->decoded_buf_ptr) {
			fprintf(stderr, "oqt_decode_audio: Failure decoding chunk [%d]: %d.\n", chunk, result);
			return 0;
		}


		// Get the number of samples in the current chunk
		chunk_samples = oqt_chunk_samples(atrack->track, chunk);

		// Calculate the number of bytes remaining in this chunk
		chunk_remain = (frame_size * (chunk_samples - (atrack->current_sample - chunk_first_sample)));


		// Work out block to copy from chunk into output buffer
		block_start = atrack->decoded_buf_ptr;
		if(chunk_first_sample < atrack->current_sample)
			block_start += frame_size*(atrack->current_sample - chunk_first_sample);

		block_len = buf_remain;
		if(block_len > chunk_remain)
			block_len = (long)chunk_remain;     


		//fprintf(stderr,"chunk=%lld, chunk_samples=%ld, chunk_remain=%d, block_len=%d, buf_remain=%d\n",
		//				chunk, chunk_samples, chunk_remain, block_len, buf_remain);
		//fprintf(stderr,"buf=%x, block_start=%x\n",
		//				buf, block_start);


		// Copy across the decoded block into the output buffer
		// - more effient way to do this ?!
		memcpy(buf, block_start, block_len);


		// Move on the next chunk ?
		if (chunk_remain<=buf_remain) {
			//fprintf(stderr, "Moving on to next chunk.\n");
			chunk++;
			chunk_first_sample += chunk_samples;
		}


		// Calculate new position in the buffer
		buf += block_len;
		buf_remain -= block_len;
		atrack->current_sample += (block_len/frame_size);
	}

	return ((outputsize-buf_remain)/frame_size);
}



int oqt_write_audio_data(
								 oqt_t		*file, 
								 int			track,
								 BYTE	*output,
								 int			bytes,
								 int			num_samples,
								 int			num_frames,
								 long*		frame_size_array)
{
	int error;
	oqt_audio_map_t *atrack = &(file->atracks[track]);
	__int64 offset = oqt_get_position(file);

	// Make sure chunk number is valid
	if (atrack->current_chunk<=0) 
		atrack->current_chunk = 1;

	// printf("Writing %u bytes\n", bytes);
	error = oqt_write_data(file, (char*)output, bytes);

	oqt_update_tables(file,
		atrack->track,
		NULL,
		offset,
		atrack->current_chunk,
		atrack->current_chunk-1,//atrack->current_sample,
		num_samples,
		bytes,
		num_frames,
		frame_size_array);

	/* Increment sample and chunk */
	atrack->current_sample += num_samples;
	atrack->current_chunk++;

	return error;    
}


/* Since all channels are written at the same time: 
Encode using the compressor for the first audio track. 
Which means all the audio channels must be on the same track. */

long oqt_encode_audio(oqt_t *file,int track, BYTE *input, long input_samples)
{
	oqt_audio_map_t *atrack = &(file->atracks[track]);
	long chunk_bytes=0;
	long copy_bytes;

	int channels = oqt_get_audio_channels(file, track);
	int sample_rate = oqt_get_audio_samplerate(file, track);
	int frame_size = channels * (oqt_get_audio_bits(file, track)/8);
	long input_bytes = input_samples * frame_size;

	/*
	fprintf(stderr, "Passed in %d samples, %d bytes.\n", input_samples, input_bytes);
	*/

	// Check that codec supports it - rather than segfault
	if (!atrack->codec->encode) {
		fprintf(stderr, "oqt_encode_audio: Error codec does not support encoding.\n");
		return -1;
	}


	// Prepare encoder if not done already //
	if (!atrack->encoder_preped) {
		if (atrack->codec->prepare_encoder) {
			atrack->samples_per_chunk = atrack->codec->prepare_encoder(file, track);
			if (atrack->samples_per_chunk < 0) {
				fprintf(stderr, "oqt_encode_audio: Codec does not support audio format.\n");
				return -1;
			}
		} else {
			atrack->samples_per_chunk = 0;
		}
		atrack->encoder_preped=1;
	}



	/* If codec did not give a value then calculate it ourselves
	using the default technique - make each chunk 0.25 seconds */
	if (atrack->samples_per_chunk == 0 ) {
		atrack->samples_per_chunk = sample_rate / 4;
	}
	chunk_bytes = atrack->samples_per_chunk * frame_size;



	/***
	FIXME: Look out for the case where too much data is passed in
	***/
	if (chunk_bytes < input_bytes) {
		fprintf(stderr, "oqt_encode_audio: **** Can't currently take more bytes than will fit in a chunk ****\n");
		return -1;
	}

	/* Allocate buffers - make encoded buffer as big as the decoded buffer
	- we can assume that it isn't going to be any bigger than it was ?
	*/
	if(!atrack->decoded_buf_ptr || atrack->decoded_buf_size < chunk_bytes)
	{
		atrack->decoded_buf_size = chunk_bytes;
		atrack->decoded_buf_ptr = (char*)realloc( atrack->decoded_buf_ptr, chunk_bytes);
	}
	if(!atrack->encoded_buf_ptr || atrack->encoded_buf_size < chunk_bytes)
	{
		atrack->encoded_buf_size = chunk_bytes;
		atrack->encoded_buf_ptr = (char*)realloc( atrack->encoded_buf_ptr, chunk_bytes);
	}


	/* Add the input samples to the raw data buffer */
	copy_bytes = input_bytes;
	if (copy_bytes > (atrack->decoded_buf_size - atrack->decoded_buf_used)) {
		copy_bytes = (atrack->decoded_buf_size - atrack->decoded_buf_used);
	}
	if (copy_bytes > 0) {
		memcpy(atrack->decoded_buf_ptr+atrack->decoded_buf_used, input, copy_bytes);
		atrack->decoded_buf_used += copy_bytes;
	}


	/*fprintf(stderr, "Got %d/%d bytes of a %d sample chunk.\n",
	atrack->decoded_buf_used,
	atrack->decoded_buf_size,
	atrack->samples_per_chunk );*/


	/* Have we got enough bytes to encode a chunk yet ? */
	if (atrack->decoded_buf_used == atrack->decoded_buf_size || 
		input_samples == 0 ) {
			int chunk_samples = atrack->decoded_buf_used/frame_size;
			int result;

			/* If input_samples is zero then it forces a chunk to be created
			- used for 'flushing' at the end of encoding */
			fprintf(stderr, "Got enough for a chunk! (%d/%d bytes)\n",
				atrack->decoded_buf_used,
				atrack->decoded_buf_size);

			atrack->encoded_buf_used = atrack->codec->encode
				(file, track,
				chunk_samples,				// Number of samples to encode
				atrack->decoded_buf_ptr, 	// Input Buffer
				atrack->encoded_buf_size,	// Size of output buffer
				atrack->encoded_buf_ptr);	// Output Buffer

			fprintf(stderr, "Encoded %d bytes from %d samples.\n",
				atrack->encoded_buf_used,
				chunk_samples );


			/* Write the encoded data to the file */
			result = oqt_write_audio_data(file, track,
				(PBYTE)atrack->encoded_buf_ptr,
				atrack->encoded_buf_used,
				chunk_samples,
				1,
				NULL);

			/* Reset the raw data buffer now that it has been enocded */
			atrack->decoded_buf_used = 0;
	}



	/* Move any left over data to the end of the chunk */
	if (copy_bytes < input_bytes) {
		int extra_bytes = input_bytes - copy_bytes;
		fprintf(stderr, "Copying %d bytes of unused raw data into buffer.\n", extra_bytes);

		memcpy(atrack->decoded_buf_ptr, input+copy_bytes, extra_bytes);
		atrack->decoded_buf_used += extra_bytes;
	}

	/* Return the number of samples that were used - (moved to work buffer) */

	/* *** Do something more intelgent here *** */
	return input_samples;
}

int oqt_reads_colormodel(oqt_t *file, int track, int colormodel )
{
	return (file->vtracks[track].codec)->reads_colormodel(file, track, colormodel);
}

int oqt_writes_colormodel(oqt_t *file, int track, int colormodel )
{
	return (file->vtracks[track].codec)->writes_colormodel(file, track, colormodel);
}

int oqt_preferred_colormodel(oqt_t *file, int track)
{
	return (file->vtracks[track].codec)->preferred_colormodel(file, track);
}



int
oqt_set_video_param(oqt_t *file, int track,
								const char* param, const void* data)
{
	oqt_video_map_t *vtrack = &(file->vtracks[track]);

	return vtrack->codec->set_param(file,track,param,data);
}


int
oqt_get_video_param(oqt_t *file, int track,
								const char* param, void* data)
{
	oqt_video_map_t *vtrack = &(file->vtracks[track]);

	return vtrack->codec->get_param(file,track,param,data);
}


int
oqt_set_audio_param(oqt_t *file, int track,
								const char* param, const void* data)
{
	oqt_audio_map_t *atrack = &(file->atracks[track]);

	return atrack->codec->set_param(file,track,param,data);
}


int
oqt_get_audio_param(oqt_t *file, int track,
								const char* param, void* data)
{
	oqt_audio_map_t *atrack = &(file->atracks[track]);

	return atrack->codec->get_param(file,track,param,data);
}


int
oqt_write_video_reference(
									  oqt_t		*file, 
									  int		track,
									  char		*url,
									  __int64	offset,
									  int		bytes,
									  int		IsAKeyFrame)
{
	int error;
	oqt_video_map_t *vtrack = &(file->vtracks[track]);

	if( IsAKeyFrame ) {
		oqt_insert_video_keyframe(file, track, vtrack->current_chunk);
	}

	error = oqt_update_tables(file,
		vtrack->track,
		url,
		offset,
		vtrack->current_chunk,
		vtrack->current_frame,
		1,
		bytes,
		1,
		NULL);

	/* Increment frame and chunk */
	vtrack->current_chunk++;
	vtrack->current_frame++;

	return error;    
}


int
oqt_write_audio_reference(
									  oqt_t		*file, 
									  int		track,
									  char		*url,
									  __int64	offset,
									  int		bytes,
									  int		num_samples)
{
	int error;
	oqt_audio_map_t *atrack = &(file->atracks[track]);

	// Make sure chunk number is valid
	if (atrack->current_chunk<=0) 
		atrack->current_chunk = 1;

	error = oqt_update_tables(file,
		atrack->track,
		url,
		offset,
		atrack->current_chunk,
		atrack->current_chunk-1,//atrack->current_sample,
		num_samples,
		bytes,
		1,
		NULL);

	/* Increment sample and chunk */
	atrack->current_sample += num_samples;
	atrack->current_chunk++;

	return error;    
}
