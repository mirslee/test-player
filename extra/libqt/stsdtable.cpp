/* stsdtable.c
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
 * $Id: stsdtable.c,v 1.21 2003/04/07 21:02:24 shitowax Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"
#pragma warning(disable:4996)



void oqt_mjqt_init(oqt_mjqt_t *mjqt)
{
}

void oqt_mjqt_delete(oqt_mjqt_t *mjqt)
{
}

void oqt_mjqt_dump(oqt_mjqt_t *mjqt)
{
}


void oqt_mjht_init(oqt_mjht_t *mjht)
{
}

void oqt_mjht_delete(oqt_mjht_t *mjht)
{
}

void oqt_mjht_dump(oqt_mjht_t *mjht)
{
}


static void oqt_read_private_stsddata(oqt_t *file, oqt_stsdwave_t *wave, int data_len)
{
	//fprintf(stderr, "read_private_stsddata: reading in %d bytes of private data.\n", data_len);

	if (wave->extra_len || wave->extra) {
		fprintf(stderr, "read_private_stsddata: error, private data buffer already has data in it.\n");
		oqt_set_position(file, oqt_get_position(file) + data_len);
	} else {
	
		wave->extra_len = data_len;
		wave->extra = (char*)malloc(data_len);
		oqt_read_data(file, (char*)wave->extra, data_len);
		
	}
}


int oqt_read_stsdwave(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom)
{
	oqt_stsdwave_t *wave = &(table->wave);
	oqt_atom_t format_atom, fmt_atom;

	oqt_atom_read_header(file, &format_atom);
	
	// Is it a 'frma' atom ?
	if (!oqt_match_32((char*)format_atom.type, "frma")) {
		fprintf(stderr,"oqt_read_stsdwave: Error, atom after 'wave' is not 'frma' (%.4s).\n", format_atom.type);
		oqt_atom_skip(file, &format_atom);
		// **** Use private memory handle instead ***
		return 0;
	}
	
	oqt_read_char32(file, wave->fmtType);	// ms 0x55

	// Read in the format atom
	oqt_atom_read_header(file, &fmt_atom);
	if (oqt_match_32((char*)fmt_atom.type, wave->fmtType)) {

		// Guess if it is WAVEFORMATEX
		if (fmt_atom.size>20) {
			// Read in WAVEFORMATEX structure (little endian rubbish)
			wave->wFormatTag = oqt_read_le_int16(file);
			wave->nChannels = oqt_read_le_int16(file);	// Could verify this value...
			wave->nSamplesPerSec = oqt_read_le_int32(file);
			wave->nAvgBytesPerSec = oqt_read_le_int32(file);
			wave->nBlockAlign = oqt_read_le_int16(file);
			wave->wBitsPerSample = oqt_read_le_int16(file);
			wave->cbSize = oqt_read_le_int16(file);
		
			// Further extension ?
			if (wave->wFormatTag == OQT_STSDWAVE_MPEG) {
				if (wave->cbSize!=12)
					fprintf(stderr,"oqt_read_stsdwave: Extension length for OQT_STSDWAVE_MPEG is not 12.\n");
				wave->wID = oqt_read_le_int16(file);
				wave->fdwFlags = oqt_read_le_int32(file);
				wave->nBlockSize = oqt_read_le_int16(file);
				wave->nFramesPerBlock = oqt_read_le_int16(file);
				wave->nCodecDelay = oqt_read_le_int16(file);
			} else {
				oqt_read_private_stsddata( file, wave, wave->cbSize );
			}
		} else {
			oqt_read_private_stsddata( file, wave, (int)(fmt_atom.size - OQT_ATOM_HEAD_LENGTH) );
		}
	
	} else {
		fprintf(stderr,"oqt_read_stsdwave: Warning, format atom (%.4s) did not match 'frma' atom's value (%.4s).\n",
		fmt_atom.type, wave->fmtType);
		oqt_atom_skip(file, &fmt_atom);
	}

	// Leaf atoms after all of this ?
	while(oqt_get_position(file) < parent_atom->end)
	{
		oqt_atom_t leaf_atom;
		oqt_atom_read_header(file, &leaf_atom);

		if(leaf_atom.size==0)
		{
			// Skip padding
			oqt_set_position(file, leaf_atom.start+OQT_ATOM_HEAD_LENGTH);
		}
		else if(oqt_atom_is(&leaf_atom, "esds"))
		{
			// Elementary Stream Descriptor
			oqt_read_esds(file, &(table->esds));
			
		}
		else if(oqt_atom_is(&leaf_atom, "enda"))
		{
			// Endian Atom
			wave->little_endian = oqt_read_int16(file);
		}
		else if(oqt_atom_is_null(&leaf_atom))
		{
			// Ignore NULL atoms
		}
		else {
#ifdef DEBUG		
			fprintf(stderr, "oqt_read_stsdwave: skipping [0x%.8x] atom\n", (int)*((long*)&leaf_atom.type));
#endif		
			oqt_atom_skip(file, &leaf_atom);
		}
	}
	
	return 0;
}

void oqt_read_stsd_audio(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom)
{
	table->version = oqt_read_int16(file);
	table->revision = oqt_read_int16(file);
	oqt_read_data(file, table->vendor, 4);
	table->channels = oqt_read_int16(file);
	table->sample_size = oqt_read_int16(file);
	table->compression_id = oqt_read_int16(file);
	table->packet_size = oqt_read_int16(file);
	table->sample_rate = oqt_read_fixed32(file);
	
	if(table->version == 1)
	{ /* Version 1 */
		table->samplesPerPacket = oqt_read_uint32(file);
		table->bytesPerPacket = oqt_read_uint32(file);
		table->bytesPerFrame = oqt_read_uint32(file);
		table->bytesPerSample = oqt_read_uint32(file);
	}
	
	// Extra atoms after the standard fields
	while(oqt_get_position(file) < parent_atom->end)
	{
		oqt_atom_t leaf_atom;
		oqt_atom_read_header(file, &leaf_atom);
		
		if(oqt_atom_is(&leaf_atom, "wave"))
		{
			oqt_read_stsdwave(file, table, &leaf_atom);
		}
		else if(oqt_atom_is_null(&leaf_atom))
		{
			// Ignore NULL atoms
		}
		else {
			oqt_read_esds(file, &(table->esds));
			/* oqt_read_esds may not read the whole atom */
			oqt_atom_skip(file, &leaf_atom);
		}
	} 

}



