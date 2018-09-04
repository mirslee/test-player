/*******************************************************************************
 lqt_quicktime.c

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
#include "stdafx.h"
#include "lqt_private.h"
#include "colormodels.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <strings.h>
#include <errno.h>
#define LQT_LIBQUICKTIME
#include "lqt_codecapi.h"

#define LOG_DOMAIN "core"

#pragma warning(disable:4996 4244)


void quicktime_set_copyright(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.copyright), &(file->moov.udta.copyright_len), string);
}

void quicktime_set_name(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.name), &(file->moov.udta.name_len), string);
}

void quicktime_set_info(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.info), &(file->moov.udta.info_len), string);
}

char* quicktime_get_copyright(quicktime_t *file)
{
	return file->moov.udta.copyright;
}

char* quicktime_get_name(quicktime_t *file)
{
	return file->moov.udta.name;
}

char* quicktime_get_info(quicktime_t *file)
{
	return file->moov.udta.info;
}

/* Extended metadata support */

void lqt_set_album(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.album), &(file->moov.udta.album_len), string);
}

void lqt_set_artist(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.artist), &(file->moov.udta.artist_len), string);
}

void lqt_set_genre(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.genre), &(file->moov.udta.genre_len), string);
}

void lqt_set_track(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.track), &(file->moov.udta.track_len), string);
}

void lqt_set_comment(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.comment), &(file->moov.udta.comment_len), string);
}

void lqt_set_author(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.author), &(file->moov.udta.author_len), string);
}

void lqt_set_creation_time(quicktime_t *file, unsigned int time)
  {
  file->moov.mvhd.creation_time = time;
  }

char * lqt_get_album(quicktime_t * file)
{
        return file->moov.udta.album;
}

char * lqt_get_artist(quicktime_t * file)
{
        return file->moov.udta.artist;
}

char * lqt_get_genre(quicktime_t * file)
{
        return file->moov.udta.genre;
}

char * lqt_get_track(quicktime_t * file)
{
        return file->moov.udta.track;
}

char * lqt_get_comment(quicktime_t *file)
{
        return file->moov.udta.comment;
}

char * lqt_get_author(quicktime_t *file)
{
        return file->moov.udta.author;
}

unsigned int lqt_get_creation_time(quicktime_t * file)
  {
  return file->moov.mvhd.creation_time;
  }


int quicktime_video_tracks(quicktime_t *file)
{
	int i, result = 0;
	for(i = 0; i < file->moov.total_tracks; i++)
	{
		if(file->moov.trak[i]->mdia.minf.is_video) result++;
	}
	return result;
}

int quicktime_audio_tracks(quicktime_t *file)
{
	int i, result = 0;
	quicktime_minf_t *minf;
	for(i = 0; i < file->moov.total_tracks; i++)
	{
		minf = &(file->moov.trak[i]->mdia.minf);
		if(minf->is_audio)
			result++;
	}
	return result;
}

int lqt_add_audio_track(quicktime_t *file,int channels, int sample_rate, int bits, char * compressor)
{
	quicktime_trak_t *trak;
	/* Fake the bits parameter for some formats. */
	if(quicktime_match_32(compressor,QUICKTIME_ULAW)||quicktime_match_32(compressor, QUICKTIME_IMA4))
		bits = 16;
	else if(quicktime_match_32(compressor,QUICKTIME_RAW))
		bits = 8;

	file->atracks = (quicktime_audio_map_t*)realloc(file->atracks, (file->total_atracks+1)*sizeof(quicktime_audio_map_t));
	memset(&(file->atracks[file->total_atracks]), 0, sizeof(quicktime_audio_map_t));

	trak = quicktime_add_track(file);
	quicktime_trak_init_audio(file, trak, channels,sample_rate, bits, compressor);
	quicktime_init_audio_map(&(file->atracks[file->total_atracks]),trak);
	file->atracks[file->total_atracks].track = trak;
	file->atracks[file->total_atracks].channels = channels;
	file->atracks[file->total_atracks].current_position = 0;
	file->atracks[file->total_atracks].current_chunk = 1;
	file->total_atracks++;
	return file->total_atracks-1;
}


int lqt_add_video_track(quicktime_t *file,int frame_w, int frame_h,int frame_duration, int timescale,char * compressor)
{
	quicktime_trak_t *trak;
    if(!file->total_vtracks)
		quicktime_mhvd_init_video(file, &(file->moov.mvhd), timescale);

    file->vtracks = (quicktime_video_map_t*)realloc(file->vtracks, (file->total_vtracks+1) * sizeof(quicktime_video_map_t));
    memset(&(file->vtracks[file->total_vtracks]), 0, sizeof(quicktime_video_map_t));
    trak = quicktime_add_track(file);

    file->total_vtracks++;
    
    quicktime_trak_init_video(file, trak, frame_w, frame_h, frame_duration, timescale, compressor);
	quicktime_init_video_map(&(file->vtracks[file->total_vtracks-1]), trak);
    return file->total_vtracks-1;
}




void quicktime_set_framerate(quicktime_t *file, double framerate)
{
	int i;
	int new_time_scale, new_sample_duration;

	if(!file->wr)
	{
		lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
                        "quicktime_set_framerate shouldn't be called in read mode.");
		return;
	}

	new_time_scale = quicktime_get_timescale(framerate);
	new_sample_duration = (int)((float)new_time_scale / framerate + 0.5);

	for(i = 0; i < file->total_vtracks; i++)
	{
		file->vtracks[i].track->mdia.mdhd.time_scale = new_time_scale;
		file->vtracks[i].track->mdia.minf.stbl.stts.table[0].sample_duration = new_sample_duration;
	}
}

/* Used for writing only */
quicktime_trak_t* quicktime_add_track(quicktime_t *file)
  {
  quicktime_moov_t *moov = &(file->moov);
  quicktime_trak_t *trak;
  
  //  for(i = moov->total_tracks; i > 0; i--)
  //    moov->trak[i] = moov->trak[i - 1];
  
  trak =
    moov->trak[moov->total_tracks] =
    (quicktime_trak_t*)calloc(1, sizeof(quicktime_trak_t));

  quicktime_trak_init(trak, file->file_type);

  moov->trak[moov->total_tracks]->tkhd.track_id = moov->mvhd.next_track_id;
  
  moov->total_tracks++;
  moov->mvhd.next_track_id++;
  return trak;
  }

