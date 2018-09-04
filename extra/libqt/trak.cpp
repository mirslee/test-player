/* trak.c
 * Copyright (C) 2001 QT4Linux and OpenQuicktime Teams 
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
 * $Id: trak.c,v 1.26 2003/04/20 00:32:58 nhumfrey Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


int oqt_trak_init(oqt_trak_t *trak)
{
	oqt_tkhd_init(&(trak->tkhd));
	oqt_edts_init(&(trak->edts));
	oqt_mdia_init(&(trak->mdia));
	return 0;
}

int oqt_trak_init_video(oqt_t *file, 
							oqt_trak_t *trak, 
							int frame_w, 
							int frame_h, 
							float frame_rate,
							char *compressor)
{
	oqt_tkhd_init_video(file, 
		&(trak->tkhd), 
		frame_w, 
		frame_h);
	oqt_mdia_init_video(file, 
		&(trak->mdia), 
		frame_w, 
		frame_h, 
		frame_rate, 
		compressor);
	oqt_edts_init_table(&(trak->edts));

	return 0;
}

int oqt_trak_init_audio(oqt_t *file, 
							oqt_trak_t *trak, 
							int channels, 
							int sample_rate, 
							int bits, 
							char *compressor)
{
	oqt_mdia_init_audio(file, &(trak->mdia), channels, sample_rate, bits, compressor);
	oqt_edts_init_table(&(trak->edts));

	return 0;
}

int oqt_trak_delete(oqt_trak_t *trak)
{
	oqt_tkhd_delete(&(trak->tkhd));
	oqt_edts_delete(&(trak->edts));
	oqt_mdia_delete(&(trak->mdia));
	return 0;
}


int oqt_trak_dump(oqt_trak_t *trak)
{
	printf(" track\n");
	oqt_tkhd_dump(&(trak->tkhd));
	oqt_edts_dump(&(trak->edts));
	oqt_mdia_dump(&(trak->mdia));

	return 0;
}


oqt_trak_t* oqt_add_trak(oqt_moov_t *moov)
{
	oqt_trak_t *trak = NULL;

	if(moov->total_tracks < OQT_MAX_TRACKS)
	{
		trak = moov->trak[moov->total_tracks] = (oqt_trak_t*)calloc(1, sizeof(oqt_trak_t));
		oqt_trak_init(trak);
		trak->tkhd.track_id = moov->mvhd.next_track_id++;
		moov->total_tracks++;
	}
	return trak;
}


int oqt_delete_trak(oqt_moov_t *moov)
{
	if(moov->total_tracks)
	{
		moov->total_tracks--;
		oqt_trak_delete(moov->trak[moov->total_tracks]);
		free(moov->trak[moov->total_tracks]);
	}
	return 0;
}


int oqt_read_trak(oqt_t *file, oqt_trak_t *trak, oqt_atom_t *trak_atom)
{
	oqt_atom_t leaf_atom;

	do
	{			  
		oqt_atom_read_header(file, &leaf_atom);
//printf("oqt_read_trak 0x%llx 0x%llx\n", oqt_get_position(file), file->oqt_ftell(file));
/* mandatory */

		if(oqt_atom_is(&leaf_atom, "tkhd"))
			{ oqt_read_tkhd(file, &(trak->tkhd)); }
		else
		if(oqt_atom_is(&leaf_atom, "mdia"))
			{ oqt_read_mdia(file, &(trak->mdia), &leaf_atom); }
		else
/* optional */
		if(oqt_atom_is(&leaf_atom, "clip"))
			{ oqt_atom_skip(file, &leaf_atom); }
		else
		if(oqt_atom_is(&leaf_atom, "matt"))
			{ oqt_atom_skip(file, &leaf_atom); }
		else
		if(oqt_atom_is(&leaf_atom, "edts"))
			{ oqt_read_edts(file, &(trak->edts), &leaf_atom); }
		else
		if(oqt_atom_is(&leaf_atom, "load"))
			{ oqt_atom_skip(file, &leaf_atom); }
		else
		if(oqt_atom_is(&leaf_atom, "tref"))
			{ oqt_atom_skip(file, &leaf_atom); }
		else
		if(oqt_atom_is(&leaf_atom, "imap"))
			{ oqt_atom_skip(file, &leaf_atom); }
		else
		if(oqt_atom_is(&leaf_atom, "udta"))
			{ oqt_atom_skip(file, &leaf_atom); }
		else
			oqt_atom_skip(file, &leaf_atom);
