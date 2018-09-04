/* moov.c
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
* $Id: moov.c,v 1.12 2003/04/07 21:02:23 shitowax Exp $
*/
#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"
#include "zlib.h"


int oqt_moov_init(oqt_moov_t *moov)
{
	int i;

	moov->total_tracks = 0;
	moov->atom.size = 0;
	moov->atom.start = 0;

	for(i = 0 ; i < OQT_MAX_TRACKS; i++) moov->trak[i] = NULL;
	oqt_mvhd_init(&(moov->mvhd));
	oqt_ctab_init(&(moov->ctab));
	oqt_iods_init(&(moov->iods));
	oqt_udta_init(moov);
	return 0;
}

int oqt_moov_delete(oqt_moov_t *moov)
{
	while(moov->total_tracks) oqt_delete_trak(moov);
	oqt_mvhd_delete(&(moov->mvhd));
	oqt_ctab_delete(&(moov->ctab));
	oqt_iods_delete(&(moov->iods));
	oqt_udta_delete(moov);
	return 0;
}

void oqt_moov_dump(oqt_moov_t *moov)
{
	int i;
	printf("movie\n");
	oqt_mvhd_dump(&(moov->mvhd));
	oqt_iods_dump(&(moov->iods));
	oqt_udta_dump(moov);
	for(i = 0; i < moov->total_tracks; i++)
		oqt_trak_dump(moov->trak[i]);
	//oqt_ctab_dump(&(moov->ctab));
}


#define QT_zlib 0x7A6C6962

int oqt_read_moov(oqt_t *file, oqt_moov_t *moov, oqt_atom_t *parent_atom)
{
	oqt_atom_t leaf_atom;

	moov->atom.size = parent_atom->size;
	moov->atom.start = parent_atom->start;

	do
	{

		oqt_atom_read_header(file, &leaf_atom);

		if(oqt_atom_is(&leaf_atom, "cmov")) 	
		{
			oqt_atom_t compressed_atom;
			__int64 cmov_sz, tlen;
			int moov_sz;

			oqt_atom_read_header(file, &compressed_atom);

			if(oqt_atom_is(&compressed_atom, "dcom"))
			{
				int zlibfourcc;
				__int64 offset;


				oqt_read_char32(file, (char *)&zlibfourcc);
				zlibfourcc = oqt_atom_read_size((char *)&zlibfourcc);

				if(zlibfourcc != QT_zlib)
					fprintf(stderr, "oqt_read_moov: Header not compressed with zlib\n");

				if(compressed_atom.size - 4 > 0)
				{
					offset = file->fileio.libqt_ftell(file->fileio.file) + compressed_atom.size - 4;
					file->fileio.libqt_fseek(file->fileio.file, offset);
				}
			}
			oqt_atom_read_header(file, &compressed_atom);

			if(oqt_atom_is(&compressed_atom, "cmvd"))
			{
				unsigned char *cmov_buf = 0;
				unsigned char *moov_buf = 0;

				z_stream zstrm;
				/* read how large uncompressed moov will be */
				oqt_read_char32(file, (char *)&moov_sz);
				moov_sz = oqt_atom_read_size((char *)&moov_sz);
				cmov_sz = compressed_atom.size - 4;

				/* Allocate buffer for compressed header */
				cmov_buf = (unsigned char *)malloc( (size_t)cmov_sz );
				if (cmov_buf == 0) 
				{
					fprintf(stderr, "QT cmov: malloc err 0");
					exit(1);
				}
				/* Read in  compressed header */

				tlen = oqt_read_data(file, (char*)cmov_buf, cmov_sz);

				if (tlen != 1)
				{ 
					fprintf(stderr,"QT cmov: read err tlen %llu\n", tlen);
					free(cmov_buf);
					return 0;
				}

				/* Allocate buffer for decompressed header */
				moov_sz += 16; /* slop?? */
				moov_buf = (unsigned char *)malloc( moov_sz );
				if (moov_buf == 0) 
				{
					fprintf(stderr,"QT cmov: malloc err moov_sz %u\n", moov_sz);
					exit(1);
				}

				zstrm.zalloc          = (alloc_func)0;
				zstrm.zfree           = (free_func)0;
				zstrm.opaque          = (voidpf)0;
				zstrm.next_in         = cmov_buf;
				zstrm.avail_in        = (unsigned int)cmov_sz;
				zstrm.next_out        = moov_buf;
				zstrm.avail_out       = moov_sz;


				int zret = inflateInit(&zstrm);
				if (zret != Z_OK) break;
				zret = inflate(&zstrm, Z_NO_FLUSH);
				if ((zret != Z_OK) && (zret != Z_STREAM_END)) break;
				moov_sz = zstrm.total_out;
				zret = inflateEnd(&zstrm);
				file->decompressed_buffer_size = moov_sz;
				file->decompressed_buffer = (char*)moov_buf;
				file->decompressed_position = 8; // Passing the first moov

				free(cmov_buf);
			} // end of "cmvd"
		} /* end of cmov */
		else if(oqt_atom_is(&leaf_atom, "mvhd"))
		{
			oqt_read_mvhd(file, &(moov->mvhd));
		}
		else if(oqt_atom_is(&leaf_atom, "iods"))
		{
			oqt_read_iods(file, &(moov->iods));
			oqt_atom_skip(file, &leaf_atom);
		}
		else if(oqt_atom_is(&leaf_atom, "clip"))
		{
			oqt_atom_skip(file, &leaf_atom);
		}
		else if(oqt_atom_is(&leaf_atom, "trak"))
		{
		  oqt_trak_t *trak = oqt_add_trak(moov);	
		  oqt_read_trak(file, trak, &leaf_atom);
		}
		else if(oqt_atom_is(&leaf_atom, "udta"))
		{
			oqt_read_udta(file, moov, &leaf_atom);
			oqt_atom_skip(file, &leaf_atom);
		}
		else if(oqt_atom_is(&leaf_atom, "ctab"))
		{
			oqt_read_ctab(file, &(moov->ctab));
			oqt_atom_skip(file, &leaf_atom);
		}
		else 
		{
#ifdef DEBUG
			fprintf(stderr, "oqt_read_moov: unknown leaf atom with 'moov' atom: %4.4s", leaf_atom.type);
#endif
			oqt_atom_skip(file, &leaf_atom);
		}


						//      printf("oqt_read_moov 0x%llx 0x%llx\n", oqt_get_position(file), parent_atom->end);

	}while((oqt_get_position(file) < parent_atom->end && file->decompressed_buffer==NULL)
		|| (oqt_get_position(file) < file->decompressed_buffer_size && file->decompressed_buffer!=NULL));

	return 0;
}