void oqt_write_stsdwave(oqt_t *file, oqt_stsdwave_t *wave, oqt_esds_t *esds)
{
	oqt_atom_t wave_atom, frma_atom, format_atom;
	oqt_atom_t endian_atom, null_atom;
	char nullchar[4] = {0,0,0,0};
	
	// The the channels have been set, to write the wave extension
	if (wave->nChannels) {
		// The wave atom
		oqt_atom_write_header(file, &wave_atom, "wave");
		
		// The format atom
		oqt_atom_write_header(file, &frma_atom, "frma");
		oqt_write_char32(file, wave->fmtType);
		oqt_atom_write_footer(file, &frma_atom);
		
		// The Format specific atom
		if( wave->fmtType[0] == 'm' && wave->fmtType[1] == 'p' && wave->fmtType[2] == '4' && wave->fmtType[3] == 'a' ) {
			char wtfisthat[4] = {(char)0xa6,(char)0xdf,0,0};
			oqt_atom_write_header(file, &format_atom, wave->fmtType);
			oqt_write_char32(file, nullchar);
			oqt_atom_write_footer(file, &format_atom);
			oqt_write_esds_audio(file, esds, 1/* fix me */);

			// Null Atom
			oqt_atom_write_header(file, &null_atom, nullchar);
			oqt_atom_write_footer(file, &null_atom);

			oqt_write_char32(file, nullchar);
			oqt_write_char32(file, wtfisthat);
		} else {
			oqt_atom_write_header(file, &format_atom, wave->fmtType);
			oqt_write_le_int16(file,wave->wFormatTag);
			oqt_write_le_int16(file, wave->nChannels);
			oqt_write_le_int32(file, wave->nSamplesPerSec);
			oqt_write_le_int32(file, wave->nAvgBytesPerSec);
			oqt_write_le_int16(file, wave->nBlockAlign);
			oqt_write_le_int16(file, wave->wBitsPerSample);
			oqt_write_le_int16(file, wave->cbSize);
		
			// MPEG Audio Extension Extension
			if (wave->wFormatTag == OQT_STSDWAVE_MPEG) {
				oqt_write_le_int16(file, wave->wID);
				oqt_write_le_int32(file, wave->fdwFlags);
				oqt_write_le_int16(file, wave->nBlockSize);
				oqt_write_le_int16(file, wave->nFramesPerBlock);
				oqt_write_le_int16(file, wave->nCodecDelay);
			} else {
				if (wave->extra) {
					oqt_write_data(file, (char*)wave->extra, wave->cbSize);
				}
			}
			oqt_atom_write_footer(file, &format_atom);
			
			// Endian Atom
			oqt_atom_write_header(file, &endian_atom, "enda");
			oqt_write_int16(file, wave->little_endian);
			oqt_atom_write_footer(file, &endian_atom);
		
			// Null Atom
			oqt_atom_write_header(file, &null_atom, nullchar);
			oqt_atom_write_footer(file, &null_atom);
		}

		
		// End the Wave atom
		oqt_atom_write_footer(file, &wave_atom);
	}
}