/* ============================= Initialization functions */

int quicktime_init(quicktime_t *file)
{
	memset(file,0,sizeof(quicktime_t));
//	quicktime_atom_write_header64(new_file, &file->mdat.atom, "mdat");
	quicktime_moov_init(&(file->moov));
	return 0;
}

static int quicktime_delete(quicktime_t *file)
{
	int i;
	if(file->total_atracks) 
	{
		for(i = 0; i < file->total_atracks; i++)
			quicktime_delete_audio_map(&(file->atracks[i]));
		free(file->atracks);
	}
	if(file->total_vtracks)
	{
		for(i = 0; i < file->total_vtracks; i++)
			quicktime_delete_video_map(&(file->vtracks[i]));
		free(file->vtracks);
	}
	if(file->total_ttracks)
	{
                for(i = 0; i < file->total_ttracks; i++)
			lqt_delete_text_map(file, &(file->ttracks[i]));
		free(file->ttracks);
	}
	file->total_atracks = 0;
	file->total_vtracks = 0;

        if(file->moov_data)
          free(file->moov_data);
        
        if(file->preload_size)
	{
		free(file->preload_buffer);
		file->preload_size = 0;
	}
	quicktime_moov_delete(&(file->moov));
	quicktime_mdat_delete(&(file->mdat));
	quicktime_ftyp_delete(&(file->ftyp));
	return 0;
}

/* =============================== Optimization functions */

int quicktime_set_cpus(quicktime_t *file, int cpus)
{
	return 0;
}

void quicktime_set_preload(quicktime_t *file, int64_t preload)
{
        file->preload_size = preload;
        if(file->preload_buffer) free(file->preload_buffer);
        file->preload_buffer = 0;
        if(preload)
                file->preload_buffer = (uint8_t*)calloc(1, preload);
        file->preload_start = 0;
        file->preload_end = 0;
        file->preload_ptr = 0;
}


int quicktime_get_timescale(double frame_rate)
{
	int timescale = frame_rate*100;
/* Encode the 29.97, 23.976, 59.94 framerates */
	if(frame_rate - (int)frame_rate != 0) 
		timescale = (int)(frame_rate * 1001 + 0.5);
	return timescale;
}



int64_t quicktime_audio_length(quicktime_t *file, int track)
{
	if(file->total_atracks > 0) 
		return quicktime_track_samples(file, file->atracks[track].track);

	return 0;
}

int quicktime_video_length(quicktime_t *file, int track)
{
/*printf("quicktime_video_length %d %d\n", quicktime_track_samples(file, file->vtracks[track].track), track); */
	if(file->total_vtracks > 0)
		return quicktime_track_samples(file, file->vtracks[track].track);
	return 0;
}


int quicktime_has_audio(quicktime_t *file)
  {
  if(quicktime_audio_tracks(file)) return 1;
  return 0;
  }

int quicktime_sample_rate(quicktime_t *file, int track)
  {
  if(file->total_atracks)
    return file->atracks[track].samplerate;
  return 0;
  }

int quicktime_audio_bits(quicktime_t *file, int track)
{
	if(file->total_atracks)
		return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].sample_size;

	return 0;
}

char* quicktime_audio_compressor(quicktime_t *file, int track)
{
	return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].format;
}

int quicktime_track_channels(quicktime_t *file, int track)
{
	if(track < file->total_atracks)
		return file->atracks[track].channels;

	return 0;
}

int quicktime_channel_location(quicktime_t *file, int *quicktime_track, int *quicktime_channel, int channel)
{
	int current_channel = 0, current_track = 0;
	*quicktime_channel = 0;
	*quicktime_track = 0;
	for(current_channel = 0, current_track = 0; current_track < file->total_atracks; )
	{
		if(channel >= current_channel)
		{
			*quicktime_channel = channel - current_channel;
			*quicktime_track = current_track;
		}

		current_channel += file->atracks[current_track].channels;
		current_track++;
	}
	return 0;
}

int quicktime_has_video(quicktime_t *file)
{
	if(quicktime_video_tracks(file)) return 1;
	return 0;
}

int quicktime_video_width(quicktime_t *file, int track)
{
//	if(file->total_vtracks)
//          return file->vtracks[track].track->tkhd.track_width;
//	return 0;
  if((track < 0) || (track >= file->total_vtracks))
    return 0;
  return file->vtracks[track].track->mdia.minf.stbl.stsd.table->width;
  
}

int quicktime_video_height(quicktime_t *file, int track)
{
//	if(file->total_vtracks)
//          return file->vtracks[track].track->tkhd.track_height;
//	return 0;
  if((track < 0) || (track >= file->total_vtracks))
    return 0;
  return file->vtracks[track].track->mdia.minf.stbl.stsd.table->height;
}

int quicktime_video_depth(quicktime_t *file, int track)
{
	if(file->total_vtracks)
		return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].depth;
	return 0;
}


void quicktime_set_depth(quicktime_t *file, int depth, int track)
{
	int i;

	for(i = 0; i < file->total_vtracks; i++)
	{
		file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].depth = depth;
	}
}

void quicktime_frame_rate(quicktime_t *file, int track,int& rate,int& scale)
{
	__int64 samplecount = 0, duration = 0;
	if(track>=0&&track<file->total_vtracks)
	{
		quicktime_stts_t* stts = &(file->vtracks[track].track->mdia.minf.stbl.stts);
		for(int i = 0; i < stts->total_entries; i++)
		{
			if(samplecount < stts->table[i].sample_count) 
			{
				samplecount = stts->table[i].sample_count;
				duration = stts->table[i].sample_duration;
			}
		}
		rate = file->vtracks[track].track->mdia.mdhd.time_scale;
		scale = duration;
	}
}


/*
 *  Return the Duration of the entire track
 */

int64_t lqt_video_duration(quicktime_t * file, int track)
  {
  int64_t dummy1;
  int64_t dummy2;
  

  return
    quicktime_sample_to_time(&(file->vtracks[track].track->mdia.minf.stbl.stts), -1,
                             &dummy1, &dummy2);
  }


