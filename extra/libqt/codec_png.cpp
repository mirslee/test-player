#include "stdafx.h"
#include "qtcodec.h"

#include ".\png\png.h"
#pragma comment(lib,".\\libqt\\pngvc8.lib")
#pragma comment(lib,".\\libqt\\zlibvc8.lib")
/* Each instance of the codec should have its own Param structure allocated */
typedef struct {
	int compression_level;

	ULONG buffer_size;
	ULONG buffer_position;
	ULONG buffer_allocated;
	BYTE  *buffer;

} Param;  



/*************************************************************/
/* Callbacks                                                 */
/*************************************************************/

static void read_function(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	Param *p = (Param*)png_get_io_ptr(png_ptr);
	if(length + p->buffer_position <= p->buffer_size)
	{
		memcpy(data, p->buffer + p->buffer_position, length);
		p->buffer_position += length;
	} else {
		fprintf(stderr, "codec_png: ran out of buffer space in read_function.\n");
	}
}

static void reset_buffer_png(Param *p) 
{
	p->buffer_size = 0;
	p->buffer_position = 0;
	p->buffer_allocated = 0;
	p->buffer = NULL;
}


/*************************************************************/
/* Decoding function                                         */
/*************************************************************/
int decode_png(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch)
{
	Param *p = (Param*)codec;
	int width = desc->width;
	int height = desc->height;
	int depth = desc->depth;
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info = 0;	
	int color_type;

	reset_buffer_png( p );
	p->buffer_size = inputsize;
	p->buffer = input;

	/* Prepare libpng and read info */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);
	png_set_read_fn(png_ptr, p, (png_rw_ptr)read_function);
	png_read_info(png_ptr, info_ptr);

	/* Check that png is the right format */
	int ret = -1;
	color_type = png_get_color_type(png_ptr, info_ptr);
	if(((depth==24&&color_type==PNG_COLOR_TYPE_RGB)||(depth==32&&color_type==PNG_COLOR_TYPE_RGB_ALPHA))&&(height==png_get_image_height(png_ptr,info_ptr)))
	{
		PBYTE* out = new PBYTE[height];
		for(int i=0;i<height;i++)
			out[i] = output+pitch*i;
		png_read_image(png_ptr,out);
		ret = 0;
	}
	/* Clean up */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	reset_buffer_png( p );

	return ret;
}

void* init_png(ImageDescriptionPtr desc,int* colormodel)
{
	void *p = malloc(sizeof(Param));
	memset(p, 0, sizeof(Param));
	((Param*)p)->compression_level = 6;
	*colormodel = BC_BGRA8888;
	reset_buffer_png((Param*)p );
	return p;
}


void delete_png(void* codec)
{
	Param *p = (Param*)codec;
	if(p) free(p);
}