void oqt_write_moov(oqt_t *file, oqt_moov_t *moov)
{
	long longest_duration = 0;
	long duration, timescale;
	int i, result;

	result = oqt_atom_write_header(file, &(moov->atom), "moov");
	if(result)
	{
		fprintf(stderr, "oqt_write_moov: failed to start writing 'moov' atom.\n");
		return;

		// Disk full.  Rewind and try again
		//oqt_set_position(file, file->mdat.atom.end - (__int64)0x100000);
		//file->mdat.atom.end = oqt_get_position(file);
		//oqt_atom_write_header(file, &atom, "moov");
	}

	/* get the duration from the longest track in the mvhd's timescale */
	for(i = 0; i < moov->total_tracks; i++)
	{
		oqt_trak_fix_counts(file, moov->trak[i]);
		oqt_trak_duration(moov->trak[i], &duration, &timescale);

		duration = (long)((float)duration / timescale * moov->mvhd.time_scale);

		if(duration > longest_duration)
		{
			longest_duration = duration;
		}
	}
	moov->mvhd.duration = longest_duration;

	// This just pisses me off :)
	//moov->mvhd.selection_duration = longest_duration;
	moov->mvhd.selection_duration = 0;

	oqt_write_mvhd(file, &(moov->mvhd));
	if(file->use_mp4)
		oqt_write_iods(file, &(moov->iods));

	oqt_write_udta(file, moov);
	for(i = 0; i < moov->total_tracks; i++)
	{
		oqt_write_trak(file, moov->trak[i], moov->mvhd.time_scale);
	}
	/*oqt_write_ctab(file, &(moov->ctab)); */

	oqt_atom_write_footer(file, &(moov->atom));
}


int oqt_shift_offsets(oqt_moov_t *moov, __int64 offset)
{
	int i;
	for(i = 0; i < moov->total_tracks; i++)
	{
		oqt_trak_shift_offsets(moov->trak[i], offset);
	}
	return 0;
}