/*
 *  Get the timescale of the track. Divide the return values
 *  of lqt_frame_duration and lqt_frame_time by the scale to
 *  get the time in seconds.
 */
  
int lqt_video_time_scale(quicktime_t * file, int track)
  {
  if(file->total_vtracks <= track)
    return 0;
  return file->vtracks[track].track->mdia.mdhd.time_scale;
  }

/*
 *  Get the duration of the NEXT frame to be decoded.
 *  If constant is not NULL it will be set to 1 if the
 *  frame duration is constant throughout the whole track
 */

int lqt_frame_duration(quicktime_t * file, int track, int *constant)
  {
  if(file->total_vtracks <= track)
    return 0;

      int64_t stts_index = 0, stts_count = 0;
  if(constant)
    {
        if(file->vtracks[track].track->mdia.minf.stbl.stts.total_entries == 1)
          *constant = 1;
        else if((file->vtracks[track].track->mdia.minf.stbl.stts.total_entries == 2) && 
                (file->vtracks[track].track->mdia.minf.stbl.stts.table[1].sample_count == 1))
          *constant = 1;
        else
          *constant = 0;
        if(!(*constant))
            quicktime_sample_to_time(&(file->vtracks[track].track->mdia.minf.stbl.stts),file->vtracks[track].current_position,&stts_index,&stts_count);
    }
    return file->vtracks[track].track->mdia.minf.stbl.stts.table[stts_index].sample_duration;
  }

char* quicktime_video_compressor(quicktime_t *file, int track)
{
	if ((track < 0) || (track >= file->total_vtracks))
	   return NULL;
	return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].format;
}

int quicktime_write_audio(quicktime_t *file,uint8_t *audio_buffer,int samples,int track)
{
        int result;
        int64_t bytes;
        quicktime_atom_t chunk_atom;
        quicktime_audio_map_t *track_map = &file->atracks[track];
        quicktime_trak_t *trak = track_map->track;
                                                                                                                  
/* write chunk for 1 track */
        bytes = samples * quicktime_audio_bits(file, track) / 8 * track_map->channels;
        quicktime_write_chunk_header(file, trak, &chunk_atom);
        result = !quicktime_write_data(file, audio_buffer, bytes);
        quicktime_write_chunk_footer(file,trak,track_map->current_chunk,&chunk_atom,samples);
                                                                                                                  
/*      file->atracks[track].current_position += samples; */
        file->atracks[track].current_chunk++;
        return result;
}

int quicktime_write_frame(quicktime_t *file,unsigned char *video_buffer,int64_t bytes,int keyframe,int track)
{
	int result = 0;
	quicktime_atom_t chunk_atom;
	quicktime_video_map_t *vtrack = &file->vtracks[track];
	quicktime_trak_t *trak = vtrack->track;
                                                                                                                  
	quicktime_write_chunk_header(file, trak, &chunk_atom);
	result = !quicktime_write_data(file, video_buffer, bytes);
	quicktime_write_chunk_footer(file,trak,vtrack->current_chunk,&chunk_atom,1);
	if(keyframe) quicktime_insert_keyframe(file,vtrack->current_chunk-1,track);

	//       if(file->vtracks[track].current_position)
	quicktime_update_stts(&file->vtracks[track].track->mdia.minf.stbl.stts,
						file->vtracks[track].current_position,
						file->vtracks[track].track->mdia.minf.stbl.stts.default_duration);

	if(file->vtracks[track].timecode_track)
		lqt_flush_timecode(file, track,file->vtracks[track].current_position*(int64_t)file->vtracks[track].track->mdia.minf.stbl.stts.default_duration, 0);

	file->vtracks[track].current_position++;
	file->vtracks[track].current_chunk++;
	return result;
}

int quicktime_write_reference_audio(quicktime_t *file,int64_t offset,int samples,int track)
{
	int64_t bytes;
	quicktime_audio_map_t *track_map = &file->atracks[track];
	quicktime_trak_t *trak = track_map->track;

	/* write chunk for 1 track */
	bytes = samples * quicktime_audio_bits(file, track) / 8 * track_map->channels;

	quicktime_write_reference_chunk(file,trak,track_map->current_chunk,offset,bytes,samples);

	/*      file->atracks[track].current_position += samples; */
	file->atracks[track].current_chunk++;
	return 0;
}

int quicktime_write_reference_video(quicktime_t *file,int64_t offset,int64_t bytes,int keyframe,int track)
{
	quicktime_video_map_t *vtrack = &file->vtracks[track];
	quicktime_trak_t *trak = vtrack->track;

	quicktime_write_reference_chunk(file,trak,vtrack->current_chunk,offset,bytes,1);
	if(keyframe) quicktime_insert_keyframe(file,vtrack->current_chunk-1,track);

	quicktime_update_stts(&file->vtracks[track].track->mdia.minf.stbl.stts,
		file->vtracks[track].current_position,
		file->vtracks[track].track->mdia.minf.stbl.stts.default_duration);

	if(file->vtracks[track].timecode_track)
		lqt_flush_timecode(file, track,file->vtracks[track].current_position*(int64_t)file->vtracks[track].track->mdia.minf.stbl.stts.default_duration, 0);

	file->vtracks[track].current_position++;
	file->vtracks[track].current_chunk++;
	return 0;
}

int quicktime_frame_size(quicktime_t *file, int frame, int track)
  {
  int bytes = 0;
  quicktime_trak_t *trak = file->vtracks[track].track;

  if(trak->mdia.minf.stbl.stsz.sample_size)
    {
    bytes = trak->mdia.minf.stbl.stsz.sample_size;
    }
  else
    {
    int total_frames = quicktime_track_samples(file, trak);
    if(frame < 0) frame = 0;
    else
      if(frame > total_frames - 1) frame = total_frames - 1;
    bytes = trak->mdia.minf.stbl.stsz.table[frame].size;
    }


  return bytes;
  }


