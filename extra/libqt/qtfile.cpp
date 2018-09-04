#include "stdafx.h"
#include "qttype.h"
#include "qtfuncs.h"
#include "libqt.h"

int oqt_init(oqt_t* file)
{
	// Zero the memory
	memset(file, 0, sizeof(oqt_t));

	file->decompressed_buffer_size = 0;
	file->decompressed_buffer = NULL;
	file->decompressed_position = 0;

	// default: data is in the same file
	file->data_file = file;

	// No audio/video tracks yet
	file->total_atracks = 0;
	file->total_vtracks = 0;
	file->atracks = NULL;
	file->vtracks = NULL;

	// Initilise the data structures
	oqt_mdat_init(&(file->mdat));
	oqt_moov_init(&(file->moov));

	return 0;
}

int oqt_uninit(oqt_t *file)
{
	int i, result=0;

	// Close referenced external resources
	for(i = 0; i < file->ext_count; ++i) {
		if(file->ext_files[i]) 
		{
			result = file->ext_files[i]->fileio.libqt_close(file->ext_files[i]->fileio.file);
			if (!result) {
				perror("oqt_close");
			}
			free(file->ext_files[i]);
			file->ext_files[i] = NULL;
		}
	}


	// Delete Audio Tracks
	if(file->total_atracks) 
	{
		free(file->atracks);
		file->atracks = NULL;
		file->total_atracks = 0;
	}

	// Delete Video Tracks
	if(file->total_vtracks)
	{
		free(file->vtracks);
		file->vtracks = NULL;
		file->total_vtracks = 0;
	}

	// Delete preload buffer
	if(file->preload_size)
	{
		free(file->preload_buffer);
		file->preload_size = 0;
	}

	// Delete the atom trees
	oqt_moov_delete(&(file->moov));
	oqt_mdat_delete(&(file->mdat));

	return result;
}

int oqt_read_headers(oqt_t *file)
{
	int result = 0, found_moov = 0, found_mdat = 0;
	int i, track;
	__int64 orig_position = oqt_get_position(file);
	oqt_atom_t leaf_atom;
	// Move to start of file
	oqt_set_position(file, 0/*LL*/);

	// Find the postions of the movie header and data
	do
	{
		result = oqt_atom_read_header(file, &leaf_atom);
		if(oqt_atom_is(&leaf_atom,"jP  ")) result = 0;

		if(!result)
		{
			if(oqt_atom_is(&leaf_atom, "mdat")) 
			{
				oqt_read_mdat(file, &(file->mdat), &leaf_atom);
				found_mdat = 1;
			}
			else
				if(oqt_atom_is(&leaf_atom, "moov")) 
				{
					oqt_read_moov(file, &(file->moov), &leaf_atom);
					found_moov = 1;
				}
				else
					if(oqt_atom_is(&leaf_atom, "free") || oqt_atom_is(&leaf_atom, "skip")) 
					{
						// Free space in file
						oqt_atom_skip(file, &leaf_atom);
					}
					else {
#ifdef DEBUG
						fprintf(stderr, "oqt_read_headers: ignoring root '%4.4s' atom.\n", leaf_atom.type);
#endif
						oqt_atom_skip(file, &leaf_atom);
					}
		}
		//printf("oqt_read_headers: ftell 0x%llx length %lld\n", oqt_get_position(file), total_length);
		//	}while(!result && oqt_get_position(file) < total_length);
	}while(!result && (found_mdat + found_moov != 2));


#ifdef DEBUG
	if (!found_moov) fprintf(stderr, "oqt_read_headers: Didn't find moov atom in file.\n");
	if (!found_mdat) fprintf(stderr, "oqt_read_headers: Didn't find mdat atom in file.\n");
#endif


	// go back to the original position 
	oqt_set_position(file, orig_position);



	if(found_moov)
	{
		// get tables for all the different tracks
		file->total_atracks = oqt_get_audio_track_count(file);
		if (file->total_atracks) 
		{
			int size = sizeof(oqt_audio_map_t) * file->total_atracks;
			file->atracks = (oqt_audio_map_t*)calloc(1,size);
			for(i = 0, track = 0; i < file->total_atracks; i++)
			{
				while(!file->moov.trak[track]->mdia.minf.is_audio)
					track++;
				file->atracks[i].track = file->moov.trak[track];
				file->atracks[i].current_chunk = 1;
			}
		}

		file->total_vtracks = oqt_get_video_track_count(file);
		if (file->total_vtracks) 
		{
			int size = sizeof(oqt_video_map_t) * file->total_vtracks;
			file->vtracks = (oqt_video_map_t*)calloc(1,size);
			for(track = 0, i = 0; i < file->total_vtracks; i++)
			{
				while(!file->moov.trak[track]->mdia.minf.is_video)
					track++;
				file->vtracks[i].track = file->moov.trak[track];
				file->vtracks[i].current_chunk = 1;
			}
		}
	}

	return !found_moov;
}


