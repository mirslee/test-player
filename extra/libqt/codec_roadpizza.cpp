#include "stdafx.h"
#include "qtcodec.h"

void qt_decode_rpza(char *encoded, int encoded_size, char *decoded, int width,int height, int bytes_per_pixel);

typedef struct {
	char *buffer;
} Param;  

void* init_roadpizza(ImageDescriptionPtr desc,int* colormodel)
{
	void *p = malloc(sizeof(Param));
	memset(p, 0, sizeof(Param));
	((Param*)p)->buffer = (char*)malloc(desc->width * desc->height * 4);
	*colormodel = BC_BGRA8888;
	return p;
}



void delete_roadpizza(void* codec)
{
	Param *p = (Param*)codec;
	if(p) 
	{
		if (p->buffer)	free(p->buffer);
		free(p);
	}
}



/*************************************************************/
/* Decoding function                                         */
/*************************************************************/
int decode_roadpizza(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch)
{
	Param *p = (Param*)codec;
	int width 	= desc->width;
	int height 	= desc->height;
	qt_decode_rpza((char*)input, inputsize,p->buffer, width, height, 4);
	for(int i = 0; i < height; i++)
		memcpy(output+i*pitch, &(p->buffer[i*width*4]), width*4);
	return 0;
}



// This may cause alignment problem on some architecture!
//#define BE_16(x) (be2me_16(*(unsigned short *)(x)))
//#define BE_32(x) (be2me_32(*(unsigned int *)(x)))

#define BE_16(x) (((WORD)*((BYTE*)x)<<8) + (WORD)*((BYTE*)x+1))
#define BE_32(x) (((ULONG)*((BYTE*)x)<<24) + ((ULONG)*((BYTE*)x+1)<<16) + ((ULONG)*((BYTE*)x+2)<<8)+(ULONG)*((BYTE*)x+3))


#define ADVANCE_BLOCK() \
{ \
	pixel_ptr += block_x_inc; \
	if (pixel_ptr >= (width * bytes_per_pixel)) \
{ \
	pixel_ptr = 0; \
	row_ptr += block_y_inc * 4; \
} \
	total_blocks--; \
	if (total_blocks < 0) \
{ \
	fprintf(stderr, "roadpizza: block counter just went negative (this should not happen)\n"); \
	return; \
} \
}

#define PAINT_CURRENT_PIXEL(r, g, b, color) \
{ \
	if (bytes_per_pixel == 2) { \
	(*(unsigned short*)(&decoded[block_ptr])) = color & 0x7fff; \
	block_ptr += 2; \
	} else { \
	decoded[block_ptr++] = (b); \
	decoded[block_ptr++] = (g); \
	decoded[block_ptr++] = (r); \
	if (bytes_per_pixel == 4) /* 32bpp */ \
		decoded[block_ptr++] = (BYTE)0xFF;\
	} \
}

#define COLOR_FIX(col_out, col_in) (col_out) = ((col_in) << 3) | ((col_in) >> 2)

#define COLOR_TO_RGB(r, g, b, color) \
{ \
	if (bytes_per_pixel != 2) { \
	unsigned short tmp; \
	tmp = (color >> 10) & 0x1f; \
	COLOR_FIX (r, tmp); \
	tmp = (color >> 5) & 0x1f; \
	COLOR_FIX (g, tmp); \
	tmp = color & 0x1f; \
	COLOR_FIX (b, tmp); \
	} \
}

#define COLORAB_TO_RGB4(rgb4, color4, colorA, colorB) \
{ \
	unsigned short ta, tb, tt; \
	if (bytes_per_pixel != 2) { \
	ta = (colorA >> 10) & 0x1f; \
	tb = (colorB >> 10) & 0x1f; \
	COLOR_FIX (rgb4[3][0], ta); \
	COLOR_FIX (rgb4[0][0], tb); \
	tt = (11 * ta + 21 * tb) >> 5; \
	COLOR_FIX (rgb4[1][0], tt); \
	tt = (21 * ta + 11 * tb) >> 5; \
	COLOR_FIX (rgb4[2][0], tt); \
	ta = (colorA >> 5) & 0x1f; \
	tb = (colorB >> 5) & 0x1f; \
	COLOR_FIX (rgb4[3][1], ta); \
	COLOR_FIX (rgb4[0][1], tb); \
	tt = (11 * ta + 21 * tb) >> 5; \
	COLOR_FIX (rgb4[1][1], tt); \
	tt = (21 * ta + 11 * tb) >> 5; \
	COLOR_FIX (rgb4[2][1], tt); \
	ta = colorA	& 0x1f; \
	tb = colorB	& 0x1f; \
	COLOR_FIX (rgb4[3][2], ta); \
	COLOR_FIX (rgb4[0][2], tb); \
	tt = (11 * ta + 21 * tb) >> 5; \
	COLOR_FIX (rgb4[1][2], tt); \
	tt = (21 * ta + 11 * tb) >> 5; \
	COLOR_FIX (rgb4[2][2], tt); \
	} else { \
	color4[3] = colorA; \
	color4[0] = colorB; \
	ta = (colorA >> 10) & 0x1f; \
	tb = (colorB >> 10) & 0x1f; \
	color4[1] = ((11 * ta + 21 * tb) << 5) & 0x7c00; \
	color4[2] = ((21 * ta + 11 * tb) << 5) & 0x7c00; \
	ta = (colorA >> 5) & 0x1f; \
	tb = (colorB >> 5) & 0x1f; \
	color4[1] |= (11 * ta + 21 * tb) & 0x3e0; \
	color4[2] |= (21 * ta + 11 * tb) & 0x3e0; \
	ta = colorA	& 0x1f; \
	tb = colorB	& 0x1f; \
	color4[1] |= (11 * ta + 21 * tb) >> 5; \
	color4[2] |= (21 * ta + 11 * tb) >> 5; \
	} \
}


