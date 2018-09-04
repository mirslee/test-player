/*******************************************************************************
 trak.c

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
#include <stdlib.h>

int quicktime_trak_init(quicktime_trak_t *trak, lqt_file_type_t type)
{
	quicktime_tkhd_init(&(trak->tkhd), type);
	quicktime_edts_init(&(trak->edts));
	quicktime_mdia_init(&(trak->mdia));
	quicktime_tref_init(&(trak->tref));
	trak->has_tref = 0;
	return 0;
}

int quicktime_trak_init_video(quicktime_t *file, 
							quicktime_trak_t *trak, 
							int frame_w, 
							int frame_h, 
                                                        int frame_duration,
                                                        int timescale,
							char *compressor)
{

	quicktime_tkhd_init_video(file, 
		&(trak->tkhd), 
		frame_w, 
		frame_h);
	quicktime_mdia_init_video(file, 
		&(trak->mdia), 
		frame_w, 
		frame_h, 
                frame_duration,
                timescale,
		compressor);
        if(!IS_MP4(file->file_type))
          {
          quicktime_edts_init_table(&(trak->edts));
          trak->has_edts = 1;
          }
        return 0;
}

int quicktime_trak_init_timecode(quicktime_t *file, 
                                 quicktime_trak_t *trak,
                                 int time_scale,
                                 int frame_duration,
                                 int num_frames,
                                 int frame_w,
                                 int frame_h,
                                 uint32_t flags)
  {
  quicktime_tkhd_init_timecode(file, &(trak->tkhd), frame_w, frame_h);
  quicktime_mdia_init_timecode(file, &(trak->mdia), time_scale, frame_duration,
                               num_frames, flags);
  quicktime_edts_init_table(&(trak->edts));
  return 0;
  }



int quicktime_trak_init_qtvr(quicktime_t *file, quicktime_trak_t *trak, int track_type, int width, int height, int frame_duration, int timescale)
{

	quicktime_tkhd_init_video(file, 
		&(trak->tkhd),
		height, 
		width);
	quicktime_mdia_init_qtvr(file, 
		&(trak->mdia), track_type, timescale, frame_duration);
	quicktime_edts_init_table(&(trak->edts));
        trak->has_edts = 1;
	quicktime_tref_init_qtvr(&(trak->tref), track_type);
        trak->has_tref = 1;
	return 0;
}

int quicktime_trak_init_panorama(quicktime_t *file, quicktime_trak_t *trak, int width, int height, int frame_duration, int timescale)
{

	quicktime_tkhd_init_video(file, 
		&(trak->tkhd), 
		height, 
		height/2);
	quicktime_mdia_init_panorama(file, 
		&(trak->mdia), width, height, timescale, frame_duration);
	quicktime_edts_init_table(&(trak->edts));
        trak->has_edts = 1;

	return 0;
}

int quicktime_trak_init_audio(quicktime_t *file, 
							quicktime_trak_t *trak, 
							int channels, 
							int sample_rate, 
							int bits, 
							char *compressor)
  {
  quicktime_mdia_init_audio(file, &(trak->mdia), 
                            channels, 
                            sample_rate, 
                            bits, 
                            compressor);
  if(!IS_MP4(file->file_type))
    {
    quicktime_edts_init_table(&(trak->edts));
    trak->has_edts = 1;
    }
  return 0;
  }

int quicktime_trak_init_text(quicktime_t * file, quicktime_trak_t * trak,
                             int timescale)
  {
  trak->tkhd.volume = 0;
  trak->tkhd.flags = 3;
  quicktime_mdia_init_text(file, &(trak->mdia), 
                           timescale);

  if(!IS_MP4(file->file_type))
    {
    quicktime_edts_init_table(&(trak->edts));
    trak->has_edts = 1;
    }
  return 0;
  }

int quicktime_trak_init_tx3g(quicktime_t * file, quicktime_trak_t * trak,
                             int timescale)
  {
  trak->tkhd.volume = 0;
  trak->tkhd.flags = 1;
  quicktime_mdia_init_tx3g(file, &(trak->mdia), 
                           timescale);
 
  return 0;
  }
                            
                            

int quicktime_trak_delete(quicktime_trak_t *trak)
{
	quicktime_mdia_delete(&(trak->mdia));
	quicktime_edts_delete(&(trak->edts));
	quicktime_tkhd_delete(&(trak->tkhd));
	quicktime_tref_delete(&(trak->tref));

        if(trak->chunk_sizes)
          free(trak->chunk_sizes);
        return 0;
}


int quicktime_trak_dump(quicktime_trak_t *trak)
{
	lqt_dump(" track (trak)\n");
	quicktime_tkhd_dump(&(trak->tkhd));
	if(trak->has_edts) quicktime_edts_dump(&(trak->edts));
	if (trak->has_tref)
	    quicktime_tref_dump(&(trak->tref));
	quicktime_mdia_dump(&(trak->mdia));

	return 0;
}

// Used when reading a file
quicktime_trak_t* quicktime_add_trak(quicktime_t *file)
{
	quicktime_moov_t *moov = &(file->moov);
	if(moov->total_tracks < MAXTRACKS)
	{
        moov->trak[moov->total_tracks] = (quicktime_trak_t*)calloc(1, sizeof(quicktime_trak_t));
		quicktime_trak_init(moov->trak[moov->total_tracks], file->file_type);
		moov->total_tracks++;
	}
	return moov->trak[moov->total_tracks - 1];
}

int quicktime_delete_trak(quicktime_moov_t *moov)
{
	if(moov->total_tracks)
	{
		moov->total_tracks--;
		quicktime_trak_delete(moov->trak[moov->total_tracks]);
		free(moov->trak[moov->total_tracks]);
	}
	return 0;
}


int quicktime_read_trak(quicktime_t *file, quicktime_trak_t *trak,
                        quicktime_atom_t *trak_atom)
  {
  quicktime_atom_t leaf_atom;

  do
    {
    quicktime_atom_read_header(file, &leaf_atom);

    /* mandatory */
    if(quicktime_atom_is(&leaf_atom, "tkhd"))
      quicktime_read_tkhd(file, &(trak->tkhd));
    else if(quicktime_atom_is(&leaf_atom, "mdia"))
      quicktime_read_mdia(file, trak, &(trak->mdia), &leaf_atom);
    /* optional */
    else if(quicktime_atom_is(&leaf_atom, "clip"))
      quicktime_atom_skip(file, &leaf_atom);
    else if(quicktime_atom_is(&leaf_atom, "matt"))
      quicktime_atom_skip(file, &leaf_atom);
    else if(quicktime_atom_is(&leaf_atom, "edts"))
      {
      quicktime_read_edts(file, &(trak->edts), &leaf_atom);
      trak->has_edts = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "load"))
      quicktime_atom_skip(file, &leaf_atom);
    else if(quicktime_atom_is(&leaf_atom, "imap"))
      quicktime_atom_skip(file, &leaf_atom);
    else if(quicktime_atom_is(&leaf_atom, "udta"))
      quicktime_atom_skip(file, &leaf_atom);
    else if(quicktime_atom_is(&leaf_atom, "tref"))
      {
      trak->has_tref = 1;
      quicktime_read_tref(file, &(trak->tref), &leaf_atom);
      }
    else quicktime_atom_skip(file, &leaf_atom);
    } while(quicktime_position(file) < trak_atom->end);
  
  return 0;
  }