int oqt_write_moviedata_init(oqt_t* file)
{
	// Start the mdat atom
	oqt_atom_write_header64(file, &file->mdat.atom, "mdat");
	return 0;	// Success
}

int oqt_write_moviedata_end(oqt_t* file)
{
	// End the 'mdat' atom and write the 'moov' atom
	oqt_atom_write_footer(file, &file->mdat.atom);
	oqt_write_moov(file, &(file->moov));
	return 0;	// Success
}











int oqt_get_video_track_count(oqt_t *file)
{
	int i, result = 0;
	for(i = 0; i < file->moov.total_tracks; i++)
	{
		if(file->moov.trak[i]->mdia.minf.is_video) result++;
	}
	return result;
}

int oqt_get_audio_track_count(oqt_t *file)
{
	int i, result = 0;
	for(i = 0; i < file->moov.total_tracks; i++)
	{
		if(file->moov.trak[i]->mdia.minf.is_audio) result++;
	}
	return result;
}

__int64 oqt_get_audio_length(oqt_t *file, int track)
{
	oqt_stts_t *stts = &(file->atracks[track].track->mdia.minf.stbl.stts);

	if(track>=0 && track < file->total_atracks) {
		/* Yann: I've modified this to handle samples with a duration 
		different from 1 ... needed for ".mp3" fourcc */

		//		return stts->table[0].sample_duration*oqt_track_samples(file, file->atracks[track].track);
		return (stts->table[0].sample_duration*oqt_track_samples(file, file->atracks[track].track)
			*oqt_get_audio_samplerate(file, track)) / file->atracks[track].track->mdia.mdhd.time_scale;
	}

	return 0;
}

__int64 oqt_get_video_length(oqt_t *file, int track)
{
	/*printf("oqt_video_length %d %lld\n", oqt_track_samples(file, file->vtracks[track].track), track); */
	if(track>=0 && track < file->total_vtracks)
		return oqt_track_samples(file, file->vtracks[track].track);
	return 0;
}

__int64 oqt_get_audio_position(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_atracks) {
		return file->atracks[track].current_sample;
	} else {
		fprintf(stderr, "oqt_get_audio_position: invalid track number (%d)\n", track);
	}

	return 0;
}

__int64 oqt_get_video_position(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_vtracks) {
		return file->vtracks[track].current_frame;
	} else {
		fprintf(stderr, "oqt_get_video_position: invalid track number (%d)\n", track);
	}

	return 0;
}



