/* esds.c
 * Copyright (C) 2002-2003 OpenQuicktime Team
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
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "libqt.h"


int oqt_esds_init(oqt_esds_t *esds)
{
/*
	esds->version = 0;
	esds->flags = 0;
	esds->decoderConfigLen = 0;
	esds->decoderConfig = NULL;
*/
	memset(esds, 0, sizeof(oqt_esds_t));
	return 0;
}

int oqt_esds_get_decoder_config(oqt_esds_t* esds, BYTE** ppBuf, int* pBufSize)
{
	if (esds->decoderConfig == NULL || esds->decoderConfigLen == 0) {
		*ppBuf = NULL;
		*pBufSize = 0;
	} else {
		*ppBuf = (BYTE*)malloc(esds->decoderConfigLen);
		if (*ppBuf == NULL) {
			*pBufSize = 0;
			return 1;
		}
		memcpy(*ppBuf, esds->decoderConfig, esds->decoderConfigLen);
		*pBufSize = esds->decoderConfigLen;
	}
	return 0;
}

int oqt_esds_set_decoder_config(oqt_esds_t* esds, BYTE* pBuf, int bufSize)
{
	free(esds->decoderConfig);
	esds->decoderConfig = (BYTE*)malloc(bufSize);
	if (esds->decoderConfig) {
		memcpy(esds->decoderConfig, pBuf, bufSize);
		esds->decoderConfigLen = bufSize;
		return 0;
	}
	return 1;
}

int oqt_esds_delete(oqt_esds_t *esds)
{
	free(esds->decoderConfig);
	return 0;
}

int oqt_esds_dump(oqt_esds_t *esds)
{
	int i;

	printf("       elementary stream descriptor\n");
	printf("        version %d\n", esds->version);
	printf("        flags %d\n", esds->flags);
	printf("        decoder config ");
	for (i = 0; i < esds->decoderConfigLen; i++) {	
		printf("%02x ", esds->decoderConfig[i]);
	}
	printf("\n");
	
	return 0;
}

int oqt_read_esds(oqt_t *file, oqt_esds_t *esds)
{
	BYTE tag;
//	ULONG length;

	esds->version = oqt_read_char(file);
	esds->flags = oqt_read_int24(file);

	/* get and verify ES_DescrTag */
	tag = oqt_read_char(file);
	if (tag == 0x03) {
		/* read length */
		if (oqt_read_mp4_descr_length(file) < 5 + 15) {
			return 1;
		}
		/* skip 3 bytes */
		oqt_set_position(file, oqt_get_position(file) + 3);
	} else {
		/* skip 2 bytes */
		oqt_set_position(file, oqt_get_position(file) + 2);
	}

	/* get and verify DecoderConfigDescrTab */
	if (oqt_read_char(file) != 0x04) {
		return 1;
	}

	/* read length */
	if (oqt_read_mp4_descr_length(file) < 15) {
		return 1;
	}

	/* skip 13 bytes */
//	oqt_set_position(file, oqt_get_position(file) + 13);
	esds->objectTypeId = oqt_read_char(file);
	esds->streamType = ((int)oqt_read_char(file)) >> 2;
	esds->bufferSizeDB = oqt_read_int24(file);
	esds->maxBitrate = oqt_read_uint32(file);
	esds->avgBitrate = oqt_read_uint32(file);

	/* get and verify DecSpecificInfoTag */
	if (oqt_read_char(file) != 0x05) {
		return 1;
	}

	/* read length */
	esds->decoderConfigLen = oqt_read_mp4_descr_length(file); 

	free(esds->decoderConfig);
	esds->decoderConfig = (BYTE*)malloc(esds->decoderConfigLen);
	if (esds->decoderConfig) {
		oqt_read_data(file, (char*)esds->decoderConfig, esds->decoderConfigLen);
	} else {
		esds->decoderConfigLen = 0;
	}

	/* let's set our mp4 flag */
	file->use_mp4 = 1;

	/* will skip the remainder of the atom */
	return 0;
}

int oqt_write_esds_common(oqt_t *file, oqt_esds_t *esds, int esid, ULONG objectType, ULONG streamType)
{
	oqt_atom_t atom;

	oqt_atom_write_header(file, &atom, "esds");

	oqt_write_char(file, esds->version);
	oqt_write_int24(file, esds->flags);

	oqt_write_char(file, 0x03);	/* ES_DescrTag */
	oqt_write_mp4_descr_length(file, 
//		3 + (5 + (13 + (5 + esds->decoderConfigLen))) + 3, 1);
		3 + (2 + (13 + (2 + esds->decoderConfigLen))) + 3, 1);

	oqt_write_int16(file, esid);
	oqt_write_char(file, 0x10);	/* streamPriorty = 16 (0-31) */

	/* DecoderConfigDescriptor */
	oqt_write_char(file, 0x04);	/* DecoderConfigDescrTag */
	oqt_write_mp4_descr_length(file, 
//		13 + (5 + esds->decoderConfigLen), 1);
		13 + (2 + esds->decoderConfigLen), 1);

	oqt_write_char(file, (char)objectType); /* objectTypeIndication */
	oqt_write_char(file, (char)((streamType)<<2)|0x01); /* streamType */

	oqt_write_int24(file, esds->bufferSizeDB);		/* buffer size */
	oqt_write_int32(file, esds->maxBitrate);		/* max bitrate */
	oqt_write_int32(file, esds->avgBitrate);		/* average bitrate */

	oqt_write_char(file, 0x05);	/* DecSpecificInfoTag */

	oqt_write_mp4_descr_length(file, esds->decoderConfigLen, 1);
	oqt_write_data(file, (char*)esds->decoderConfig, esds->decoderConfigLen);

	/* SLConfigDescriptor */
	oqt_write_char(file, 0x06);	/* SLConfigDescrTag */
	oqt_write_char(file, 0x01);	/* length */
	oqt_write_char(file, 0x02);	/* constant in mp4 files */

	/* no IPI_DescrPointer */
	/* no IP_IdentificationDataSet */
	/* no IPMP_DescriptorPointer */
	/* no LanguageDescriptor */
	/* no QoS_Descriptor */
	/* no RegistrationDescriptor */
	/* no ExtensionDescriptor */

	oqt_atom_write_footer(file, &atom);
	
	return 0;
}

int oqt_write_esds_audio(oqt_t *file, oqt_esds_t *esds, int esid)
{
	return oqt_write_esds_common(file, esds, esid, (ULONG)0x40, (ULONG)0x05);
}

int oqt_write_esds_video(oqt_t *file, oqt_esds_t *esds, int esid)
{
	return oqt_write_esds_common(file, esds, esid, (ULONG)0x20, (ULONG)0x04);
}