int quicktime_write_trak(quicktime_t *file, 
                         quicktime_trak_t *trak)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "trak");

  quicktime_write_tkhd(file, &(trak->tkhd));
        
  if(trak->has_edts)
    quicktime_write_edts(file, &(trak->edts));
  quicktime_write_mdia(file, &(trak->mdia));
	
  if (trak->has_tref) 
    quicktime_write_tref(file, &(trak->tref));
	
  quicktime_atom_write_footer(file, &atom);

  return 0;
  }

int64_t quicktime_track_samples(quicktime_t *file, quicktime_trak_t *trak)
  {
  quicktime_stts_t *stts = &(trak->mdia.minf.stbl.stts);
  int i;
  int64_t total = 0;
  
  if(trak->mdia.minf.is_audio)
    {
    for(i = 0; i < stts->total_entries; i++)
      {
      total += (int64_t)((uint32_t)stts->table[i].sample_count) *
        stts->table[i].sample_duration;
      }
    }
  else
    {
    for(i = 0; i < stts->total_entries; i++)
      {
      total += (uint32_t)stts->table[i].sample_count;
      }
    }
  return total;
  }

int quicktime_sample_of_chunk(quicktime_trak_t *trak, int chunk)
{
	quicktime_stsc_table_t *table = trak->mdia.minf.stbl.stsc.table;
	int total_entries = trak->mdia.minf.stbl.stsc.total_entries;
	int chunk1entry, chunk2entry;
	int chunk1, chunk2, chunks, total = 0;

	for(chunk1entry = total_entries - 1, chunk2entry = total_entries; 
		chunk1entry >= 0; 
		chunk1entry--, chunk2entry--)
	{
		chunk1 = table[chunk1entry].chunk;

		if(chunk > chunk1)
		{
			if(chunk2entry < total_entries)
			{
				chunk2 = table[chunk2entry].chunk;

				if(chunk < chunk2) chunk2 = chunk;
			}
			else
				chunk2 = chunk;

			chunks = chunk2 - chunk1;

			total += chunks * table[chunk1entry].samples;
		}
	}

	return total;
}