int oqt_set_audio_position(oqt_t *file, int track, __int64 sample)
{
	__int64 offset, chunk_sample;
	long chunk;
	oqt_trak_t *trak;
	long sd_id;


	if(track>=0 && track < file->total_atracks)
	{
		trak = file->atracks[track].track;
		file->atracks[track].current_sample = sample;
		//		printf("BEFORE  oqt_chunk_of_sample track %d sample %li\n", track, sample);
		oqt_chunk_of_sample(&chunk_sample, &chunk, trak, &sd_id, sample);
		// atrack->current_chunk is the chunk currently in the buffers,
		// don't change it until we really update the buffers
		//		printf("AFTER  oqt_chunk_of_sample chunk %d chunk_sample %d\n", chunk, chunk_sample);
		offset = oqt_sample_to_offset(trak, &sd_id, sample);
		//		printf("AFTER  oqt_sample_to_offset offset %li\n", offset);
		file->data_file = oqt_file_from_sd_id(file, trak, sd_id);
		if(!file->data_file) return -1;
		oqt_set_position(file->data_file, offset);
	}

	return 0;
}

int oqt_set_video_position(oqt_t *file, int track, __int64 frame)
{
	__int64 offset, chunk_sample;
	long chunk;
	oqt_trak_t *trak;
	long sd_id;

	if(track>=0 && track < file->total_vtracks)
	{
		trak = file->vtracks[track].track;
		file->vtracks[track].current_frame = frame;
		oqt_chunk_of_sample(&chunk_sample, &chunk, trak, &sd_id, frame);
		// vtrack->current_chunk is the chunk currently in the buffers,
		// don't change it until we really update the buffers
		offset = oqt_sample_to_offset(trak, &sd_id, frame);
		//fprintf(stderr, "oqt_set_video_position: offset: frame %lld -> 0x%llx\n",frame,offset);
		file->data_file = oqt_file_from_sd_id(file, trak, sd_id);
		if(!file->data_file) return -1;
		oqt_set_position(file->data_file, offset);
	} else {
		fprintf(stderr, "oqt_set_video_position: invalid track number (%d)\n", track);
	}

	return 0;
}


long oqt_get_audio_samplerate(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_atracks) {
		return (long)file->atracks[track].track->mdia.minf.stbl.stsd.table[0].sample_rate;
	} else {
		fprintf(stderr, "oqt_get_audio_samplerate: invalid track number (%d)\n", track);
	}

	return 0;
}

int oqt_get_audio_bits(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_atracks) 
	{
		oqt_stsd_table_t& table= file->atracks[track].track->mdia.minf.stbl.stsd.table[0];
		int bits = table.version==1?table.bytesPerPacket/table.samplesPerPacket*8:table.sample_size;
		if(bits==0&&table.sample_size!=0) bits = table.sample_size;
		return bits;
	} else {
		fprintf(stderr, "oqt_get_audio_bits: invalid track number (%d)\n", track);
	}

	return 0;
}

char* oqt_get_audio_compressor(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_atracks) {
		return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].format;
	} else {
		fprintf(stderr, "oqt_get_audio_compressor: invalid track number (%d)\n", track);
	}

	return NULL;
}

const oqt_codec_info_t* oqt_get_audio_codec(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_atracks) {
		return (const oqt_codec_info_t*)file->atracks[track].codec;
	} else {
		fprintf(stderr, "oqt_get_audio_codec: invalid track number (%d)\n", track);
	}

	return NULL;
}


int oqt_get_audio_channels(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_atracks) {
		return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].channels;
	} else {
		fprintf(stderr, "oqt_get_audio_channels: invalid track number (%d)\n", track);
	}

	return 0;
}

int oqt_get_audio_channel_loc(oqt_t *file, int *oqt_track, int *oqt_channel, int channel)
{
	int current_channel = 0, current_track = 0;
	*oqt_channel = 0;
	*oqt_track = 0;
	for(current_channel = 0, current_track = 0; current_track < file->total_atracks; )
	{
		if(channel >= current_channel)
		{
			*oqt_channel = channel - current_channel;
			*oqt_track = current_track;
		}

		current_channel += oqt_get_audio_channels(file,current_track);
		current_track++;
	}
	return 0;
}

