/*******************************************************************************
 lqt_codecs.c

 libquicktime - A library for reading and writing quicktime/avi/mp4 files.
 http://libquicktime.sourceforge.net

 Copyright (C) 2002 Heroine Virtual Ltd.
 Copyright (C) 2002-2007 Members of the libquicktime project.

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 2.1 of the License, or (at your option)
 any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along
 with this library; if not, write to the Free Software Foundation, Inc., 51
 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*******************************************************************************/ 

#include "lqt_private.h"
#include "quicktime/colormodels.h"
#define LQT_LIBQUICKTIME
#include <quicktime/lqt_codecapi.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#define LOG_DOMAIN "codecs"

static int quicktime_delete_vcodec_stub(quicktime_video_map_t *vtrack)
{
        lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
                "quicktime_delete_vcodec_stub called");
	return 0;
}

static int quicktime_delete_acodec_stub(quicktime_audio_map_t *atrack)
{
        lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
                "quicktime_delete_acodec_stub called");
	return 0;
}

static int quicktime_decode_video_stub(quicktime_t *file, 
				unsigned char **row_pointers, 
				int track)
{
        lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
                "quicktime_decode_video_stub called");
	return 1;
}

static int quicktime_encode_video_stub(quicktime_t *file, 
				unsigned char **row_pointers, 
				int track)
{
        lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
                "quicktime_encode_video_stub called");
	return 1;
}

static int quicktime_decode_audio_stub(quicktime_t *file, 
                                       void * output,
                                       long samples,
                                       int track)
{
        lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
                "quicktime_decode_audio_stub called");
	return 0;
}

static int quicktime_encode_audio_stub(quicktime_t *file, 
                                       void * input,
                                       long samples,
                                       int track)
{
        lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
                "quicktime_encode_audio_stub called");
	return 1;
}

static int quicktime_flush_codec_stub(quicktime_t *file, int track)
{
return 0;
}


static int quicktime_codec_defaults(quicktime_codec_t *codec)
{
	codec->delete_vcodec = quicktime_delete_vcodec_stub;
	codec->delete_acodec = quicktime_delete_acodec_stub;
	codec->decode_video = quicktime_decode_video_stub;
	codec->encode_video = quicktime_encode_video_stub;
	codec->decode_audio = quicktime_decode_audio_stub;
	codec->encode_audio = quicktime_encode_audio_stub;
	codec->flush = quicktime_flush_codec_stub;
	return 0;
}

/*
 *  Original quicktime4linux function changed for dynamic loading
 */

int quicktime_init_vcodec(quicktime_video_map_t *vtrack, int encode,
                          lqt_codec_info_t * codec_info)
  {
  lqt_codec_info_t ** codec_array = (lqt_codec_info_t**)0;
  
  lqt_init_video_codec_func_t init_codec;
  lqt_init_video_codec_func_t (*get_codec)(int);
    
  void * module = (void*)0;
  
  char *compressor = vtrack->track->mdia.minf.stbl.stsd.table[0].format;

  vtrack->codec = calloc(1, sizeof(quicktime_codec_t));
  quicktime_codec_defaults((quicktime_codec_t*)vtrack->codec);

  ((quicktime_codec_t*)vtrack->codec)->module = (void*)0;
  
  /* Try to find the codec */

  if(!codec_info)
    {
    
    codec_array = lqt_find_video_codec(compressor, encode);
  
    if(!codec_array)
      {
      lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN, "Could not find video %s for fourcc %4s",
              (encode ? "Encoder" : "Decoder"), compressor);
      return -1;
      }
    codec_info = *codec_array;
    }

  vtrack->compatibility_flags = codec_info->compatibility_flags;
  
  lqt_log(NULL, LQT_LOG_DEBUG, LOG_DOMAIN,
          "Loading module %s", codec_info->module_filename);
  
  /* dlopen the module */
  module = dlopen(codec_info->module_filename, RTLD_NOW);
  
  if(!module)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN, "Loading module %s failed: %s",
            codec_info->module_filename, dlerror());
    
    if(codec_array)
      lqt_destroy_codec_info(codec_array);

    return -1;
    }
  
  ((quicktime_codec_t*)vtrack->codec)->codec_name =
    malloc(strlen(codec_info->name)+1);
  strcpy(((quicktime_codec_t*)vtrack->codec)->codec_name, codec_info->name);
  
  /* Set the module */
  
  ((quicktime_codec_t*)vtrack->codec)->module = module;
  
  /* Get the codec finder for the module */
  
  get_codec =
    (lqt_init_video_codec_func_t(*)(int))dlsym(module,
                                               "get_video_codec");
  
  if(!get_codec)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
            "Module %s contains no function get_video_codec",
            codec_info->module_filename);
    if(codec_array)
      lqt_destroy_codec_info(codec_array);

    return -1;
    }
  
  /* Obtain the initializer for the actual codec */
  
  init_codec = get_codec(codec_info->module_index);
  
  init_codec(vtrack);
  if(codec_array)
    lqt_destroy_codec_info(codec_array);

  //  vtrack->stream_cmodel = lqt_get_decoder_colormodel(quicktime_t * file, int track);
  
  return 0;
  
  }