int quicktime_get_keyframe_before(quicktime_t *file, int frame, int track)
{
	quicktime_trak_t *trak = file->vtracks[track].track;
	quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;
	quicktime_stps_t *stps = &trak->mdia.minf.stbl.stps;
	int lo, hi;
	if(stps->table&&(stps->total_entries>0))
	{
		lo = 0;
		hi = stps->total_entries-1;
		if (stps->table[lo].sample-1> frame) goto syncsample;
		if (stps->table[hi].sample-1<=frame) return stps->table[hi].sample-1;
		while (hi>lo+1) {
			/* here: stps->table[lo].sample-1<=frame
			stps->table[hi].sample-1> frame */	       
			int med = (lo+hi)/2;
			if (stps->table[med].sample-1<=frame) lo = med; else hi = med;
		}
		/* here: hi=lo+1 */
		return stps->table[lo].sample-1;
	}
syncsample:
	if(!stss->table||stss->total_entries==0) return frame;


	lo = 0;
	hi = stss->total_entries-1;
	if (stss->table[lo].sample-1> frame) return -1;
	if (stss->table[hi].sample-1<=frame) return stss->table[hi].sample-1;
	while (hi>lo+1) {
		/* here: stss->table[lo].sample-1<=frame
		stss->table[hi].sample-1> frame */	       
		int med = (lo+hi)/2;
		if (stss->table[med].sample-1<=frame) lo = med; else hi = med;
	}
	/* here: hi=lo+1 */
	return stss->table[lo].sample-1;
}

#if 0
static int quicktime_get_keyframe_after(quicktime_t *file, int frame, int track)
{
        quicktime_trak_t *trak = file->vtracks[track].track;
        quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;
                                                                                                                  
                                                                                                                  
                                                                                                                  
                                                                                                                  
                                                                                                                  
// Offset 1
        frame++;
                                                                                                                  
                                                                                                                  
        for(i = 0; i < stss->total_entries; i++)
        {
                if(stss->table[i].sample >= frame) return stss->table[i].sample - 1;
        }
                                                                                                                  
        return 0;
}
#endif

void quicktime_insert_keyframe(quicktime_t *file, int frame, int track)
{
	quicktime_trak_t *trak = file->vtracks[track].track;
	quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;
	quicktime_stps_t *stps = &trak->mdia.minf.stbl.stps;
	int i;

	frame++;
        
	if(stss->total_entries==0)
	{
		stss->table[0].sample = frame;
		stss->total_entries++;
		return;
	}
// Get the keyframe greater or equal to new frame
	for(i = stps->total_entries; i >0; i--)
	{
		if(stps->table[i-1].sample < frame) break;
	}

// Expand table
	if(stps->entries_allocated <= stps->total_entries)
	{
		stps->entries_allocated *= 2;
		stps->table = (quicktime_stps_table_t*)realloc(stps->table, sizeof(quicktime_stps_table_t) * stps->entries_allocated);
	}

// Insert before existing frame
	if(i < stps->total_entries)
	{
		if(stps->table[i].sample > frame)
		{
			int j, k;
			for(j = stps->total_entries, k = stps->total_entries - 1;
				k >= i;
				j--, k--)
			{
				stps->table[j] = stps->table[k];
			}
			stps->table[i].sample = frame;
		}
	}
	else
// Insert after last frame
		stps->table[i].sample = frame;

	stps->total_entries++;
}


int quicktime_has_keyframes(quicktime_t *file, int track)
{
	quicktime_trak_t *trak = file->vtracks[track].track;
	quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;
	
	return stss->total_entries > 0;
}






int quicktime_init_video_map(quicktime_video_map_t *vtrack,quicktime_trak_t *trak)
{
	vtrack->track = trak;
	vtrack->current_position = 0;
	vtrack->current_chunk = 1;
	return 0;
}

int quicktime_delete_video_map(quicktime_video_map_t *vtrack)
{
    if(vtrack->timecodes)
          free(vtrack->timecodes);
	return 0;
}

int quicktime_init_audio_map(quicktime_audio_map_t *atrack, quicktime_trak_t *trak)
  {
  atrack->track = trak;
  atrack->channels = trak->mdia.minf.stbl.stsd.table[0].channels;
  atrack->samplerate = (int)(trak->mdia.minf.stbl.stsd.table[0].samplerate + 0.5);
  atrack->current_position = 0;
  atrack->current_chunk = 1;
  return 0;
  }

int quicktime_delete_audio_map(quicktime_audio_map_t *atrack)
{
     return 0;
}

// Initialize maps, for reading only

void quicktime_init_maps(quicktime_t * file)
  {
  int i, j, k, dom, track;
  /* get tables for all the different tracks */
  file->total_atracks = quicktime_audio_tracks(file);

  if(file->total_atracks)
    {
    file->atracks = (quicktime_audio_map_t *)calloc(1, sizeof(*file->atracks) * file->total_atracks);
    for(i = 0, track = 0; i < file->total_atracks; i++, track++)
      {
		  while(!file->moov.trak[track]->mdia.minf.is_audio)
			track++;
		  quicktime_init_audio_map(&(file->atracks[i]), file->moov.trak[track]);
      }
    }

  file->total_vtracks = quicktime_video_tracks(file);

  if(file->total_vtracks)
    {
    file->vtracks = (quicktime_video_map_t *)calloc(1, sizeof(*file->vtracks) * file->total_vtracks);
    
    for(track = 0, i = 0; i < file->total_vtracks; i++, track++)
      {
      while(!file->moov.trak[track]->mdia.minf.is_video)
        track++;
      
      quicktime_init_video_map(&(file->vtracks[i]), file->moov.trak[track]);
      
      /* Get interlace mode */
      if((file->vtracks[i].interlace_mode == LQT_INTERLACE_NONE) &&
         (file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].has_fiel))
        {
        dom = file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].fiel.dominance;
        if (file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].fiel.fields == 2)
          {
          if (dom == 14 || dom == 6)
            file->vtracks[i].interlace_mode = LQT_INTERLACE_BOTTOM_FIRST;
          else if (dom == 9 || dom == 1)
            file->vtracks[i].interlace_mode = LQT_INTERLACE_TOP_FIRST;
          }
        }
      /* Timecode track */
      if(file->moov.trak[track]->has_tref)
        {
        for(j = 0; j < file->moov.trak[track]->tref.num_references; j++)
          {
          /* Track reference has type tmcd */
          if(quicktime_match_32(file->moov.trak[track]->tref.references[j].type, "tmcd"))
            {
            for(k = 0; k < file->moov.total_tracks; k++)
              {
              if(file->moov.trak[track]->tref.references[j].tracks[0] ==
                 file->moov.trak[k]->tkhd.track_id)
                {
                file->vtracks[i].timecode_track = file->moov.trak[k];
                break;
                }
              }
            break;
            }
          }
        }
      
      }
    }

  /* Text tracks */

  file->total_ttracks = lqt_text_tracks(file);

  if(file->total_ttracks)
    {
    file->ttracks = (quicktime_text_map_t *)calloc(file->total_ttracks, sizeof(*file->ttracks));

    for(track = 0, i = 0; i < file->total_ttracks; i++, track++)
      {
      while(!file->moov.trak[track]->mdia.minf.is_text)
        track++;
      lqt_init_text_map(file,
                        &file->ttracks[i], file->moov.trak[track], 0);
      }
    }
  
  

  }