int oqt_get_video_width(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_vtracks)
		return (int)file->vtracks[track].track->tkhd.track_width;
	return 0;
}

int oqt_get_video_height(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_vtracks)
		return (int)file->vtracks[track].track->tkhd.track_height;
	return 0;
}

int oqt_get_video_depth(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_vtracks)
		return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].depth;
	return 0;
}

void oqt_set_video_depth(oqt_t *file, int track, int depth)
{
	if(track>=0 && track < file->total_vtracks)
		file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].depth = depth;
}


float oqt_get_video_framerate(oqt_t *file, int track)
{
	oqt_stts_t *stts;
	__int64 samplecount = 0, duration = 0;
	int i;

	if(track>=0 && track < file->total_vtracks) {
		//		return (float)file->vtracks[track].track->mdia.mdhd.time_scale / 
		//			file->vtracks[track].track->mdia.minf.stbl.stts.table[0].sample_duration;

		stts = &(file->vtracks[track].track->mdia.minf.stbl.stts);

		// We're gonna return the framerate used the most, we simply returned the first one before
		for(i = 0; i < stts->total_entries; i++)
		{
			if(samplecount < stts->table[i].sample_count) {
				samplecount = stts->table[i].sample_count;
				duration = stts->table[i].sample_duration;
			}
		}
	}

	return (float)file->vtracks[track].track->mdia.mdhd.time_scale / duration;
}

__int64 oqt_get_video_frame_duration(oqt_t *file, int track, __int64 frame_number)
{
	// returns the duration of the frame in 1/10000000 second .... :)
	oqt_stts_t *stts;
	__int64 samplecount = 0, duration = 0;
	double sec_duration;
	int i;

	if(track >=0 && track < file->total_vtracks) {
		stts = &(file->vtracks[track].track->mdia.minf.stbl.stts);

		for(i = 0; i < stts->total_entries; i++)
		{
			samplecount += stts->table[i].sample_count;
			duration = stts->table[i].sample_duration;
			if(samplecount >= frame_number)
				break;
		}

		sec_duration = (double)duration/(double)file->vtracks[track].track->mdia.mdhd.time_scale;
		duration = (__int64)(sec_duration * 10000000);
	}

	return duration;
}

long oqt_get_audio_frame_duration(oqt_t *file, int track, __int64 frame_number)
{
	// returns the duration of the frame in samples
	oqt_stts_t *stts;
	__int64 samplecount = 0, duration = 0;
	int i;

	if(track >=0 && track < file->total_atracks) {
		stts = &(file->atracks[track].track->mdia.minf.stbl.stts);

		for(i = 0; i < stts->total_entries; i++)
		{
			samplecount += stts->table[i].sample_count;
			duration = stts->table[i].sample_duration;
			if(samplecount >= frame_number)
				break;
		}
	}

	return (long)duration;
}

__int64 oqt_get_video_poster(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_vtracks)
		return (__int64)(file->moov.mvhd.poster_time /
		((float)file->vtracks[track].track->mdia.minf.stbl.stts.table[0].sample_duration *
		((float)file->vtracks[track].track->mdia.mdhd.time_scale / file->moov.mvhd.time_scale)));

	return 0;
}

void oqt_set_video_poster(oqt_t *file, int track, __int64 frame)
{
	if(track>=0 && track < file->total_vtracks)
		file->moov.mvhd.poster_time = (int)(frame * 
		((float)file->vtracks[track].track->mdia.minf.stbl.stts.table[0].sample_duration *
		((float)file->vtracks[track].track->mdia.mdhd.time_scale / file->moov.mvhd.time_scale)));
}