//printf("oqt_read_trak 0x%llx 0x%llx\n", oqt_get_position(file), leaf_atom.end);
	}while(oqt_get_position(file) < trak_atom->end);

	return 0;
}

int oqt_write_trak(oqt_t *file, 
	oqt_trak_t *trak, 
	long moov_time_scale)
{
	long duration;
	long timescale;
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, "trak");
	oqt_trak_duration(trak, &duration, &timescale);

	fprintf(stderr, "oqt_write_trak duration %d\n", duration);
	
/* get duration in movie's units */
	trak->tkhd.duration = (long)((float)duration / timescale * moov_time_scale);
	trak->mdia.mdhd.duration = duration;
	trak->mdia.mdhd.time_scale = timescale;

	oqt_write_tkhd(file, &(trak->tkhd));
	oqt_write_edts(file, &(trak->edts), trak->tkhd.duration);
	oqt_write_mdia(file, &(trak->mdia));

	oqt_atom_write_footer(file, &atom);

	return 0;
}

__int64 oqt_track_samples(oqt_t *file, oqt_trak_t *trak)
{
	if(file->stream_mode&OQT_MODE_WRITE)
	{
// get the sample count when creating a new file
 		oqt_stsc_table_t *table = trak->mdia.minf.stbl.stsc.table;
		long total_entries = trak->mdia.minf.stbl.stsc.total_entries;
		long chunk = trak->mdia.minf.stbl.stco.total_entries;
		__int64 sample;

		if(chunk)
		{
			sample = oqt_sample_of_chunk(trak, chunk);
			sample += table[total_entries - 1].samples;
		}
		else 
			sample = 0;
		
		return sample;
	}
	else
	{
/* get the sample count when reading only */
		oqt_stts_t *stts = &(trak->mdia.minf.stbl.stts);
		int i;
		long total = 0;

		for(i = 0; i < stts->total_entries; i++)
		{
			total += stts->table[i].sample_count;
		}
		return total;
	}
}