void quicktime_read_moov_preload(quicktime_t *file, quicktime_atom_t *atom)
{
	/* Set preload and preload the moov atom here */
	int64_t start_position = quicktime_position(file);
	int temp_size = atom->end - start_position;
	int readsize = (temp_size < 0x100000) ? 0x100000 : temp_size;
	unsigned char *temp = (unsigned char*)malloc(readsize);
	quicktime_set_preload(file, readsize);
	quicktime_read_data(file, temp, temp_size);
	quicktime_set_position(file, start_position);
	quicktime_read_moov(file, &(file->moov), atom);
	free(temp);
}

int quicktime_read_info(quicktime_t *file)
{
        int result = 0, got_header = 0;
        int64_t start_position = quicktime_position(file);
	quicktime_atom_t leaf_atom = {0};

		quicktime_set_position(file, 0LL);
                                                                                                                  
		file->file_type = LQT_FILE_NONE;
                                                                                                                  
        quicktime_set_position(file, 0LL);

        do
        {
                result = quicktime_atom_read_header(file, &leaf_atom);
                                                                                                          
                if(!result)
                {
                        if(quicktime_atom_is(&leaf_atom, "mdat"))
                        {
                                quicktime_read_mdat(file, &(file->mdat), &leaf_atom);
                        }
                        else
                        if(quicktime_atom_is(&leaf_atom, "ftyp"))
                        {
                                quicktime_read_ftyp(file, &file->ftyp, &leaf_atom);
                                file->file_type = quicktime_ftyp_get_file_type(&file->ftyp);
                                file->has_ftyp = 1;
                        }
                        else
                        if(quicktime_atom_is(&leaf_atom, "moov"))
                        {
							quicktime_read_moov_preload(file,&leaf_atom);
							got_header = 1;
                        }
						else
						if (quicktime_atom_is(&leaf_atom, "free"))
						{
							int64_t start_position = quicktime_position(file);
							quicktime_atom_t testatom = { 0 };
							int freeresult = quicktime_atom_read_header(file, &testatom);
							if (!freeresult && quicktime_atom_is(&testatom, "moov"))
							{ 
								quicktime_set_position(file, start_position);
								quicktime_read_moov_preload(file, &leaf_atom);
								got_header = 1;
							}
							else
								quicktime_atom_skip(file, &leaf_atom);
						}
						else
							quicktime_atom_skip(file, &leaf_atom);
                }
        }while(!result && quicktime_position(file) < file->total_length);
		
		/* read QTVR sample atoms -- object */
		if (lqt_qtvr_get_object_track(file) >= 0)
		{
			quicktime_qtatom_t leaf_atom, root_atom;
			int64_t start_position = quicktime_position(file);
			quicktime_set_position(file, file->moov.trak[lqt_qtvr_get_object_track(file)]->mdia.minf.stbl.stco.table[0].offset);
			quicktime_qtatom_read_container_header(file);
			/* root qtatom "sean" */
			quicktime_qtatom_read_header(file, &root_atom);
			
			do
			{
				quicktime_qtatom_read_header(file, &leaf_atom);
				if(quicktime_qtatom_is(&leaf_atom, "obji"))
				{
					quicktime_read_obji(file, &file->qtvr_node[0].obji);					
				}     
				else
				if(quicktime_qtatom_is(&leaf_atom, "ndhd"))
				{
					quicktime_read_ndhd(file, &file->qtvr_node[0].ndhd);					
				}     
				else
                                        quicktime_qtatom_skip(file, &leaf_atom);
			} while(quicktime_position(file) < root_atom.end);
			
			quicktime_set_position(file, start_position);
		}
		
		/* read QTVR sample atoms  -- panorama */
		if (lqt_qtvr_get_panorama_track(file) >= 0 && lqt_qtvr_get_qtvr_track(file) >= 0)
		{
			quicktime_qtatom_t leaf_atom, root_atom;
			int64_t start_position = quicktime_position(file);
			quicktime_set_position(file, file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stco.table[0].offset);
			quicktime_qtatom_read_container_header(file);
			/* root qtatom "sean" */
			quicktime_qtatom_read_header(file, &root_atom);
			
			do
			{
				quicktime_qtatom_read_header(file, &leaf_atom);
				if(quicktime_qtatom_is(&leaf_atom, "pdat"))
				{
					quicktime_read_pdat(file, &file->qtvr_node[0].pdat);					
				}     
				else
				if(quicktime_qtatom_is(&leaf_atom, "ndhd"))
				{
					quicktime_read_ndhd(file, &file->qtvr_node[0].ndhd);					
				}     
				else
                                        quicktime_qtatom_skip(file, &leaf_atom);
			} while(quicktime_position(file) < root_atom.end);
			
			quicktime_set_position(file, start_position);
		}
		
		if (lqt_qtvr_get_qtvr_track(file) >= 0)
		{
			quicktime_qtatom_t leaf_atom, root_atom;
			int64_t start_position = quicktime_position(file);
			quicktime_set_position(file, file->moov.trak[lqt_qtvr_get_qtvr_track(file)]->mdia.minf.stbl.stco.table[0].offset);
			quicktime_qtatom_read_container_header(file);
			/* root qtatom "sean" */
			quicktime_qtatom_read_header(file, &root_atom);
			
			do
			{
				quicktime_qtatom_read_header(file, &leaf_atom);
				if(quicktime_qtatom_is(&leaf_atom, "ndhd"))
				{
					quicktime_read_ndhd(file, &file->qtvr_node[0].ndhd);					
				}     
				else
                                        quicktime_qtatom_skip(file, &leaf_atom);
			} while(quicktime_position(file) < root_atom.end);
			
			quicktime_set_position(file, start_position);
		}
/* go back to the original position */
        quicktime_set_position(file, start_position);
                                                                                                                  
        
/* Initialize track map objects */
        if(got_header)
        {
                quicktime_init_maps(file);
        }

        /* Set file type if no ftyp is there */
        if(file->file_type == LQT_FILE_NONE)
          file->file_type = LQT_FILE_QT_OLD;
        
/* Shut down preload in case of an obsurdly high temp_size */
        quicktime_set_preload(file, 0);
                                                                                                                  
        return !got_header;
}