void oqt_set_video_framerate(oqt_t *file, float framerate)
{
	int i;
	int new_time_scale, new_sample_duration;

	new_time_scale = oqt_get_timescale(framerate);
	new_sample_duration = (int)((float)new_time_scale / framerate + 0.5);

	for(i = 0; i < file->total_vtracks; i++)
	{
		file->vtracks[i].track->mdia.mdhd.time_scale = new_time_scale;
		file->vtracks[i].track->mdia.minf.stbl.stts.table[0].sample_duration = new_sample_duration;
	}
}

char* oqt_get_video_compressor(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_vtracks)
		return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].format;

	return NULL;
}

const oqt_codec_info_t* oqt_get_video_codec(oqt_t *file, int track)
{
	if(track>=0 && track < file->total_vtracks)
		return (const oqt_codec_info_t*)file->vtracks[track].codec;

	return NULL;
}

/* stux: it was necessary to get audio sample sizes for framed audio */
long oqt_get_audio_framesize(oqt_t *file, int track, __int64 frame)
{
	long bytes = 0;
	oqt_trak_t *trak;

	if(track>=0 && track < file->total_atracks)
		trak = file->atracks[track].track;
	else
		return 0;

	if( trak->mdia.minf.stbl.stsz.sample_size )
	{
		bytes = (long)trak->mdia.minf.stbl.stsz.sample_size;
	}
	else
	{
		long total_frames = (long)oqt_track_samples(file, trak);

		if(frame < 0)
			frame = 0;
		else if(frame > total_frames - 1)
			frame = total_frames - 1;

		bytes = (long)trak->mdia.minf.stbl.stsz.table[frame].size;
	}


	return bytes;
}

long oqt_get_video_framesize(oqt_t *file, int track, __int64 frame)
{
	long bytes = 0;
	oqt_trak_t *trak;

	if(track>=0 && track < file->total_vtracks)
		trak = file->vtracks[track].track;
	else
		return 0;

	if(trak->mdia.minf.stbl.stsz.sample_size)
	{
		bytes = (long)trak->mdia.minf.stbl.stsz.sample_size;
	}
	else
	{
		long total_frames = (long)oqt_track_samples(file, trak);

		if(frame < 0)
			frame = 0;
		else if(frame > total_frames - 1)
			frame = total_frames - 1;

		bytes = (long)trak->mdia.minf.stbl.stsz.table[frame].size;
	}


	return bytes;
}


long oqt_get_audio_chunks(oqt_t *file, int track)
{
	return file->atracks[track].track->mdia.minf.stbl.stco.total_entries;
}