void oqt_write_stsd_audio(oqt_t *file, oqt_stsd_table_t *table)
{
	oqt_write_int16(file, (short)table->version);
	oqt_write_int16(file, (short)table->revision);
	oqt_write_data(file, table->vendor, 4);
	oqt_write_int16(file, (short)table->channels);
	oqt_write_int16(file, (short)table->sample_size);
	oqt_write_int16(file, (short)table->compression_id);
	oqt_write_int16(file, (short)table->packet_size);
	oqt_write_fixed32(file, table->sample_rate);
	if(table->version == 1)
	{ /* Version 1 */
		oqt_write_uint32(file, table->samplesPerPacket);
		oqt_write_uint32(file, table->bytesPerPacket);
		oqt_write_uint32(file, table->bytesPerFrame);
		oqt_write_uint32(file, table->bytesPerSample);
		oqt_write_stsdwave(file, &(table->wave), &(table->esds));
	} else if(file->use_mp4) {
		oqt_write_esds_audio(file, &(table->esds),
			file->atracks[0].track->tkhd.track_id);
	}
}

void oqt_read_stsd_video(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom)
{
	table->version = oqt_read_int16(file);
	table->revision = oqt_read_int16(file);
	oqt_read_data(file, table->vendor, 4);
	table->temporal_quality = oqt_read_int32(file);
	table->spatial_quality = oqt_read_int32(file);
	table->width = oqt_read_int16(file);
	table->height = oqt_read_int16(file);
	table->dpi_horizontal = oqt_read_fixed32(file);
	table->dpi_vertical = oqt_read_fixed32(file);
	table->data_size = oqt_read_int32(file);
	table->frames_per_sample = oqt_read_int16(file);
	oqt_read_char(file);
	oqt_read_data(file, table->compressor_name, 31);
	table->depth = oqt_read_int16(file);
	table->ctab_id = oqt_read_int16(file);
	
	
	
	while(oqt_get_position(file) < parent_atom->end)
	{
		oqt_atom_t leaf_atom = {0};
		if(oqt_atom_read_header(file, &leaf_atom)) break;

/*		fprintf(stderr, "oqt_read_stsd_video: [%.4s] 0x%llx -> 0x%llx (%lld) 0x%llx (parent end 0x%llx)\n",
			leaf_atom.type,
			leaf_atom.start, leaf_atom.end,
			leaf_atom.size, oqt_get_position(file), parent_atom->end);*/

		if(leaf_atom.size==0)
		{
			// Skip padding
			oqt_set_position(file, leaf_atom.start+OQT_ATOM_HEAD_LENGTH);
		}
		else
		if(oqt_atom_is(&leaf_atom, "ctab"))
		{
			oqt_read_ctab(file, &(table->ctab));
		}
		else
		if(oqt_atom_is(&leaf_atom, "gama"))
		{
			table->gamma = oqt_read_fixed32(file);
		}
		else
		if(oqt_atom_is(&leaf_atom, "fiel"))
		{
			table->fields = oqt_read_char(file);
			table->field_dominance = oqt_read_char(file);
		}
		else
		if(oqt_atom_is(&leaf_atom, "esds"))
		{
			oqt_read_esds(file, &(table->esds));
			/* oqt_read_esds may not read the whole atom */
			oqt_atom_skip(file, &leaf_atom);
		}
		else
		if(oqt_atom_is(&leaf_atom, "strf")||oqt_atom_is(&leaf_atom, "avcC")||oqt_atom_is(&leaf_atom, "SMI ")||oqt_atom_is(&leaf_atom, "jp2h"))
		{
			table->strf = (BYTE*)malloc((size_t)leaf_atom.size);
			table->strf_len = (long)leaf_atom.size;
			oqt_read_data(file, (char*)table->strf, table->strf_len);
		}
//		else
//		if(oqt_atom_is(&leaf_atom, "mjqt"))
//		{
//			oqt_read_mjqt(file, &(table->mjqt));
//		}
//		else
//		if(oqt_atom_is(&leaf_atom, "mjht"))
//		{
//			oqt_read_mjht(file, &(table->mjht));
//		}
		else {
#ifdef DEBUG		
			fprintf(stderr, "oqt_read_stsd_video: skipping [%.4s] atom\n", leaf_atom.type);
#endif		
//			table->strf = (BYTE*)malloc(leaf_atom.size);
//			table->strf_len = (long)leaf_atom.size;
//			oqt_read_data(file, (char*)table->strf, table->strf_len);
			oqt_atom_skip(file, &leaf_atom);
		}
	}
	
	if (table->ctab_id>0) {
		// Load a default colour table
		oqt_ctab_default(&(table->ctab), table->ctab_id);
		
	} else if (table->ctab_id==0 && table->ctab.size==0)  {
		// When ID is zero there should have been a ctab atom
#ifdef _WIN32		
		OutputDebugString("oqt_read_stsd_video: Warning ctab atom is missing.\n");
		
#endif		
	}
	
//printf("oqt_read_stsd_video 2\n");
}