int quicktime_dump(quicktime_t *file)
{
	lqt_dump("quicktime_dump\n");
        if(file->has_ftyp)
          quicktime_ftyp_dump(&(file->ftyp));
	
        lqt_dump("movie data (mdat)\n");
	lqt_dump(" size %"PRId64"\n", file->mdat.atom.size);
	lqt_dump(" start %"PRId64"\n", file->mdat.atom.start);
	quicktime_moov_dump(&(file->moov));
	if (lqt_qtvr_get_object_track(file) >= 0)
	{
		quicktime_obji_dump(&(file->qtvr_node[0].obji));
	}
	if (lqt_qtvr_get_panorama_track(file) >= 0)
	{
		quicktime_pdat_dump(&(file->qtvr_node[0].pdat));
	}
	if (lqt_qtvr_get_qtvr_track(file) >= 0)
	{
		quicktime_ndhd_dump(&(file->qtvr_node[0].ndhd));
	}
	lqt_dump("dumpflush");
	return 0;
}



int quicktime_close(quicktime_t *file)
{
	int result = 0;
	if(file->wr)
	{
//		quicktime_codecs_flush(file);
		                                                                                                          
		if (lqt_qtvr_get_object_track(file) >= 0)
		{
			lqt_qtvr_add_object_node(file);
		}
		else if (lqt_qtvr_get_panorama_track(file) >= 0)
		{
			lqt_qtvr_add_panorama_node(file);
		}
		// Atoms are only written here
		quicktime_atom_write_footer(file, &file->mdat.atom);
		quicktime_finalize_moov(file, &(file->moov),0);
		quicktime_write_moov(file, &(file->moov));
	}
	quicktime_delete(file);
	return result;
}




int quicktime_major()
{
        return QUICKTIME_MAJOR;
}

int quicktime_minor()
{
        return QUICKTIME_MINOR;
}

int quicktime_release()
{
        return QUICKTIME_RELEASE;
}

int quicktime_div3_is_key(unsigned char *data, int size)
  {
  int result = 0;

// First 2 bits are pict type.
  result = (data[0] & 0xc0) == 0;


  return result;
  }


int lqt_get_wav_id(quicktime_t *file, int track)
  {
  quicktime_trak_t * trak;
  trak = file->atracks[track].track;
  return trak->mdia.minf.stbl.stsd.table[0].compression_id;
  }

int64_t * lqt_get_chunk_sizes(quicktime_t * file, quicktime_trak_t *trak)
  {
  int i, j;
  int64_t * ret;
  int64_t next_offset;
  int num_chunks;
  int num_tracks;
  int * chunk_indices;
  
  num_chunks = trak->mdia.minf.stbl.stco.total_entries;
  ret = (int64_t*)calloc(num_chunks, sizeof(int64_t));

  num_tracks = file->moov.total_tracks;

  chunk_indices = (int*)malloc(num_tracks * sizeof(int));

  for(i = 0; i < num_tracks; i++)
    {
    chunk_indices[i] = 0;
    }
  
  for(i = 0; i < num_chunks; i++)
    {
    next_offset = -1;
    for(j = 0; j < num_tracks; j++)
      {
      if(chunk_indices[j] < 0)
        continue;

      while(file->moov.trak[j]->mdia.minf.stbl.stco.table[chunk_indices[j]].offset <=
            trak->mdia.minf.stbl.stco.table[i].offset)
        {
        if(chunk_indices[j] >= file->moov.trak[j]->mdia.minf.stbl.stco.total_entries - 1)
          {
          chunk_indices[j] = -1;
          break;
          }
        else
          chunk_indices[j]++;
        }
      if(chunk_indices[j] < 0)
        continue;
      if((next_offset == -1) ||
         (file->moov.trak[j]->mdia.minf.stbl.stco.table[chunk_indices[j]].offset < next_offset))
        next_offset = file->moov.trak[j]->mdia.minf.stbl.stco.table[chunk_indices[j]].offset;
      }
    if(next_offset > 0)
      {
      ret[i] = next_offset - trak->mdia.minf.stbl.stco.table[i].offset;
      }
    else /* Last chunk: Take the end of the mdat atom */
      {
      ret[i] = file->mdat.atom.start +  file->mdat.atom.size - trak->mdia.minf.stbl.stco.table[i].offset;
      if(ret[i] < 0)
        ret[i] = 0;
      }
    }
  free(chunk_indices);
  return ret;
  }

int lqt_read_audio_chunk(quicktime_t * file, int track,
                         int chunk,
                         uint8_t ** buffer, int * buffer_alloc, int * samples)
  {
  int64_t offset;
  quicktime_trak_t * trak;
  int result;

  trak = file->atracks[track].track;

  if(chunk > trak->mdia.minf.stbl.stco.total_entries)
    {
    /* Read beyond EOF */
    file->atracks[track].eof = 1;
    return 0;
    }
  if(!trak->chunk_sizes)
    {
    trak->chunk_sizes = lqt_get_chunk_sizes(file, trak);
    }
  if(samples)
    *samples = quicktime_chunk_samples(trak, chunk);
  /* Reallocate buffer */

  if(*buffer_alloc < trak->chunk_sizes[chunk-1] + 16)
    {
    *buffer_alloc = trak->chunk_sizes[chunk-1] + 32;
    *buffer = (uint8_t*)realloc(*buffer, *buffer_alloc);
    }
  
  /* Get offset */
  
  offset = quicktime_chunk_to_offset(file, trak, chunk);

  quicktime_set_position(file, offset);

  result = quicktime_read_data(file, *buffer, trak->chunk_sizes[chunk-1]);

  memset((*buffer) + trak->chunk_sizes[chunk-1], 0, 16);
  
  return result ? trak->chunk_sizes[chunk-1] : 0;
  }