int quicktime_chunk_of_sample(int64_t *chunk_sample,int64_t *chunk,int *sd_id,quicktime_trak_t *trak,int64_t sample)
{
	quicktime_stsc_table_t *table = trak->mdia.minf.stbl.stsc.table;
	int total_entries = trak->mdia.minf.stbl.stsc.total_entries;
	int chunk2entry,current_chunk,sample_duration;;
	int chunk1, chunk2, chunk1samples;
	int64_t range_samples, total = 0;
	quicktime_stts_t *stts = &(trak->mdia.minf.stbl.stts);

	*sd_id = 1;
	chunk1 = 1;
	chunk1samples = 0;
	chunk2entry = 0;

	if(!total_entries)
	{
		*chunk_sample = 0;
		*chunk = 0;
		return 0;
	}

	*sd_id = table[0].id;
	do
	{
		chunk2 = table[chunk2entry].chunk;
		*chunk = chunk2 - chunk1;
		range_samples = *chunk * chunk1samples;
		if(sample < total + range_samples) break;

		*sd_id = table[chunk2entry].id;

		if(trak->mdia.minf.is_audio)
		{
			int i = stts->total_entries - 1;

			do {
				current_chunk = stts->table[i].sample_count;
				i--;
			} while(i >= 0 && current_chunk > chunk2entry);	

			sample_duration = stts->table[i+1].sample_duration;
		}
		else
			sample_duration = 1; // this way nothing is broken ... I hope

		chunk1samples = table[chunk2entry].samples * sample_duration;
		chunk1 = chunk2;

		if(chunk2entry < total_entries)
		{
			chunk2entry++;
			total += range_samples;
		}
	}while(chunk2entry < total_entries);

	if(chunk1samples)
		*chunk = (sample - total) / chunk1samples + chunk1;
	else
		*chunk = 1;

	*chunk_sample = total + (*chunk - chunk1) * chunk1samples;
	return 0;
}

int64_t quicktime_chunk_to_offset(quicktime_t *file,quicktime_trak_t *trak, int64_t chunk)
{
	quicktime_stco_table_t *table = trak->mdia.minf.stbl.stco.table;
	int64_t result = 0;

	if(trak->mdia.minf.stbl.stco.total_entries && 
		chunk > trak->mdia.minf.stbl.stco.total_entries)
		result = table[trak->mdia.minf.stbl.stco.total_entries - 1].offset;
	else
	if(trak->mdia.minf.stbl.stco.total_entries)
		result = table[chunk - 1].offset;
	else
		result = HEADER_LENGTH * 2;

	return result;
}

int quicktime_offset_to_chunk(int64_t *chunk_offset, 
	quicktime_trak_t *trak, 
	int64_t offset)
{
	quicktime_stco_table_t *table = trak->mdia.minf.stbl.stco.table;
	int i;

	for(i = trak->mdia.minf.stbl.stco.total_entries - 1; i >= 0; i--)
	{
		if(table[i].offset <= offset)
		{
			*chunk_offset = table[i].offset;
			return i + 1;
		}
	}
	*chunk_offset = HEADER_LENGTH * 2;
	return 1;
}


int64_t quicktime_sample_range_size(quicktime_trak_t *trak,int64_t chunk_sample,int64_t sample)
{

int64_t i, total;
        /* LQT: For audio, quicktime_sample_rage_size makes no sense */
        if(trak->mdia.minf.is_audio)
          return 0;
	else
          {
          /* All frames have the same size */
          if(trak->mdia.minf.stbl.stsz.sample_size)
            {
            total = (sample - chunk_sample) *
              trak->mdia.minf.stbl.stsz.sample_size;
            }
/* probably video */
          else
            {
            for(i = chunk_sample, total = 0; i < sample; i++)
              {
              total += trak->mdia.minf.stbl.stsz.table[i].size;
              }
            }
          
	}
	return total;
}

int64_t quicktime_sample_to_offset(quicktime_t *file,
                                   quicktime_trak_t *trak,int *sd_id,int sample)
{
	int64_t chunk, chunk_sample, chunk_offset1, chunk_offset2;
	quicktime_chunk_of_sample(&chunk_sample, &chunk,sd_id, trak, sample);
	chunk_offset1 = quicktime_chunk_to_offset(file, trak, chunk);
	chunk_offset2 = chunk_offset1 +quicktime_sample_range_size(trak, chunk_sample, sample);
	return chunk_offset2;
}