int quicktime_init_acodec(quicktime_audio_map_t *atrack, int encode,
                          lqt_codec_info_t * codec_info)
  {
  lqt_codec_info_t ** codec_array = (lqt_codec_info_t**)0;
  lqt_init_audio_codec_func_t init_codec;
  lqt_init_audio_codec_func_t (*get_codec)(int);
    
  void * module;

  char *compressor = atrack->track->mdia.minf.stbl.stsd.table[0].format;
  int wav_id       = atrack->track->mdia.minf.stbl.stsd.table[0].compression_id;
  atrack->codec = calloc(1, sizeof(quicktime_codec_t));
  quicktime_codec_defaults((quicktime_codec_t*)atrack->codec);

  ((quicktime_codec_t*)(atrack->codec))->module = (void*)0;
  
  /* Try to find the codec */

  if(!codec_info)
    {
    

    if(compressor && (*compressor != '\0'))
      {
      codec_array = lqt_find_audio_codec(compressor, encode);
      }
    else if(wav_id)
      {
      codec_array = lqt_find_audio_codec_by_wav_id(wav_id, encode);
      }
    if(!codec_array)
      {
      lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN, "Could not find audio %s for fourcc %4s",
              (encode ? "Encoder" : "Decoder"), compressor);
      return -1;
      }
    codec_info = *codec_array;
    }

  atrack->compatibility_flags = codec_info->compatibility_flags;
  
  lqt_log(NULL, LQT_LOG_DEBUG, LOG_DOMAIN,
          "Loading module %s", codec_info->module_filename);
  
  /* dlopen the module */
  module = dlopen(codec_info->module_filename, RTLD_NOW);
  
  if(!module)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN, "Loading module %s failed: %s",
            codec_info->module_filename, dlerror());
    if(codec_array)
      lqt_destroy_codec_info(codec_array);
    return -1;
    }

  ((quicktime_codec_t*)atrack->codec)->codec_name = malloc(strlen(codec_info->name)+1);
  strcpy(((quicktime_codec_t*)atrack->codec)->codec_name, codec_info->name);
    
  /* Set the module */
  
  ((quicktime_codec_t*)((quicktime_codec_t*)atrack->codec))->module = module;
  
  /* Get the codec finder for the module */
  
  get_codec =
    (lqt_init_audio_codec_func_t(*)(int))dlsym(module,
                                               "get_audio_codec");
  
  if(!get_codec)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
            "Module %s contains no function get_audio_codec",
            codec_info->module_filename);
    if(codec_array)
      lqt_destroy_codec_info(codec_array);
    return -1;
    }
  
  /* Obtain the initializer for the actual codec */

  init_codec = get_codec(codec_info->module_index);
  
  init_codec(atrack);

  /* We set the wav ids from our info structure, so we don't have to do this
     in the plugin sources */
  
  if(codec_info->num_wav_ids)
    atrack->wav_id = codec_info->wav_ids[0];
  
  if(codec_array)
    lqt_destroy_codec_info(codec_array);
  return 0;
  }


int quicktime_delete_vcodec(quicktime_video_map_t *vtrack)
  {
  ((quicktime_codec_t*)vtrack->codec)->delete_vcodec(vtrack);
  /* Close the module */
  
  if(((quicktime_codec_t*)vtrack->codec)->module)
    dlclose(((quicktime_codec_t*)vtrack->codec)->module);
  if(((quicktime_codec_t*)vtrack->codec)->codec_name)
    free(((quicktime_codec_t*)vtrack->codec)->codec_name);
  free(vtrack->codec);
  vtrack->codec = 0;
  return 0;
}