__int64 oqt_sample_of_chunk(oqt_trak_t *trak, long chunk)
{
	oqt_stsc_table_t *table = trak->mdia.minf.stbl.stsc.table;
	long total_entries = trak->mdia.minf.stbl.stsc.total_entries;
	long chunk1entry, chunk2entry;
	long chunk1, chunk2, chunks, total = 0;

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

int oqt_chunk_of_sample(__int64 *chunk_sample,long *chunk,oqt_trak_t *trak,long *sd_id,__int64 sample)
{
	oqt_stsc_table_t *table = trak->mdia.minf.stbl.stsc.table;
	long total_entries = trak->mdia.minf.stbl.stsc.total_entries;
	long chunk2entry, i, current_chunk, sample_duration;
	long chunk1, chunk2, chunk1samples, range_samples, total = 0;
	oqt_stts_t *stts = &(trak->mdia.minf.stbl.stts);

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
		*sd_id = table[chunk2entry].id;
		chunk2 = table[chunk2entry].chunk;
		*chunk = chunk2 - chunk1;
		range_samples = *chunk * chunk1samples;

		if(sample < total + range_samples) break;

		*sd_id = table[chunk2entry].id;

		/* Yann: I've modified this to handle samples with duration
		   different from 1 ... needed by ".mp3" fourcc */
		
		if(trak->mdia.minf.is_audio)
		  {
		    i = stts->total_entries - 1;
		    
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
		*chunk = (int)((sample-total)/chunk1samples+chunk1);
	else
		*chunk = 1;

	*chunk_sample = total + (*chunk - chunk1) * chunk1samples;
	return 0;
}

__int64 oqt_chunk_to_offset(oqt_trak_t *trak, long chunk)
{
	oqt_stco_table_t *table = trak->mdia.minf.stbl.stco.table;

	if(trak->mdia.minf.stbl.stco.total_entries && chunk > trak->mdia.minf.stbl.stco.total_entries) {
		fprintf(stderr, "oqt_chunk_to_offset: error chunk number is out of range (%ld)\n", (long)chunk);
		return -1;
	} else if(trak->mdia.minf.stbl.stco.total_entries)
		return table[chunk - 1].offset;

	/**** Stupid default value ****/
	fprintf(stderr, "oqt_chunk_to_offset: warning no stco table.\n");
	return OQT_ATOM_HEAD_LENGTH * 2;
}


__int64 oqt_sample_range_size(oqt_trak_t *trak,__int64 chunk_sample,__int64 sample)
{
	oqt_stts_t *stts = &(trak->mdia.minf.stbl.stts);
	__int64 i, total;
  
	if(trak->mdia.minf.stbl.stsz.sample_size)
	{
		if(trak->mdia.minf.is_video)
			return (sample - chunk_sample) * trak->mdia.minf.stbl.stsz.sample_size;
		else
			return (sample-chunk_sample)*trak->mdia.minf.stbl.stsz.sample_size*trak->mdia.minf.stbl.stsd.table[0].channels*trak->mdia.minf.stbl.stsd.table[0].sample_size/8; 
	}
	else if(trak->mdia.minf.is_video)
	{
		for(i = chunk_sample, total = 0; i < sample; i++)
			total += trak->mdia.minf.stbl.stsz.table[i].size;
	}
	else // Yann: again, for my .mp3 VBR ...
	{
		long duration_index = 0;
		long duration = stts->table[duration_index].sample_duration;
		long sample_passed = 0;
		//printf("\t\t  VBR audio duration %d\n", duration);

		for(i = chunk_sample, total = 0; i < sample; i+=duration)
		{
			long chunk_index = (long)(i/duration);
			//printf("\t\t i/duration %li\n", i/duration);
			if(chunk_index >= trak->mdia.minf.stbl.stsz.total_entries) break;

			total += trak->mdia.minf.stbl.stsz.table[chunk_index].size;
			if(chunk_index > sample_passed + stts->table[duration_index].sample_count) 
			{
				sample_passed += stts->table[duration_index].sample_count;
				duration_index++;
				duration = stts->table[duration_index].sample_duration;

				/*	      total += trak->mdia.minf.stbl.stsz.table[chunk_index].size;

				if(chunk_index > sample_passed + stts->table[duration_index].sample_count) {
				sample_passed += stts->table[duration_index].sample_count;
				duration_index++;
				duration = stts->table[duration_index].sample_duration;
				*/		
			}
		}
		//printf("\t\t  VBR audio total %d\n", total);
	}
	return total;
}

__int64 oqt_sample_to_offset(oqt_trak_t *trak, long *sd_id, __int64 sample)
{
	__int64 chunk_sample, chunk_offset1, chunk_offset2;
	long chunk;

	oqt_chunk_of_sample(&chunk_sample, &chunk, trak, sd_id, sample);
	//		printf("\tBEFORE  oqt_chunk_to_offset chunk %lld, chunk_sample %lld\n", chunk, chunk_sample);
	chunk_offset1 = oqt_chunk_to_offset(trak, chunk);
	//		printf("\tAFTER  oqt_chunk_to_offset %lld\n", chunk_offset1);
	chunk_offset2 = chunk_offset1 + oqt_sample_range_size(trak, chunk_sample, sample);
	//		printf("\tAFTER  AFTER %lld\n", chunk_offset2);
//printf("oqt_sample_to_offset chunk %lld sample %lld chunk_offset %lld chunk_sample %lld chunk_offset + samples %lld\n",
//	 chunk, sample, chunk_offset1, chunk_sample, chunk_offset2);
	return chunk_offset2;
}

/*!
	@param url				The data URL, NULL means this file (default)
	@param offset			The offset of the chunk in the file (bytes)
	@param chunk			The chunk number in the track
	@param sample			The number of first sample in the chunk
	@param num_samples		The number of samples in the chunk
	@param sample_size		The length in bytes of the chunk
	@param num_frames		Number of frames in the chunk
	@param frame_size_array	Array contain the size of each frame in the chunk
*/
int oqt_update_tables(	oqt_t *file, 
			oqt_trak_t *trak, 
			char *url,
			__int64 offset, 
			__int64 chunk, 
			__int64 sample, 
			__int64 num_samples, 
			long sample_size,
			long	num_frames,
			long	*frame_size_array)
{
	int sd_id = 1;

	if(url == NULL && offset + sample_size > file->mdat.atom.size) {
		file->mdat.atom.size = offset + sample_size;
	}

	oqt_update_stco(&(trak->mdia.minf.stbl.stco), (long)chunk, offset);
	if(sample_size && num_frames <=1) 
		oqt_update_stsz(&(trak->mdia.minf.stbl.stsz), sample, sample_size);
	else {
		int i;
		oqt_stsz_t *stsz = &(trak->mdia.minf.stbl.stsz);
		int cur_total_entries = stsz->total_entries;
		for(i = cur_total_entries; i < cur_total_entries+num_frames; i++) {
			oqt_update_stsz(stsz, i, frame_size_array[i-cur_total_entries]);
		}
	}

	if(url != NULL) {
		// external reference, make sure stsd and dref are updated
		oqt_stsd_t *stsd = &trak->mdia.minf.stbl.stsd;
		oqt_dref_t *dref = &trak->mdia.minf.dinf.dref;
		int dref_index;

		dref_index = oqt_dref_find_url_entry(dref, url);
		if(dref_index <= 0) {
			dref_index = oqt_dref_add_url_entry(dref, url);
		}

		sd_id = oqt_stsd_find_reference_entry(stsd, dref_index);
		if(sd_id <= 0) {
			sd_id = oqt_stsd_add_reference_entry(stsd, dref_index);
		}
	}

	oqt_update_stsc(&(trak->mdia.minf.stbl.stsc), (long)chunk, num_samples, sd_id);
	return 0;
}

int oqt_trak_duration(oqt_trak_t *trak, 
	long *duration, 
	long *timescale)
{
	oqt_stts_t *stts = &(trak->mdia.minf.stbl.stts);
	int i;
	*duration = 0;

	for(i = 0; i < stts->total_entries; i++)
	{
		*duration += stts->table[i].sample_duration * stts->table[i].sample_count;
	}

	*timescale = trak->mdia.mdhd.time_scale;
	return 0;
}

int oqt_trak_fix_counts(oqt_t *file, oqt_trak_t *trak)
{
	long samples =(long) oqt_track_samples(file, trak);

	trak->mdia.minf.stbl.stts.table[0].sample_count = samples;

	if(trak->mdia.minf.stbl.stsz.sample_size)
		trak->mdia.minf.stbl.stsz.total_entries = samples;

	return 0;
}

__int64 oqt_chunk_samples(oqt_trak_t *trak, long chunk)
{
	long result, current_chunk;
	oqt_stsc_t *stsc = &(trak->mdia.minf.stbl.stsc);
	long i = stsc->total_entries - 1;
	oqt_stts_t *stts = &(trak->mdia.minf.stbl.stts);
	__int64 interm;

	do
	{
		current_chunk = stsc->table[i].chunk;
		result = stsc->table[i].samples;
		i--;
	}while(i >= 0 && current_chunk > chunk);	

	i = stts->total_entries - 1;

	/* Yann: I've modified this to handle samples with a duration 
	   different from 1 ... needed for ".mp3" fourcc */

	do
	{
		current_chunk = stts->table[i].sample_count;
		i--;
	}while(i >= 0 && current_chunk > chunk);	

//	return result*stts->table[i+1].sample_duration;

/*	if(stts->table[i+1].sample_duration)
	return result*stts->table[i+1].sample_duration;
	else
		return result*stts->table[i].sample_duration;
*/
	// Yann : the previous code assume that timescale and sample_rates are equals and valid
	// and this is not the case with fucked up .mp4 ... we're trying to bypass that ...

	if(stts->table[i+1].sample_duration)
		interm = result * stts->table[i+1].sample_duration;
	else
		interm = result * stts->table[i].sample_duration;

	return __int64((interm*trak->mdia.minf.stbl.stsd.table->sample_rate) / trak->mdia.mdhd.time_scale);
}

int oqt_trak_shift_offsets(oqt_trak_t *trak, __int64 offset)
{
	oqt_stco_t *stco = &(trak->mdia.minf.stbl.stco);
	int i;

	for(i = 0; i < stco->total_entries; i++)
	{
		stco->table[i].offset += offset;
	}
	return 0;
}
