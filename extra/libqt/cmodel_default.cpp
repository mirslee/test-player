/* cmodel_default.c
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
 * $Id: cmodel_default.c,v 1.10 2003/04/07 21:02:18 shitowax Exp $
 */

#include "stdafx.h"
#include "qtfuncs.h"
#include "qttype.h"
#include "cmodel_permutation.h"


#define TRANSFER_FRAME_DEFAULT(output, \
	input, \
	y_in_offset, \
	u_in_offset, \
	v_in_offset, \
	input_column) \
{ \
	register int i, j; \
 \
	switch(in_colormodel) \
	{ \
		case BC_YUV888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_RGB8((output), (input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_RGBA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_YUV101010((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_YUV888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_YUV888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_YUV888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_YUV422((output), \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_YUV888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_YUVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_VYU888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_VYU888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_UYVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV888_to_UYVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_YUVA8888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_RGB8((output), (input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_RGBA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_VYU888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_VYU888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_YUVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_UYVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_UYVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_YUV101010((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_YUVA8888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_YUVA8888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_YUVA8888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_YUV422((output), \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_YUV161616: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_RGB8((output), (WORD*)(input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_BGR565((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_RGB565((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_BGR888((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_BGR8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_RGB888((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_RGBA8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_YUV161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_YUV161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_YUV161616_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_YUV422((output), \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_YUV101010((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_YUVA8888((output), \
						(WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_VYU888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_VYU888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_UYVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_UYVA8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV161616: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV161616_to_YUV161616((WORD**)(output), \
						(WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_YUVA16161616: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_RGB8((output), (WORD*)(input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_BGR565((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_RGB565((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_BGR888((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_BGR8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_RGB888((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_RGBA8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_YUV101010((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_VYU888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_VYU888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_UYVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_UYVA8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_YUVA16161616((WORD**)(output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_YUVA16161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_YUVA16161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_YUVA16161616_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA16161616_to_YUV422((output), \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_YUV101010: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_RGB8((output), (input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_RGB888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_RGBA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_YUV888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_YUVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_RGB161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_RGBA16161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV161616: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_YUV161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV101010_to_YUVA16161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_VYU888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_RGB8((output), (input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_RGB888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_RGBA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_YUV888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_YUVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_RGB161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_RGBA16161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV161616: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_YUV161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_VYU888_to_YUVA16161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_UYVA8888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_RGB8((output), (input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_RGB888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_RGBA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_YUV888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_YUVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_RGB161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_RGBA16161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV161616: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_YUV161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_UYVA8888_to_YUVA16161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
\
		case BC_ABGR8888: \
			switch(out_colormodel) \
			{ \
				case BC_ABGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_ARGB8888_to_ARGB8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
		case BC_ARGB8888: \
			switch(out_colormodel) \
			{ \
				case BC_ARGB8888: \
					TRANSFER_FRAME_HEAD \
					transfer_ARGB8888_to_ARGB8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_ARGB8888_to_RGBA8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
\
		case BC_RGB565: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB565_to_RGB888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB565_to_BGR888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB565_to_RGBA8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_ARGB8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB565_to_ARGB8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_ABGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB565_to_ABGR8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
\
		case BC_RGB888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGB8((output), (input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGBA8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGB161616((WORD**)(output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGBA16161616((WORD**)(output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_ARGB8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_ARGB8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_ABGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_ABGR8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUV888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUV161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUVA16161616((WORD**)(output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUV101010((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGB888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUV422((output), (input), j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGB888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGB888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_RGBA8888: \
			switch(out_colormodel) \
			{ \
				case BC_TRANSPARENCY: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_TRANSPARENCY((output), (input), &bit_counter); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB8: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB8bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB8((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR565bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR565((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB565bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB565((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR888bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR888((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB888bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB888((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGBA8888((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_RGB161616((WORD**)(output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_RGBA16161616((WORD**)(output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR8888bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR8888((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_YUV888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_YUVA8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_YUV161616((WORD**)(output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA16161616: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_YUVA16161616((WORD**)(output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_YUV101010((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGBA888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA888_to_YUV422((output), (input), j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGBA888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGBA888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_RGB161616: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_RGB8((output), (WORD*)(input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_BGR565((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_RGB565((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_BGR888((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_BGR8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_RGB888((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_RGBA8888((output), (WORD*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_YUV888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_YUVA8888((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_YUV101010((output), (WORD*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGB161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGB161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGB161616_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_RGBA16161616: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGB8((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_BGR565((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGB565((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_BGR888((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
				break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_BGR8888((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGB888((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGBA8888((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV101010: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_YUV101010((output), (WORD*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGBA16161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGBA16161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGBA16161616_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(WORD*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_BGR8888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR8888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR8888_to_BGR8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
 \
		case BC_BGR888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				default: TRANSFER_FAIL break; \
			} \
			break; \
		default: TRANSFER_FAIL break; \
	} \
}




void oqt_cmodel_transfer_default(PERMUTATION_ARGS)
{
/* 
	this lovely function hard-codes all possible transforms and scales into ONE function... great.
	
	this little bit of insanity pre-processes to about 128KB of code (ie characters of source)
	
	which neatly kills my compiler... well, not quite, it just spits the dummy.
*/

#ifdef _WIN32
	if(scale)
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			input_row + column_table[j] * in_pixelsize,
			0,
			0,
			0,
			0);
	}
	else
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			input_row + j * in_pixelsize,
			0,
			0,
			0,
			0);
	}
#endif /* __MACOS__ */
}
