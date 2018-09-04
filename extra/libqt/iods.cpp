/* iods.c
 * Copyright (C) 2001-2003 OpenQuicktime Team
 *
 * This file is part of OpenQuicktime, a free QuickTime library.
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
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


int oqt_iods_init(oqt_iods_t *iods)
{
	iods->version = 0;
	iods->flags = 0;
	iods->audioProfileId = 0xFF;
	iods->videoProfileId = 0xFF;
	return 0;
}

int oqt_iods_set_audio_profile(oqt_iods_t* iods, int id)
{
	iods->audioProfileId = id;
	return 0;
}

int oqt_iods_set_video_profile(oqt_iods_t* iods, int id)
{
	iods->videoProfileId = id;
	return 0;
}

int oqt_iods_delete(oqt_iods_t *iods)
{
	return 0;
}

int oqt_iods_dump(oqt_iods_t *iods)
{
	printf(" initial object descriptor\n");
	printf("  version %d\n", iods->version);
	printf("  flags %ld\n", (long)iods->flags);
	printf("  audioProfileId %u\n", iods->audioProfileId);
	printf("  videoProfileId %u\n", iods->videoProfileId);
	return 0;
}

int oqt_read_iods(oqt_t *file, oqt_iods_t *iods)
{
	iods->version = oqt_read_char(file);
	iods->flags = oqt_read_int24(file);
	oqt_read_char(file); /* skip tag */
	oqt_read_mp4_descr_length(file);	/* skip length */
	/* skip ODID, ODProfile, sceneProfile */
	oqt_set_position(file, oqt_get_position(file) + 4);
	iods->audioProfileId = oqt_read_char(file);
	iods->videoProfileId = oqt_read_char(file);
	/* will skip the remainder of the atom */
	return 0;
}

int oqt_write_iods(oqt_t *file, oqt_iods_t *iods)
{
	oqt_atom_t atom;
	int i;

	if (!file->use_mp4) {
		return 0;
	}

	oqt_atom_write_header(file, &atom, "iods");

	oqt_write_char(file, (char)iods->version);
	oqt_write_int24(file, iods->flags);

	oqt_write_char(file, 0x10);	/* MP4_IOD_Tag */
	oqt_write_char(file, 7 + (file->moov.total_tracks * (1+1+4)));	/* length */
	oqt_write_int16(file, 0x004F); /* ObjectDescriptorID = 1 */
	oqt_write_char(file, (char)0xFF);	/* ODProfileLevel */
	oqt_write_char(file, (char)0xFF);	/* sceneProfileLevel */
	oqt_write_char(file, (char)iods->audioProfileId);	/* audioProfileLevel */
	oqt_write_char(file, (char)iods->videoProfileId);	/* videoProfileLevel */
	oqt_write_char(file, (char)0xFF);	/* graphicsProfileLevel */

	for (i = 0; i < file->moov.total_tracks; i++) {
		oqt_write_char(file, 0x0E);	/* ES_ID_IncTag */
		oqt_write_char(file, 0x04);	/* length */
		oqt_write_int32(file, file->moov.trak[i]->tkhd.track_id);	
	}

	/* no OCI_Descriptors */
	/* no IPMP_DescriptorPointers */
	/* no Extenstion_Descriptors */

	oqt_atom_write_footer(file, &atom);
	return 0;
}

