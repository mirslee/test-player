/* colormodels.h
 * Copyright (C) 2001-2003 QT4Linux and OpenQuicktime Teams
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
 * $ID:$
 */

/*!
	@header OpenQuicktime
	OpenQuicktime aims to be a portable library for handling Apple's QuickTime (TM)
	popular media files on Unix-like environments.
	
	This header file contains colour model constants and
	interface to color-model conversion functions in the library.
*/

#pragma once

/* Colormodels */
#define BC_TRANSPARENCY 0
#define BC_COMPRESSED   1
#define BC_RGB8         2
#define BC_RGB565       3
#define BC_BGR565       4
#define BC_BGR888       5
#define BC_BGR8888      6

/* Working bitmaps are packed to simplify processing */
#define BC_RGB888       9
#define BC_RGBA8888     10
#define BC_ARGB8888     20
#define BC_ABGR8888     21
#define BC_RGB161616    11
#define BC_RGBA16161616 12
#define BC_YUV888       13
#define BC_YUVA8888     14
#define BC_YUV161616    15
#define BC_YUVA16161616 16
#define BC_YUV422       19
#define BC_A8           22
#define BC_A16          23
#define BC_YUV101010    24
#define BC_VYU888       25
#define BC_UYVA8888     26

/* Planar */
#define BC_YUV420P      7
#define BC_YUV422P      17
#define BC_YUV411P      18
#define BC_YUV444P      27

/* Colour Mapped */
#define BC_M8			30		/* 8-bit colour mapped */
#define BC_M4			31		/* 4-bit colour mapped */
#define BC_M2			32		/* 2-bit colour mapped */
#define BC_M1			33		/* 1-bit colour mapped */

/* Packed */
#define BC_YUV420PACKED 50

/* Used for frame dropping */
#define BC_NONE         -1
#define BC_UNSUPPORTED	-2

/* unimplemented, for q4l compatibility */
#define BC_BGRA8888		BC_BGR8888      
/*#define BC_A8           100
#define BC_A16          101
#define BC_YUV101010    102
#define BC_VYU888       103
#define BC_UYVA8888     104
*/


/*!
  @function			oqt_get_cmodel_name
  @abstract			Returns the name of a colour model number
  @param colormodel	Colour model to get the name of
  @result			A string name for the colour model.
*/
const char* oqt_get_cmodel_name(int colormodel);

/*!
  @function			oqt_cmodel_is_rgb
  @abstract			Returns true if colour model is RGB
  @param colormodel	Colour model to check
  @result			True (1) if colour model is rgb
  					False (0) if colour model is non-rgb
*/
int oqt_cmodel_is_rgb(int colormodel);

/*!
  @function			oqt_cmodel_is_mapped
  @abstract			Returns true if colour model is colour mapped
  @discussion		Colour model specified indexes in a colour table (ctab)
  					to get the acutual 32-bit RGB colour value to be used.
  @param colormodel	Colour model to check
  @result			True (1) if colour model is mapped
  					False (0) if colour model is not mapped
*/
int oqt_cmodel_is_mapped(int colormodel);

/*!
  @function			oqt_cmodel_is_planar
  @abstract			Returns true if colour model is Planar
  @param colormodel	Colour model to check
  @result			True (1) if colour model is planar
  					False (0) if colour model is non-planar
*/
int oqt_cmodel_is_planar(int colormodel);

/*!
  @function			oqt_cmodel_is_yuv
  @abstract			Returns true if colour model is YUV
  @param colormodel	Colour model to check
  @result			True (1) if colour model is YUV.
  					False (0) if colour model is not YUV.
*/
int oqt_cmodel_is_yuv(int colormodel);

/*!
  @function			oqt_cmodel_components
  @abstract			Returns the number of components in a colour model (ie 3 for RGB)
  @param colormodel	Colour model to check
  @result			The number of components in the colour model.
  					Or less than zero if the number of components is unknown
*/
int oqt_cmodel_components(int colormodel);


/*!
  @function			oqt_cmodel_calculate_pixelsize
  @abstract			Returns the length, in bytes, of a single pixel
  @param colormodel	Colour model to check
  @result			The length in bytes of one pixel encoded using colour model
  					If a pixel is less than one byte then 0 is returned.
  					If the colour model is unknown then a value less than zero is returned.
*/
int oqt_cmodel_calculate_pixelsize(int colormodel);

/*!
  @function			oqt_cmodel_calculate_max
  @abstract			Returns the maximum single component value for a colour model
  @param colormodel	Colour model to check
  @result			The maximum single component value for the colour model.
  					If the colour model is unknown then a value less than zero is returned.
*/
int oqt_cmodel_calculate_max(int colormodel);

/*!
  @function			oqt_cmodel_calculate_framesize
  @abstract			Get the length in bytes of a specified frame
  @param width			The width of the frame
  @param height			The height of the frame
  @param colormodel		The colour model to check.
  						Optional - if bytes_per_line is provided, pass -1.
  @param bytes_per_line	The number of bytes per line.
  						Optional - if colormodel is provided, pass -1.
  @result			The length in bytes of the frame.
  					A value a zero or less may be return if calculation is not possible.
*/
int oqt_cmodel_calculate_framesize(
	int width,
	int height,
	int colormodel,
	int bytes_per_line);