void oqt_write_stsd_video(oqt_t *file, oqt_stsd_table_t *table)
{
	oqt_write_int16(file, (short)table->version);
	oqt_write_int16(file, (short)table->revision);
	oqt_write_data(file, table->vendor, 4);
	oqt_write_int32(file, table->temporal_quality);
	oqt_write_int32(file, table->spatial_quality);
	oqt_write_int16(file, (short)table->width);
	oqt_write_int16(file, (short)table->height);
	oqt_write_fixed32(file, table->dpi_horizontal);
	oqt_write_fixed32(file, table->dpi_vertical);
	oqt_write_int32(file, (long)table->data_size);
	oqt_write_int16(file, (short)table->frames_per_sample);
	oqt_write_char(file, (char)strlen(table->compressor_name));
	oqt_write_data(file, table->compressor_name, 31);
	oqt_write_int16(file, (short)table->depth);
	oqt_write_int16(file, (short)table->ctab_id);
	
	if(table->fields)
	{
		oqt_atom_t atom;

		oqt_atom_write_header(file, &atom, "fiel");
		oqt_write_char(file, (char)table->fields);
		oqt_write_char(file, (char)table->field_dominance);
		oqt_atom_write_footer(file, &atom);
	}
	
	if(table->strf)
	{
		oqt_atom_t atom;

		oqt_atom_write_header(file, &atom, "strf");
		oqt_write_data(file, (char*)table->strf, table->strf_len);
		oqt_atom_write_footer(file, &atom);
	}
	
	// Only write ctab atom if table exists and ID is zero
	if (table->ctab.size && table->ctab_id==0)
	{
		oqt_write_ctab(file, &(table->ctab));
	}

	if(table->esds.decoderConfigLen > 0)
	{
		oqt_write_esds_video(file, &(table->esds), file->vtracks[0].track->tkhd.track_id);
	}
}