void qt_decode_rpza(char *encoded, int encoded_size, char *decoded, int width,int height, int bytes_per_pixel)
{
	int stream_ptr = 0;
	int chunk_size;
	unsigned char opcode;
	int n_blocks;
	unsigned short colorA = 0, colorB = 0;
	unsigned char r = 0, g = 0, b = 0;
	unsigned char rgb4[4][3];
	unsigned short color4[4];
	unsigned char index, idx;

	int row_ptr = 0;
	int pixel_ptr = 0;
	int pixel_x, pixel_y;
	int row_inc = bytes_per_pixel * (width - 4);
	int max_height = row_inc * height;
	int block_x_inc = bytes_per_pixel * 4;
	int block_y_inc = bytes_per_pixel * width;
	int block_ptr;
	int total_blocks;


	/* First byte is always 0xe1. Warn if it's different */
	if ((unsigned char)encoded[stream_ptr] != 0xe1)
		fprintf(stderr, "roadpizza: First chunk byte is 0x%02x instead of 0x1e\n",
		(unsigned char)encoded[stream_ptr]);

	/* Get chunk size, ingnoring first byte */
	chunk_size = BE_32(&encoded[stream_ptr]) & 0x00FFFFFF;
	stream_ptr += 4;

	/* If length mismatch use size from MOV file and try to decode anyway */
	if (chunk_size != encoded_size)
		fprintf(stderr, "roadpizza: MOV chunk size (%d) != encoded chunk size (%d); using MOV chunk size\n",
		chunk_size, encoded_size);

	chunk_size = encoded_size;

	/* Number of 4x4 blocks in frame. */
	total_blocks = (width * height) / (4 * 4);

	/* Process chunk data */
	while (stream_ptr < chunk_size) {
		opcode = encoded[stream_ptr++]; /* Get opcode */

		n_blocks = (opcode & 0x1f) +1; /* Extract block counter from opcode */

		/* If opcode MSbit is 0, we need more data to decide what to do */
		if ((opcode & 0x80) == 0) {
			colorA = (opcode << 8) | ((unsigned char)encoded[stream_ptr++]);
			opcode = 0;
			if ((encoded[stream_ptr] & 0x80) != 0) {
				/* Must behave as opcode 110xxxxx, using colorA computed above.*/
				/* Use fake opcode 0x20 to enter switch block at the right place */
				opcode = 0x20;
				n_blocks = 1;
			}
		}
		switch (opcode & 0xe0) {
			/* Skip blocks */
			case 0x80:
				while (n_blocks--)
					ADVANCE_BLOCK();
				break;

				/* Fill blocks with one color */
			case 0xa0:
				colorA = BE_16 (&encoded[stream_ptr]);
				stream_ptr += 2;
				COLOR_TO_RGB (r, g, b, colorA);
				while (n_blocks--) {
					block_ptr = row_ptr + pixel_ptr;
					for (pixel_y = 0; pixel_y < 4; pixel_y++) {
						for (pixel_x = 0; pixel_x < 4; pixel_x++){
							PAINT_CURRENT_PIXEL(r, g, b, colorA);
						}
						block_ptr += row_inc;
					}
					ADVANCE_BLOCK();
				}
				break;

				/* Fill blocks with 4 colors */
			case 0xc0:
				colorA = BE_16 (&encoded[stream_ptr]);
				stream_ptr += 2;
			case 0x20:
				colorB = BE_16 (&encoded[stream_ptr]);
				stream_ptr += 2;
				COLORAB_TO_RGB4 (rgb4, color4, colorA, colorB);
				while (n_blocks--) {
					block_ptr = row_ptr + pixel_ptr;
					for (pixel_y = 0; pixel_y < 4; pixel_y++) {
						index = encoded[stream_ptr++];
						for (pixel_x = 0; pixel_x < 4; pixel_x++){
							idx = (index >> (2 * (3 - pixel_x))) & 0x03;
							PAINT_CURRENT_PIXEL(rgb4[idx][0], rgb4[idx][1], rgb4[idx][2], color4[idx]);
						}
						block_ptr += row_inc;
					}
					ADVANCE_BLOCK();
				}
				break;

				/* Fill block with 16 colors */
			case 0x00:
				block_ptr = row_ptr + pixel_ptr;
				for (pixel_y = 0; pixel_y < 4; pixel_y++) {
					for (pixel_x = 0; pixel_x < 4; pixel_x++){
						/* We already have color of upper left pixel */
						if ((pixel_y != 0) || (pixel_x !=0)) {
							colorA = BE_16 (&encoded[stream_ptr]);
							stream_ptr += 2;
						}
						COLOR_TO_RGB (r, g, b, colorA);
						PAINT_CURRENT_PIXEL(r, g, b, colorA);
					}
					block_ptr += row_inc;
				}
				ADVANCE_BLOCK();
				break;

				/* Unknown opcode */
			default:
				fprintf(stderr, "roadpizza: Unknown opcode %d in rpza chunk."
					" Skip remaining %lu bytes of chunk data.\n", opcode,
					(unsigned long)(chunk_size - stream_ptr));
				return;
		} /* Opcode switch */
	}
}