int quicktime_delete_acodec(quicktime_audio_map_t *atrack)
  {
  ((quicktime_codec_t*)atrack->codec)->delete_acodec(atrack);
  /* Close the module */
  if(((quicktime_codec_t*)atrack->codec)->module)
    dlclose(((quicktime_codec_t*)atrack->codec)->module);
  if(((quicktime_codec_t*)atrack->codec)->codec_name)
    free(((quicktime_codec_t*)atrack->codec)->codec_name);
  free(atrack->codec);
  atrack->codec = 0;
  return 0;
  }

int quicktime_supported_video(quicktime_t *file, int track)
{
	char *compressor = quicktime_video_compressor(file, track);
        lqt_codec_info_t ** test_codec =
          lqt_find_video_codec(compressor, file->wr);
        if(!test_codec)
          return 0;
        
        lqt_destroy_codec_info(test_codec);
	return 1;
}

int quicktime_supported_audio(quicktime_t *file, int track)
{
         lqt_codec_info_t ** test_codec;
         char *compressor = quicktime_audio_compressor(file, track);

         test_codec = (lqt_codec_info_t**)0;
         
         if(compressor && (*compressor != '\0'))
           test_codec = lqt_find_audio_codec(compressor, file->wr);
         else if(lqt_is_avi(file))
           test_codec = lqt_find_audio_codec_by_wav_id(lqt_get_wav_id(file, track), file->wr);
         
         if(!test_codec)
           return 0;
        
        lqt_destroy_codec_info(test_codec);
        return 1;
}

void lqt_update_frame_position(quicktime_video_map_t * track)
  {
  track->timestamp +=
    track->track->mdia.minf.stbl.stts.table[track->stts_index].sample_duration;

  track->stts_count++;

  if(track->stts_count >=
     track->track->mdia.minf.stbl.stts.table[track->stts_index].sample_count)
    {
    track->stts_index++;
    track->stts_count = 0;
    }
  track->current_position++;
  }

/* Set the io_rowspan for the case the user didn't. */

static void set_default_rowspan(quicktime_t *file, int track)
  {
  if(file->vtracks[track].io_row_span)
    return;

  lqt_get_default_rowspan(file->vtracks[track].io_cmodel,
                          quicktime_video_width(file, track),
                          &(file->vtracks[track].io_row_span),
                          &(file->vtracks[track].io_row_span_uv));
  }




/* Copy audio */

int lqt_copy_audio(int16_t ** dst_i, float ** dst_f,
                   int16_t ** src_i, float ** src_f,
                   int dst_pos, int src_pos,
                   int dst_size, int src_size, int num_channels)
  {
  
  int i, j, i_tmp;
  int samples_to_copy;
  samples_to_copy = src_size < dst_size ? src_size : dst_size;

  
  if(src_i)
    {
    for(i = 0; i < num_channels; i++)
      {
      if(dst_i && dst_i[i]) /* int -> int */
        {
        memcpy(dst_i[i] + dst_pos, src_i[i] + src_pos, samples_to_copy * sizeof(int16_t));
        }
      if(dst_f && dst_f[i]) /* int -> float */
        {
        for(j = 0; j < samples_to_copy; j++)
          {
          dst_f[i][dst_pos + j] = (float)src_i[i][src_pos + j] / 32767.0;
          }
        }
      }
    }
  else if(src_f)
    {
    for(i = 0; i < num_channels; i++)
      {
      if(dst_i && dst_i[i]) /* float -> int */
        {
        for(j = 0; j < samples_to_copy; j++)
          {
          i_tmp = (int)(src_f[i][src_pos + j] * 32767.0);
          
          if(i_tmp > 32767)
            i_tmp = 32767;

          if(i_tmp < -32768)
            i_tmp = -32768;
          
          dst_i[i][dst_pos + j] = i_tmp;
          }
        }
      if(dst_f && dst_f[i]) /* float -> float */
        {
        memcpy(dst_f[i] + dst_pos, src_f[i] + src_pos, samples_to_copy * sizeof(float));
        }
      }
    }
  return samples_to_copy;
  }