void oqt_read_stsd_strm(oqt_t *file, oqt_stsd_table_t *table, oqt_atom_t *parent_atom)
{
	oqt_read_int32(file);
	oqt_read_int32(file);
	oqt_read_int32(file);
	while(oqt_get_position(file) < parent_atom->end)
	{
		oqt_atom_t leaf_atom;
		oqt_atom_read_header(file, &leaf_atom);
		if(leaf_atom.size==0)
			oqt_set_position(file, leaf_atom.start+OQT_ATOM_HEAD_LENGTH);
		else if(oqt_atom_is(&leaf_atom, "sean"))
			{
				oqt_read_int32(file);
				oqt_read_int32(file);
				oqt_read_int32(file);
				oqt_atom_t leaf_atom;
				oqt_atom_read_header(file, &leaf_atom);
				if(oqt_atom_is(&leaf_atom, "m12v"))
					oqt_read_m12v(file, &table->m12v,&leaf_atom);
			}
		oqt_atom_skip(file, &leaf_atom);
	}	
}

void oqt_write_stsd_strm(oqt_t *file, oqt_stsd_table_t *table)
{

}

void oqt_read_stsd_table(oqt_t *file, oqt_minf_t *minf, oqt_stsd_table_t *table)
{
	oqt_atom_t leaf_atom;

	oqt_atom_read_header(file, &leaf_atom);
	
	table->format[0] = leaf_atom.type[0];
	table->format[1] = leaf_atom.type[1];
	table->format[2] = leaf_atom.type[2];
	table->format[3] = leaf_atom.type[3];
	
	// Work arround for very old versions of quicktime
	// leaving format as null for uncompressed tracks
	if (table->format[0]==0 && table->format[1]==0 &&
	    table->format[2]==0 && table->format[3]==0)
	{
		table->format[0] = 'r';
		table->format[1] = 'a';
		table->format[2] = 'w';
		table->format[3] = ' ';
	}	
	
	oqt_read_data(file, (char*)table->reserved, 6);
	table->data_reference = oqt_read_int16(file);

	if(minf->is_audio) oqt_read_stsd_audio(file, table, &leaf_atom);
	if(minf->is_video) oqt_read_stsd_video(file, table, &leaf_atom);
	if(minf->is_strm) oqt_read_stsd_strm(file, table, &leaf_atom);
}

void oqt_stsdwave_init(oqt_stsdwave_t *wave)
{
	// We don't know enough here to do things
	// values must be set to sensible values by Codec

	wave->fmtType[0] = 0;
	wave->fmtType[1] = 0;
	wave->fmtType[2] = 0;
	wave->fmtType[3] = 0;
	wave->little_endian = 0;
	
	wave->wFormatTag = 0;
	wave->nChannels = 0;		// When this set to zero or less
								// then the wave header isn't written
	wave->nSamplesPerSec = 0;
	wave->nAvgBytesPerSec = 0;
	wave->nBlockAlign = 0;
	wave->wBitsPerSample = 0;
	wave->cbSize = 0;
	
	/* MPEG Audio extension (0x55) */
    wave->wID = 0;
	wave->fdwFlags = 0;
	wave->nBlockSize = 0;
	wave->nFramesPerBlock = 0;
	wave->nCodecDelay = 0;
	
	/* Other formats are shoved in buffer */
	wave->extra_len = 0;
	wave->extra = NULL;
}




