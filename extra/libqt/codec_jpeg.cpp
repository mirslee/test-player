#include "stdafx.h"
#include "qtcodec.h"
#include "setjmp.h"
#include "libjpeg\jpeglib.h"
#pragma comment(lib,".\\libqt\\jpegvc8.lib")
#define JPEG_DEFAULT_QUALITY 85

typedef struct {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
} my_error_mgr;
/* Each instance of the codec should have its own Param structure allocated */
typedef struct {
	/* Error structure */
	my_error_mgr my_jerr;

	/* YUV Working buffer */
	BYTE **yuv_line[3];

	/* quality */
	int quality;

	/* decompression structure */
	struct jpeg_decompress_struct decinfo;
	struct jpeg_source_mgr jsrc;

	/* compression structure */
	struct jpeg_compress_struct encinfo;
	struct jpeg_destination_mgr jdest;

} Param;  


static void jpeg_oqt_error_exit(j_common_ptr cinfo)
{
	my_error_mgr * jerr = (my_error_mgr *) cinfo->err;
	fprintf(stderr, "jpeg_oqt_error_exit: got error.\n");

	/* Display the error message */
	(*cinfo->err->output_message) (cinfo);

	/* Abort the compression/decompression so we can resuse */
	jpeg_abort(cinfo);

	/* Go to jump point near start of encode/decode */	
	longjmp(jerr->setjmp_buffer, 1);
}


/* Verify that the format of the frame is correct */
static inline int verify_jpeg_format(ImageDescriptionPtr desc,Param *p )
{

	/* Check width */
	if (p->decinfo.output_width != desc->width) {
		fprintf(stderr, "verify_jpeg_format: error, width of jpeg frame is not same as track.\n");
		jpeg_abort_decompress(&p->decinfo);
		return 1;
	}

	/* Check height */
	if (p->decinfo.output_height != desc->height) {
		fprintf(stderr, "verify_jpeg_format: error, height of jpeg frame is not same as track.\n");
		jpeg_abort_decompress(&p->decinfo);
		return 1;
	}

	/* Check the comonents */
	if (p->decinfo.output_components != 3) {
		fprintf(stderr, "verify_jpeg_format: error, number of colour components is not 3.\n");
		jpeg_abort_decompress(&p->decinfo);
		return 1;
	}

	/* Check component size */
	if (BITS_IN_JSAMPLE!=8) {
		fprintf(stderr, "verify_jpeg_format: error, bits per JSample is not 8.\n");
		jpeg_abort_decompress(&p->decinfo);
		return 1;
	}


	/* Success */
	return 0;
}

static int allocate_jpeg_yuv(ImageDescriptionPtr desc, Param *p )
{
	int height = desc->height;

	/* Check not allocated */
	if (p->yuv_line[0] || p->yuv_line[1] || p->yuv_line[2]) {
		fprintf(stderr, "allocate_jpeg_yuv: buffers already allocated.\n");
		return 0;
	}

	/* Allocate memory for lines of YUV */
	p->yuv_line[0] = (BYTE**)malloc(height*sizeof(BYTE*));
	p->yuv_line[1] = (BYTE**)malloc(height*sizeof(BYTE*)/2);
	p->yuv_line[2] = (BYTE**)malloc(height*sizeof(BYTE*)/2);

	if(!p->yuv_line[0] || !p->yuv_line[1] || !p->yuv_line[2]) {
		fprintf(stderr, "allocate_jpeg_yuv: failed to allocate memory for YUV buffers.\n");
		return 1;
	}

	/* Success */
	return 0;
}