int lqt_append_audio_chunk(quicktime_t * file, int track,
                           int chunk,
                           uint8_t ** buffer, int * buffer_alloc,
                           int initial_bytes)
  {
  int64_t offset;
  quicktime_trak_t * trak;
  int result;

  trak = file->atracks[track].track;

  if(chunk > trak->mdia.minf.stbl.stco.total_entries)
    {
    /* Read beyond EOF */
    file->atracks[track].eof = 1;
    return 0;
    }

  if(!trak->chunk_sizes)
    {
    trak->chunk_sizes = lqt_get_chunk_sizes(file, trak);
    }

  /* Reallocate buffer */

  if(*buffer_alloc < trak->chunk_sizes[chunk-1] + 16 + initial_bytes)
    {
    *buffer_alloc = trak->chunk_sizes[chunk-1] + 32 + initial_bytes;
    *buffer = (uint8_t*)realloc(*buffer, *buffer_alloc);
    }
  
  /* Get offset */
  
  offset = quicktime_chunk_to_offset(file, trak, chunk);

  quicktime_set_position(file, offset);

  result = quicktime_read_data(file, (*buffer) + initial_bytes, trak->chunk_sizes[chunk-1]);

  memset((*buffer) + initial_bytes + trak->chunk_sizes[chunk-1], 0, 16);
  
  return result ? trak->chunk_sizes[chunk-1] : 0;
  }


/* Interlace mode */

static struct
  {
  lqt_interlace_mode_t mode;
  const char * name;
  }
interlace_modes[] =
  {
    { LQT_INTERLACE_NONE,         "None (Progressive)" },
    { LQT_INTERLACE_TOP_FIRST,    "Top field first"    },
    { LQT_INTERLACE_BOTTOM_FIRST, "Bottom field first" }
  };
  
lqt_interlace_mode_t lqt_get_interlace_mode(quicktime_t * file, int track)
  {
  if(track < 0 || track > file->total_vtracks)
    return LQT_INTERLACE_NONE;
  return file->vtracks[track].interlace_mode;
  }

int lqt_set_interlace_mode(quicktime_t * file, int track,
                           lqt_interlace_mode_t mode)
  {
  if(track < 0 || track > file->total_vtracks)
    return 0;
  file->vtracks[track].interlace_mode = mode;
  return 1;
  }

const char * lqt_interlace_mode_to_string(lqt_interlace_mode_t mode)
  {
  int i;
  for(i = 0; i < sizeof(interlace_modes)/sizeof(interlace_modes[0]); i++)
    {
    if(interlace_modes[i].mode == mode)
      return interlace_modes[i].name;
    }
  return interlace_modes[0].name;
  }


/* Sample format */

static struct
  {
  lqt_sample_format_t format;
  const char * name;
  }
sample_formats[] =
  {
    { LQT_SAMPLE_UNDEFINED, "Undefined" }, /* If this is returned, we have an error */
    { LQT_SAMPLE_INT8, "8 bit signed" },
    { LQT_SAMPLE_UINT8, "8 bit unsigned" },
    { LQT_SAMPLE_INT16, "16 bit signed" },
    { LQT_SAMPLE_INT32, "32 bit signed" },
    { LQT_SAMPLE_FLOAT, "Floating point" }, /* Float is ALWAYS machine native */
    { LQT_SAMPLE_DOUBLE, "Double precision" } /* Double is ALWAYS machine native */
  };

const char * lqt_sample_format_to_string(lqt_sample_format_t format)
  {
  int i;
  for(i = 0; i < sizeof(sample_formats)/sizeof(sample_formats[0]); i++)
    {
    if(sample_formats[i].format == format)
      return sample_formats[i].name;
    }
  return sample_formats[0].name;
  }


void lqt_init_vbr_audio(quicktime_t * file, int track)
  {
  quicktime_trak_t * trak = file->atracks[track].track;
  trak->mdia.minf.stbl.stsd.table[0].compression_id = -2;
  trak->mdia.minf.stbl.stsz.sample_size = 0;
  trak->mdia.minf.is_audio_vbr = 1;
  }

void lqt_start_audio_vbr_chunk(quicktime_t * file, int track)
  {
  file->atracks[track].vbr_num_frames = 0;
  }

void lqt_start_audio_vbr_frame(quicktime_t * file, int track)
  {
  quicktime_audio_map_t * atrack = &file->atracks[track];
  atrack->vbr_frame_start = quicktime_position(file);
  }

void lqt_finish_audio_vbr_frame(quicktime_t * file, int track, int num_samples)
  {
  quicktime_stsz_t * stsz;
  quicktime_stts_t * stts;
  quicktime_audio_map_t * atrack = &file->atracks[track];
  
  stsz = &(file->atracks[track].track->mdia.minf.stbl.stsz);
  stts = &(file->atracks[track].track->mdia.minf.stbl.stts);
  
  /* Update stsz */

  quicktime_update_stsz(stsz, file->atracks[track].vbr_frames_written, 
                        quicktime_position(file) - file->atracks[track].vbr_frame_start);
  /* Update stts */
  
  quicktime_update_stts(stts, file->atracks[track].vbr_frames_written, num_samples);

  atrack->vbr_num_frames++;
  atrack->vbr_frames_written++;

  
  }

/* VBR Reading support */

/* Check if VBR reading should be enabled */

int lqt_audio_is_vbr(quicktime_t * file, int track)
  {
  return file->atracks[track].track->mdia.minf.is_audio_vbr;
  }

/* Get the index of the VBR packet (== sample) containing a specified
   uncompressed sample */

static uint64_t packet_of_sample(quicktime_stts_t * stts, int64_t sample)
  {
  int i;
  int64_t packet_count = 0; 
  int64_t sample_count = 0; 
  
  for(i = 0; i < stts->total_entries; i++)
    {
    if(sample_count + stts->table[i].sample_count * stts->table[i].sample_duration > sample)
      {
      return packet_count + (sample - sample_count) / stts->table[i].sample_duration;
      }
    sample_count += stts->table[i].sample_count * stts->table[i].sample_duration;
    packet_count += stts->table[i].sample_count;
    }
  return -1;
  }