void oqt_stsd_table_init(oqt_stsd_table_t *table)
{
	int i;
	table->format[0] = 'y';
	table->format[1] = 'u';
	table->format[2] = 'v';
	table->format[3] = '2';
	for(i = 0; i < 6; i++) table->reserved[i] = 0;
	table->data_reference = 1;
	table->same_but_external = 0;

	table->version = 0;
	table->revision = 0;
 	table->vendor[0] = 'o';
 	table->vendor[1] = 'q';
 	table->vendor[2] = 't';
 	table->vendor[3] = ' ';

	table->temporal_quality = 100;
	table->spatial_quality = 258;
	table->width = 0;
	table->height = 0;
	table->dpi_horizontal = 72;
	table->dpi_vertical = 72;
	table->data_size = 0;
	table->frames_per_sample = 1;
	for(i = 0; i < 32; i++) table->compressor_name[i] = 0;
	sprintf(table->compressor_name,"Jetsen QT Unknown");
	table->depth = 24;
	table->ctab_id = 65535;	// (-1 == default colour table)
	oqt_ctab_init(&(table->ctab));
	table->gamma = 0;
	table->fields = 0;
	table->field_dominance = 1;
	oqt_mjqt_init(&(table->mjqt));
	oqt_mjht_init(&(table->mjht));
	
	table->strf_len = 0;
	table->strf = NULL;

	table->channels = 0;
	table->sample_size = 0;
	table->compression_id = 0;
	table->packet_size = 0;
	table->sample_rate = 0;

	/* For the Version 1 */
	table->samplesPerPacket = 0;
	table->bytesPerPacket = 0;
	table->bytesPerFrame = 0;
	table->bytesPerSample = 0;
	
	/* Wave Extensions */
	oqt_stsdwave_init(&(table->wave));

	/* ESDS MP4 atom */
	oqt_esds_init(&(table->esds));
	
	oqt_m12v_init(&(table->m12v));
}


void oqt_stsdwave_delete(oqt_stsdwave_t *wave)
{
	if (wave->extra) {
		free(wave->extra);
		wave->extra = NULL;
		wave->extra_len = 0;
	}
}


void oqt_stsd_table_delete(oqt_stsd_table_t *table)
{
	oqt_ctab_delete(&(table->ctab));
	oqt_mjqt_delete(&(table->mjqt));
	oqt_mjht_delete(&(table->mjht));
	oqt_stsdwave_delete(&(table->wave));
	oqt_esds_delete(&(table->esds));
	oqt_m12v_delete(&(table->m12v));
	
	if (table->strf) {
		free(table->strf);
		table->strf = NULL;
		table->strf_len = 0;
	}
}

void oqt_stsd_video_dump(oqt_stsd_table_t *table)
{
	printf("       version %d\n", table->version);
	printf("       revision %d\n", table->revision);
	printf("       vendor %c%c%c%c\n", table->vendor[0], table->vendor[1], table->vendor[2], table->vendor[3]);
	printf("       temporal_quality %ld\n", (long)table->temporal_quality);
	printf("       spatial_quality %ld\n", (long)table->spatial_quality);
	printf("       width %d\n", table->width);
	printf("       height %d\n", table->height);
	printf("       dpi_horizontal %f\n", (double)table->dpi_horizontal);
	printf("       dpi_vertical %f\n", (double)table->dpi_vertical);
	printf("       data_size %lld\n", table->data_size);
	printf("       frames_per_sample %d\n", table->frames_per_sample);
	printf("       compressor_name %s\n", table->compressor_name);
	printf("       depth %d\n", table->depth);
	printf("       ctab_id %d\n", table->ctab_id);
	printf("       gamma %f\n", (double)table->gamma);
	printf("       strf_len %d\n", table->strf_len);
	if(table->fields)
	{
		printf("       fields %d\n", table->fields);
		printf("       field dominance %d\n", table->field_dominance);
	}
	if(&(table->ctab)) oqt_ctab_dump(&(table->ctab));
	oqt_mjqt_dump(&(table->mjqt));
	oqt_mjht_dump(&(table->mjht));

	oqt_esds_dump(&(table->esds));

}