/*************************************************************/
/* Decoding function                                         */
/*************************************************************/
int decode_JPEG(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch)
{
	Param *p = (Param*)codec;

	/* Setup return point for errors */
	if(setjmp(p->my_jerr.setjmp_buffer)) {
		// exception: we longjmp'ed here
		fprintf(stderr, "decode_JPEG: caught error - failed to decode JPEG stream\n");
		return 1;
	}
	p->my_jerr.pub.error_exit = jpeg_oqt_error_exit;


	/* Read in the JPEG headers */
	p->jsrc.next_input_byte = input;
	p->jsrc.bytes_in_buffer = inputsize;
	jpeg_read_header(&p->decinfo, TRUE);


	/* Configure Decoder */
	p->decinfo.do_fancy_upsampling	= FALSE;
	p->decinfo.do_block_smoothing	= FALSE;
	p->decinfo.dct_method       	= JDCT_IFAST;
	p->decinfo.dither_mode			= JDITHER_ORDERED;


	
	int width, height;
	
	int width2, i, j, k, r_v;
	BYTE *base[3] = {};
	base[0] = output;
	base[1] = output+pitch*desc->height;
	base[2] = output+pitch*desc->height+pitch*desc->height/4;

	/* get read for raw data output */
	r_v = p->decinfo.cur_comp_info[0]->v_samp_factor;
	p->decinfo.raw_data_out = TRUE;
	p->decinfo.out_color_space = JCS_YCbCr;

	/* Start decompressing JPEG */
	jpeg_start_decompress(&p->decinfo);
	width = p->decinfo.output_width;
	height = p->decinfo.output_height;
	width2 = width >> 1;

	/* Check format is correct */
	if (verify_jpeg_format(desc, p)) return -1;
	/* Make sure YUV buffers have been allocated */
	if (!p->yuv_line[0] && allocate_jpeg_yuv(desc, p)) return -1;

	/* Copy raw data into output buffer */
	for (i = 0; i < height; i += r_v*DCTSIZE) 
	{
		for (j=0, k=0; j< (r_v*DCTSIZE); j += r_v, k++) 
		{
			p->yuv_line[0][j]   = base[0]; base[0] += width;
			if (r_v == 2) 
			{
				p->yuv_line[0][j+1] = base[0]; base[0] += width;
			}
			p->yuv_line[1][k]   = base[1]; 
			p->yuv_line[2][k]   = base[2];
			if (r_v == 2 || k&1) 
			{
				base[1] += width2; base[2] += width2;
			}
		}
		jpeg_read_raw_data(&p->decinfo, p->yuv_line, r_v*DCTSIZE);
	}
	jpeg_finish_decompress(&p->decinfo);
	return 0;
}

/* Dummy decoding functions */
static void jpegdec_init_source (j_decompress_ptr cinfo){}
static boolean jpegdec_fill_input_buffer (j_decompress_ptr cinfo){
	/* There is no more data than we placed in
	the buffer in the first place */
	fprintf(stderr, "jpegdec_fill_input_buffer: error this should not be called.\n");
	return 0;	
}
static void jpegdec_skip_input_data (j_decompress_ptr cinfo, long num_bytes){
	if (num_bytes <=0) return;
	cinfo->src->next_input_byte += num_bytes;
	cinfo->src->bytes_in_buffer -= num_bytes;
	if (cinfo->src->bytes_in_buffer<0)
		fprintf(stderr, "jpegdec_skip_input_data: error, ran out of data.\n");
}
static void jpegdec_term_source (j_decompress_ptr cinfo){}

/* Dummy encoding functions */
static void jpegenc_init_destination (j_compress_ptr cinfo){}
static boolean jpegenc_flush_destination (j_compress_ptr cinfo){return 1;}
static void jpegenc_term_destination (j_compress_ptr cinfo){}


void* init_JPEG(ImageDescriptionPtr desc,int* colormodel)
{
	/* Here we go, now we support as many initializatin as you want ;)  */
	Param *p =  (Param*)malloc(sizeof(Param));
	memset(p, 0, sizeof(Param));
	p->decinfo.err = p->encinfo.err = jpeg_std_error(&p->my_jerr.pub);

	/* Set YUV buffers pointers to null */
	p->yuv_line[0] = NULL;
	p->yuv_line[1] = NULL;
	p->yuv_line[2] = NULL;

	/* Decoder init */
	jpeg_create_decompress(&p->decinfo);
	p->jsrc.init_source       = jpegdec_init_source;
	p->jsrc.fill_input_buffer = jpegdec_fill_input_buffer;
	p->jsrc.skip_input_data   = jpegdec_skip_input_data;
	p->jsrc.term_source       = jpegdec_term_source;
	p->decinfo.src = &p->jsrc;


	/* Encoder init */
	jpeg_create_compress(&p->encinfo);
	p->quality = JPEG_DEFAULT_QUALITY;

	p->jdest.init_destination    = jpegenc_init_destination;
	p->jdest.empty_output_buffer = jpegenc_flush_destination;
	p->jdest.term_destination    = jpegenc_term_destination;
	p->encinfo.dest              = &p->jdest;

	return p;
}


void delete_JPEG(void* codec)
{
	Param *p = (Param*)codec;
	if(p) 
	{
		if (p->yuv_line[0]) { free(p->yuv_line[0]); p->yuv_line[0] = NULL; }
		if (p->yuv_line[1]) { free(p->yuv_line[1]); p->yuv_line[1] = NULL; }
		if (p->yuv_line[2]) { free(p->yuv_line[2]); p->yuv_line[2] = NULL; }

		// Free memory allocated by libjpeg
		jpeg_destroy_decompress(&p->decinfo);
		jpeg_destroy_compress(&p->encinfo);

		free(p);
	}
}