/* the length in bytes of a chunk
chunk should be a number greater than 0 */
long oqt_get_audio_chunksize(oqt_t *file,int track,long chunk)
{
	oqt_audio_map_t *atrack = &(file->atracks[track]);
	oqt_stsc_table_t *stsc_table = atrack->track->mdia.minf.stbl.stsc.table;
	oqt_stsz_table_t *stsz_table = atrack->track->mdia.minf.stbl.stsz.table;
	int stsc_entries = atrack->track->mdia.minf.stbl.stsc.total_entries;
	int stsz_entries = atrack->track->mdia.minf.stbl.stsz.total_entries;

	int sample_size = (int)atrack->track->mdia.minf.stbl.stsz.sample_size;
	int chunk_bytes=0, first_sample=0;//, offset_bytes;
	int i, entry;


	for(entry=0,i=1;i<=chunk;i++) 
	{
		if ((entry+1)<stsc_entries && stsc_table[entry+1].chunk==i) entry++;

		if (i==chunk) break;
		first_sample+=stsc_table[entry].samples;
		//fprintf(stdout, "chunk=%.2d    entry=%.2d   ec=%.2d   samples=%.2d   total_samples=%d\n",
		//			i, entry, stsc_table[entry].chunk, stsc_table[entry].samples, first_sample);
		if (entry>=stsc_entries) {
			fprintf(stderr, "oqt_get_audio_chunksize: ran out of stsc entries (%d).\n", entry);
			break;
		}
	}


	// Ok now we know what sample we have got to
	//fprintf(stdout, "%lld: upto=%ld   entry=%ld   this chunk=%ld\n", chunk, 
	//		first_sample, entry, stsc_table[entry].samples);


	if(sample_size)
	{

		if (sample_size == 1) 
		{
			// Usually a sample size of 1 is meaningless and
			// we must calculate it ourselves...
			chunk_bytes = oqt_get_audio_frames_to_bytes(file, track, stsc_table[entry].samples);

			if (chunk_bytes<=0 && stsc_table[entry].samples>0) {
				fprintf(stderr, "oqt_get_audio_chunksize: No stsz table and codec does not support frames_to_bytes().\n");
				chunk_bytes = sample_size * stsc_table[entry].samples;	// Poor attempt at a guess
			}

		} else {

			chunk_bytes = sample_size>stsc_table[entry].samples?sample_size:sample_size * stsc_table[entry].samples;
		}

		// If sample_size is zero then look it up in the table
	} else {
		int last_sample = first_sample + stsc_table[entry].samples;

		for(i = first_sample; i < last_sample; ++i)
		{
			if (i>=stsz_entries) {
				fprintf(stderr, "oqt_get_audio_chunksize: ran out of stsz entries (%d).\n", i);
				break;
			}
			chunk_bytes += (int)stsz_table[i].size;
		}
	}

	// Difference between the 2 sound chunk offsets 
	//offset_bytes =  oqt_chunk_to_offset(atrack->track, chunk+1) - 
	//			   oqt_chunk_to_offset(atrack->track, chunk);
	//fprintf(stdout, "Newly calced length of chunk is %d\n", chunk_bytes);
	//fprintf(stdout, "Offset Length of chunk %d is %d\n", chunk, offset_bytes);

	return chunk_bytes;
}

static long oqt_sd_id_of_chunk(oqt_t *file,oqt_trak_t *trak,long chunk)
{
	oqt_stsc_table_t *stsc_table = trak->mdia.minf.stbl.stsc.table;
	int stsc_entries = trak->mdia.minf.stbl.stsc.total_entries;
	int first_sample=0;
	int i, entry;


	for(entry=0,i=1;i<=chunk;i++) {
		if ((entry+1)<stsc_entries && stsc_table[entry+1].chunk==i) entry++;
		if (i==chunk) {
			return stsc_table[entry].id;
		}
		first_sample+=stsc_table[entry].samples;
		if (entry>=stsc_entries) {
			fprintf(stderr, "oqt_sd_id_of_chunk: ran out of stsc entries (%d).\n", entry);
			break;
		}
	}

	// failed for whatever reason
	// be on the safe side and just say 1 (should always be there)
	return 1;
}

int oqt_set_audio_position_to_chunk(oqt_t *file, int track, long chunk)
{
	oqt_audio_map_t *atrack = &(file->atracks[track]);
	long sd_id = oqt_sd_id_of_chunk(file, atrack->track, chunk);
	__int64 offset = oqt_chunk_to_offset(atrack->track, chunk);
	file->data_file = oqt_file_from_sd_id(file, atrack->track, sd_id);
	if(!file->data_file) return -1;
	return oqt_set_position(file->data_file, offset);
}

__int64 oqt_get_audio_chunk_samples(oqt_t *file, int track, long chunk)
{
	oqt_audio_map_t *atrack = &(file->atracks[track]);
	return oqt_chunk_samples(atrack->track, chunk);
}