void oqt_stsdwave_dump(oqt_stsdwave_t *wave)
{
	// if (private_memory) .....
	printf("       wave extension\n");

	if (wave->nChannels) {
		printf("         format %.4s\n", wave->fmtType);
		printf("         little endian: %d\n", wave->little_endian);
		printf("         wFormatTag 0x%x\n", wave->wFormatTag);
		printf("         nChannels %d\n", wave->nChannels);
		printf("         nSamplesPerSec %d\n", wave->nSamplesPerSec);
		printf("         nAvgBytesPerSec %d\n", wave->nAvgBytesPerSec);
		printf("         nBlockAlign %d\n", wave->nBlockAlign);
		printf("         wBitsPerSample %d\n", wave->wBitsPerSample);
		printf("         cbSize %d\n", wave->cbSize);
		
		// MPEG Audio Extension Extension
		if (wave->wFormatTag == OQT_STSDWAVE_MPEG) {
			printf("         MPEG Audio Extension Extenson\n");
			printf("           wID %d\n", wave->wID);
			printf("           fdwFlags %d\n", wave->fdwFlags);
			printf("           nBlockSize %d\n", wave->nBlockSize);
			printf("           nFramesPerBlock %d\n", wave->nFramesPerBlock);
			printf("           nCodecDelay %d\n", wave->nCodecDelay);
		} else if (wave->cbSize) {
			printf("         - Unknown Extension Extenson\n");
		}
	}
	
	printf("         extra data length: %d\n", wave->extra_len);
}

void oqt_stsd_audio_dump(oqt_stsd_table_t *table)
{
	printf("       version %d\n", table->version);
	printf("       revision %d\n", table->revision);
	printf("       vendor %c%c%c%c\n", table->vendor[0], table->vendor[1], table->vendor[2], table->vendor[3]);
	printf("       channels %d\n", table->channels);
	printf("       sample_size %d\n", table->sample_size);
	printf("       compression_id %d\n", table->compression_id);
	printf("       packet_size %d\n", table->packet_size);
	printf("       sample_rate %f\n", (double)table->sample_rate);
	if(table->version == 1)
	{ /* Version 1 */
		printf("       Sample Per Packet %u\n", table->samplesPerPacket);
		printf("       Bytes Per Packet %u\n", table->bytesPerPacket);
		printf("       Bytes Per Frames %u\n", table->bytesPerFrame);
		printf("       Bytes Per Sample %u\n", table->bytesPerSample);
		oqt_stsdwave_dump(&(table->wave));
	}

	oqt_esds_dump(&(table->esds));

}

void oqt_stsd_table_dump(void *minf_ptr, oqt_stsd_table_t *table)
{
	oqt_minf_t *minf = (oqt_minf_t*)minf_ptr;
	printf("       format %c%c%c%c\n", table->format[0], table->format[1], table->format[2], table->format[3]);
	oqt_print_chars("       reserved ", (char*)table->reserved, 6);
	printf("       data_reference %d\n", table->data_reference);

	if(minf->is_audio) oqt_stsd_audio_dump(table);
	if(minf->is_video) oqt_stsd_video_dump(table);
}

void oqt_write_stsd_table(oqt_t *file, oqt_minf_t *minf, oqt_stsd_table_t *table)
{
	oqt_atom_t atom;
	oqt_atom_write_header(file, &atom, table->format);
/*printf("oqt_write_stsd_table %c%c%c%c\n", table->format[0], table->format[1], table->format[2], table->format[3]); */
	oqt_write_data(file, (char*)table->reserved, 6);
	oqt_write_int16(file, (short)table->data_reference);
	
	if(minf->is_audio) oqt_write_stsd_audio(file, table);
	if(minf->is_video) oqt_write_stsd_video(file, table);

	oqt_atom_write_footer(file, &atom);
}