void quicktime_write_chunk_header(quicktime_t *file, 
	quicktime_trak_t *trak, 
	quicktime_atom_t *chunk)
{
    chunk->start = quicktime_position(file);
}

void quicktime_write_chunk_footer(quicktime_t *file, 
                                  quicktime_trak_t *trak,
                                  int current_chunk,
                                  quicktime_atom_t *chunk, 
                                  int samples)
  {
  int64_t offset = chunk->start;
  int sample_size = (int)(quicktime_position(file) - offset);

  if(offset + sample_size > file->mdat.atom.size)
    file->mdat.atom.size = offset + sample_size;

  quicktime_update_stco(&(trak->mdia.minf.stbl.stco), 
                        current_chunk, 
                        offset);

  if(trak->mdia.minf.is_video || trak->mdia.minf.is_text)
    quicktime_update_stsz(&(trak->mdia.minf.stbl.stsz), 
                          current_chunk - 1, 
                          sample_size);
  /* Need to increase sample count for CBR (the VBR routines to it
     themselves) */
  if(trak->mdia.minf.is_audio && !trak->mdia.minf.is_audio_vbr)
  {
	  quicktime_update_audio_stts(&trak->mdia.minf.stbl.stts,samples);
  }     
  if(trak->mdia.minf.is_panorama)
    {
    quicktime_update_stsz(&(trak->mdia.minf.stbl.stsz), 
                          current_chunk - 1, 
                          sample_size);	
    }
  
  if(trak->mdia.minf.is_qtvr)
    {
    quicktime_update_stsz(&(trak->mdia.minf.stbl.stsz), 
                          current_chunk - 1, 
                          sample_size);
    }
  
  quicktime_update_stsc(&(trak->mdia.minf.stbl.stsc), 
                        current_chunk, 
                        samples);
  }

void quicktime_write_reference_chunk(quicktime_t *file,quicktime_trak_t *trak,int current_chunk,int64_t offset,int sample_size,int samples)
{
	quicktime_update_stco(&(trak->mdia.minf.stbl.stco),current_chunk,offset);

	if(trak->mdia.minf.is_video || trak->mdia.minf.is_text)
		quicktime_update_stsz(&(trak->mdia.minf.stbl.stsz),current_chunk - 1, sample_size);
	/* Need to increase sample count for CBR (the VBR routines to it
	themselves) */
	if(trak->mdia.minf.is_audio && !trak->mdia.minf.is_audio_vbr)
		quicktime_update_audio_stts(&trak->mdia.minf.stbl.stts,samples);
	if(trak->mdia.minf.is_panorama)
		quicktime_update_stsz(&(trak->mdia.minf.stbl.stsz),current_chunk - 1,sample_size);
	if(trak->mdia.minf.is_qtvr)
		quicktime_update_stsz(&(trak->mdia.minf.stbl.stsz),current_chunk - 1,sample_size);
 
	quicktime_update_stsc(&(trak->mdia.minf.stbl.stsc),current_chunk,samples);
}

int quicktime_trak_duration(quicktime_trak_t *trak, 
                            int64_t *duration, 
                            int *timescale)
  {
  quicktime_stts_t *stts = &(trak->mdia.minf.stbl.stts);
  int i;
  *duration = 0;

  for(i = 0; i < stts->total_entries; i++)
    {
    *duration += stts->table[i].sample_duration * stts->table[i].sample_count;
    }
  if(timescale)
    *timescale = trak->mdia.mdhd.time_scale;
        
  return 0;
  }

void quicktime_fix_stbl(quicktime_stsc_t* stsc, quicktime_stsz_t* stsz, quicktime_stco_t* stco)
{
	quicktime_stsc_table_t* stsct_n = stsc->table;
	quicktime_stsc_table_t* stsct_o = stsc->table;
	quicktime_stsz_table_t* stszt = stsz->table;
	quicktime_stco_table_t* stcot_n = stco->table;
	quicktime_stco_table_t* stcot_o = stco->table;

	int chunk = stsct_n->chunk+1;
	if(stsz->sample_size == 0)
	{
		for (int i = 0; i < stsc->total_entries - 1; i++)
		{
			if ((stcot_o[i].offset + stszt[i].size) == stcot_o[i + 1].offset)
			{
				stsct_n->samples++;
			}
			else
			{
				stsct_n++;
				stcot_n++;
				stsct_n->chunk = chunk++;
				stsct_n->samples = stsct_o[i+1].id;
				stsct_n->id = stsct_o[i+1].id;
				stcot_n->offset = stcot_o[i+1].offset;
			}
		}
	}
	else
	{
		for (int i = 0; i < stsc->total_entries - 1; i++)
		{
			if ((stcot_o[i].offset + stsz->sample_size) == stcot_o[i + 1].offset)
			{
				stsct_n->samples++;
			}
			else
			{
				stsct_n++;
				stcot_n++;
				stsct_n->chunk = chunk++;
				stsct_n->samples = stsct_o[i+1].id;
				stsct_n->id = stsct_o[i+1].id;
				stcot_n->offset = stcot_o[i+1].offset;
			}
		}
	}
	stsc->total_entries = (int)(stsct_n - stsc->table)+1;
	stco->total_entries = (int)(stcot_n - stco->table)+1;
}