long oqt_add_audio_track(oqt_t *file,int channels,long sample_rate,int bits,char *compressor)
{
	oqt_trak_t *trak;

	file->atracks = (oqt_audio_map_t*)realloc(file->atracks,
		sizeof(oqt_audio_map_t)*(file->total_atracks+1));
	if (!file->atracks) {
		perror("realloc failed for atracks in oqt_add_audio_track\n");
		return -1;
	}

	// Add a track to the file
	trak = oqt_add_trak(&(file->moov));
	if (!trak) {
		perror("oqt_add_trak failed in oqt_add_video_track\n");
		return -1;
	}

	file->total_atracks++;
	oqt_trak_init_audio(file, trak, channels, sample_rate, bits, compressor);
	file->atracks[file->total_atracks-1].track = trak;
	file->atracks[file->total_atracks-1].current_sample = 0;
	file->atracks[file->total_atracks-1].current_chunk = 1;
	return file->total_atracks-1;   // Return the number of the track created
}


long oqt_add_video_track(oqt_t *file,int frame_w,int frame_h,float frame_rate,char *compressor)
{
	oqt_trak_t *trak;

	// *** Check to see this has already been inited ? ***
	oqt_mhvd_init_video(file, &(file->moov.mvhd), frame_rate);

	file->vtracks = (oqt_video_map_t*)realloc(file->vtracks,
		sizeof(oqt_video_map_t)*(file->total_vtracks+1));
	if (!file->vtracks) {
		perror("realloc failed for vtracks in oqt_add_video_track\n");
		return -1;
	}

	// Add a track to the file
	trak = oqt_add_trak(&(file->moov));
	if (!trak) {
		perror("oqt_add_trak failed in oqt_add_video_track\n");
		return -1;
	}

	// Increment the number of video tracks
	file->total_vtracks++;

	oqt_trak_init_video(file, trak, frame_w, frame_h, frame_rate, compressor);

	file->vtracks[file->total_vtracks-1].track = trak;
	file->vtracks[file->total_vtracks-1].current_frame = 0;
	file->vtracks[file->total_vtracks-1].current_chunk = 1;
	return file->total_vtracks-1;   // Return the number of the track created
}




long oqt_get_audio_frames_to_bytes(oqt_t *file,int track,int frames)
{
	int channels = oqt_get_audio_channels(file, track);
	int bytesPerPacket = file->atracks[track].track->mdia.minf.stbl.stsd.table[0].bytesPerPacket;
	int samplesPerPacket = file->atracks[track].track->mdia.minf.stbl.stsd.table[0].samplesPerPacket;
	int bytesPerSample = file->atracks[track].track->mdia.minf.stbl.stsd.table[0].sample_size>>3;

	int bytes=0;
	//fprintf(stderr, "oqt_get_audio_frames_to_bytes: codec=%d bytesPerPacket=%d samplesPerPacket=%d frames=%d channels=%d\n",
	//		 bytes, bytesPerPacket, samplesPerPacket, frames, channels);

	// Try using the value from codecs function first
	if (bytes>0) return bytes;

	// Now try calculating it from the stsd
	if (bytesPerPacket && samplesPerPacket)
		return (long)(((float)frames/samplesPerPacket)*bytesPerPacket*channels);
	else if(bytesPerSample)
		return frames*bytesPerSample*channels;
	// Erm, can't calculate it - return an error
	return -1;
}