/*
 *  Helper function: Get the "durarion of a sample range" (which means the
 *  uncompressed samples in a range of VBR packets)
 */

static int64_t get_uncompressed_samples(quicktime_stts_t * stts, int start_sample,
                                    int end_sample)
  {
  int count, i, stts_index = 0, stts_count = 0;

  int64_t ret;
  
  count = 0;
  ret = 0;
  
  for(i = 0; i < stts->total_entries; i++)
    {
    if(count + stts->table[i].sample_count > start_sample)
      {
      stts_index = i;
      stts_count = start_sample - count;
      break;
      }
    count += stts->table[i].sample_count;
    }

  ret = 0;
  for(i = start_sample; i < end_sample; i++)
    {
    ret += stts->table[stts_index].sample_duration;
    stts_count++;
    if(stts_count >= stts->table[stts_index].sample_count)
      {
      stts_index++;
      stts_count = 0;
      }
    }
  return ret;
  }

/* Analog for quicktime_chunk_samples for VBR files */

int lqt_chunk_of_sample_vbr(int64_t *chunk_sample, 
                            int64_t *chunk, 
                            quicktime_trak_t *trak, 
                            int64_t sample)
  {
  int64_t packet;
  int64_t chunk_packet;
  int sd_id = 1;
  /* Get the index of the packet containing the uncompressed sample */
  packet = packet_of_sample(&trak->mdia.minf.stbl.stts, sample);

  /* Get the chunk of the packet */
  
  quicktime_chunk_of_sample(&chunk_packet,chunk,&sd_id,trak,packet);

  /* Get the first uncompressed sample of the first packet of
     this chunk */
  
  *chunk_sample = get_uncompressed_samples(&trak->mdia.minf.stbl.stts, 0,chunk_packet);
  return 0;
  }


/* Determine the number of VBR packets (=samples) in one chunk */

int lqt_audio_num_vbr_packets(quicktime_t * file, int track, int chunk, int * samples)
  {
  int64_t start_sample;
  
  quicktime_trak_t * trak;
  int result = 0;
  quicktime_stsc_t *stsc;
  int i;

  
  trak = file->atracks[track].track;
    
  stsc = &(trak->mdia.minf.stbl.stsc);

  if(chunk >= trak->mdia.minf.stbl.stco.total_entries)
    return 0;
  
  i = stsc->total_entries - 1;
  
  if(!stsc->total_entries)
    return 0;
  
  start_sample = 0;

  for(i = 0; i < stsc->total_entries; i++)
    {
    if(((i < stsc->total_entries - 1) && (stsc->table[i+1].chunk > chunk)) ||
       (i == stsc->total_entries - 1))
      {
      start_sample += (chunk - stsc->table[i].chunk) * stsc->table[i].samples;
      result = stsc->table[i].samples;
      break;
      }
    else
      start_sample += (stsc->table[i+1].chunk - stsc->table[i].chunk) * stsc->table[i].samples;
    }
  if(samples)
    *samples = get_uncompressed_samples(&(trak->mdia.minf.stbl.stts), start_sample, start_sample + result);
  
  return result;
  }

/* Read one VBR packet */
int lqt_audio_read_vbr_packet(quicktime_t * file, int track, int chunk, int packet,
                              uint8_t ** buffer, int * buffer_alloc, int * samples)
  {
  int64_t offset;
  int i, stsc_index;
  quicktime_trak_t * trak;
  quicktime_stsc_t *stsc;
  int packet_size;
  int first_chunk_packet; /* Index of first packet in the chunk */
  
  trak = file->atracks[track].track;
  stsc = &(trak->mdia.minf.stbl.stsc);

  if(chunk >= trak->mdia.minf.stbl.stco.total_entries)
    return 0;
    
  i = 0;
  stsc_index = 0;
  first_chunk_packet = 0;

  for(i = 0; i < chunk-1; i++)
    {
    if((stsc_index < stsc->total_entries-1) && (stsc->table[stsc_index+1].chunk-1 == i))
      stsc_index++;
    first_chunk_packet += stsc->table[stsc_index].samples;
    }

  /* Get offset */
  offset = trak->mdia.minf.stbl.stco.table[chunk-1].offset;
  for(i = 0; i < packet; i++)
    {
    if(trak->mdia.minf.stbl.stsz.table)
      offset += trak->mdia.minf.stbl.stsz.table[first_chunk_packet+i].size;
    else
      offset += trak->mdia.minf.stbl.stsz.sample_size;
    }
  /* Get packet size */
  if(trak->mdia.minf.stbl.stsz.table)
    packet_size = trak->mdia.minf.stbl.stsz.table[first_chunk_packet+packet].size;
  else
    packet_size = trak->mdia.minf.stbl.stsz.sample_size;
  
  /* Get number of audio samples */
  if(samples)
    *samples = get_uncompressed_samples(&trak->mdia.minf.stbl.stts,
                                        first_chunk_packet+packet, first_chunk_packet+packet+1);
  
  /* Read the data */
  if(*buffer_alloc < packet_size+16)
    {
    *buffer_alloc = packet_size + 128;
    *buffer = (uint8_t*)realloc(*buffer, *buffer_alloc);
    }
  quicktime_set_position(file, offset);
  quicktime_read_data(file, *buffer, packet_size);
  return packet_size;
  }

static struct
  {
  lqt_file_type_t type;
  const char * name;
  }
filetypes[] =
  {
      { LQT_FILE_NONE,     "Unknown/Undefined" },
      { LQT_FILE_QT_OLD,   "Quicktime"         },
      { LQT_FILE_QT,       "Quicktime"         },
      { LQT_FILE_MP4,      "MP4"               },
      { LQT_FILE_M4A,      "M4A"               },
      { LQT_FILE_3GP,      "3GP"               }
  };
  
const char * lqt_file_type_to_string(lqt_file_type_t type)
  {
  int i;
  for(i = 0; i < sizeof(filetypes)/sizeof(filetypes[0]); i++)
    {
    if(filetypes[i].type == type)
      return filetypes[i].name;
    }
  return filetypes[0].name;
  }

lqt_file_type_t lqt_get_file_type(quicktime_t * file)
  {
  return file->file_type;
  }