int quicktime_trak_fix_counts(quicktime_t *file, quicktime_trak_t *trak, int moov_time_scale, int64_t suggestduration)
  {
  int64_t duration;
  int timescale;
  quicktime_stts_t* stts;
  quicktime_stsc_t* stsc;
  quicktime_stsz_t* stsz;
  quicktime_stco_t* stco;

  int64_t samples = quicktime_track_samples(file, trak);
  
  quicktime_trak_duration(trak, &duration, &timescale);
  
  /* get duration in movie's units */
  trak->tkhd.duration = (duration*moov_time_scale*2+ timescale)/(2*timescale);
  if ((suggestduration > 0) && (trak->tkhd.duration > (uint64_t)suggestduration))
	  trak->tkhd.duration = suggestduration;

  trak->mdia.mdhd.duration = duration;
  trak->mdia.mdhd.time_scale = timescale;
  
  if(trak->has_edts)
    trak->edts.elst.table[0].duration = trak->tkhd.duration;

  if(trak->mdia.minf.is_panorama)
    trak->edts.elst.total_entries = 1;
  
  stts = &trak->mdia.minf.stbl.stts;

  stsc = &trak->mdia.minf.stbl.stsc;
  stsz = &trak->mdia.minf.stbl.stsz;
  stco = &trak->mdia.minf.stbl.stco;
  if (trak->mdia.minf.is_video)
	  quicktime_fix_stbl(stsc, stsz, stco);

  quicktime_compress_stsc(stsc);
  
  if(trak->mdia.minf.is_video || trak->mdia.minf.is_text ||
     trak->mdia.minf.is_timecode)
    {
    quicktime_compress_stts(stts);
    if(stts->total_entries == 1)
      stts->table[0].sample_count = (int)samples;
    }
  else if(trak->mdia.minf.is_audio_vbr)
    {
    quicktime_compress_stts(stts);
    }
  else if(trak->mdia.minf.stbl.stts.total_entries == 1)
    trak->mdia.minf.stbl.stts.table[0].sample_count = (int)samples;

  if(trak->mdia.minf.is_video &&
     IS_MP4(file->file_type) &&
     trak->mdia.minf.stbl.has_ctts)
    {
		quicktime_fix_ctts(&trak->mdia.minf.stbl.ctts);
    }
  
  if(!trak->mdia.minf.stbl.stsz.total_entries)
    {
    // trak->mdia.minf.stbl.stsz.sample_size = 1;
    trak->mdia.minf.stbl.stsz.total_entries = (int)samples;
    }
  
  return 0;
  }

int quicktime_chunk_samples(quicktime_trak_t *trak, int chunk)
{
	int result, current_chunk;
	quicktime_stsc_t *stsc = &(trak->mdia.minf.stbl.stsc);
	quicktime_stts_t *stts = &(trak->mdia.minf.stbl.stts);
	quicktime_stsd_t *stsd = &(trak->mdia.minf.stbl.stsd);
	int i = stsc->total_entries - 1;

        if(!stsc->total_entries)
          return 0;
	do
	{
		current_chunk = stsc->table[i].chunk;
		result = stsc->table[i].samples;
		i--;
	}while(i >= 0 && current_chunk > chunk);
        /* LQT: Multiply with duration */
        if(stsd->table[0].compression_id == -2)
		{
			if ((trak->mdia.minf.is_audio == 1) && (stsd->table[0].audio_samples_per_packet > 0))
			{
				 result *= stsd->table[0].audio_samples_per_packet;
			}
			else
			{
				 result *= stts->table[0].sample_duration;
			}
		}
	return result;
}

int quicktime_trak_shift_offsets(quicktime_trak_t *trak, int64_t offset)
{
	quicktime_stco_t *stco = &(trak->mdia.minf.stbl.stco);
	int i;

	for(i = 0; i < stco->total_entries; i++)
	{
		stco->table[i].offset += offset;
	}
	return 0;
}