/* return -1 if there is NO keyframe before */
__int64 oqt_get_video_keyframe_before(oqt_t *file, int track, __int64 frame)
{
	oqt_trak_t *trak = file->vtracks[track].track;
	oqt_stss_t *stss = &trak->mdia.minf.stbl.stss;
	int lo, hi;

	/* per spec: if this table is absent, every sample is a sync sample */
	if (!stss->table) return frame;

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

/* return -1 if there is NO keyframe after */
__int64 oqt_get_video_keyframe_after(oqt_t *file, int track, __int64 frame)
{
	oqt_trak_t *trak = file->vtracks[track].track;
	oqt_stss_t *stss = &trak->mdia.minf.stbl.stss;
	int lo, hi;

	/* per spec: if this table is absent, every sample is a sync sample */
	if (!stss->table) return frame;

	lo = 0;
	hi = stss->total_entries-1;
	if (stss->table[lo].sample-1>=frame) return stss->table[lo].sample-1;
	if (stss->table[hi].sample-1< frame) return -1;
	while (hi>lo+1) {
		/* here: stss->table[lo].sample-1<frame
		stss->table[hi].sample-1>=frame */	       
		int med = (lo+hi)/2;
		if (stss->table[med].sample-1<frame) lo = med; else hi = med;
	}
	/* here: hi=lo+1 */
	return stss->table[hi].sample-1;
}


void oqt_insert_video_keyframe(oqt_t *file, int track, __int64 frame)
{
	oqt_trak_t *trak = file->vtracks[track].track;
	oqt_stss_t *stss = &trak->mdia.minf.stbl.stss;
	int i;

	//printf("Total Entries %lu stss->entries_allocated %lu\n", stss->total_entries, stss->entries_allocated);

	// Get the keyframe greater or equal to new frame
	for(i = 0; i < stss->total_entries; i++)
	{
		if(stss->table[i].sample >= frame) break;
	}

	// Expand table
	if (stss->entries_allocated <= stss->total_entries)
	{
		stss->entries_allocated = stss->entries_allocated*2+1;
		stss->table = (oqt_stss_table_t*)realloc(stss->table, sizeof(oqt_stss_table_t) * stss->entries_allocated);
	}

	// Insert before existing frame
	if(i < stss->total_entries)
	{
		if(stss->table[i].sample > frame)
		{
			int j, k;
			for(j = stss->total_entries, k = stss->total_entries - 1;
				k >= i;
				j--, k--)
			{
				stss->table[j] = stss->table[k];
			}
			stss->table[i].sample = (long)frame;
		}
	}
	else
		// Insert after last frame
		stss->table[i].sample = (long)frame;
	stss->total_entries++;
}

int oqt_get_mp4_video_decoder_config(oqt_t *file, int track, BYTE** ppBuf, int* pBufSize)
{
	oqt_esds_t* esds;

	if (!file->total_vtracks) {
		return 0;
	}
	esds = &file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].esds;
	return oqt_esds_get_decoder_config(esds, ppBuf, pBufSize);
}

int oqt_set_mp4_video_decoder_config(oqt_t *file, int track, BYTE* pBuf, int bufSize)
{
	oqt_esds_t* esds;

	if (!file->total_vtracks) {
		return 0;
	}
	esds = &file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].esds;
	return oqt_esds_set_decoder_config(esds, pBuf, bufSize);
}

int oqt_get_mp4_audio_decoder_config(oqt_t *file, int track, BYTE** ppBuf, int* pBufSize)
{
	oqt_esds_t* esds;

	if (!file->total_atracks) {
		return 0;
	}
	esds = &file->atracks[track].track->mdia.minf.stbl.stsd.table[0].esds;
	return oqt_esds_get_decoder_config(esds, ppBuf, pBufSize);
}

int oqt_set_mp4_audio_decoder_config(oqt_t *file, int track, BYTE* pBuf, int bufSize)
{
	oqt_esds_t* esds;

	if (!file->total_atracks) {
		return 0;
	}
	esds = &file->atracks[track].track->mdia.minf.stbl.stsd.table[0].esds;
	return oqt_esds_set_decoder_config(esds, pBuf, bufSize);
}

int oqt_get_iod_audio_profile_level(oqt_t *file)
{
	return file->moov.iods.audioProfileId;
}

int oqt_set_iod_audio_profile_level(oqt_t *file, int id)
{
	file->use_mp4 = 1;
	return oqt_iods_set_audio_profile(&file->moov.iods, id);
}

int oqt_get_iod_video_profile_level(oqt_t *file)
{
	return file->moov.iods.videoProfileId;
}

int oqt_set_iod_video_profile_level(oqt_t *file, int id)
{
	file->use_mp4 = 1;
	return oqt_iods_set_video_profile(&file->moov.iods, id);
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


int oqt_write_video_data(
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
