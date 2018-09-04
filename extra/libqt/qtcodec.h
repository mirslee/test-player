#pragma once
#include "qttype.h"
#include "colormodels.h"
#include "qtsdk\ImageCompression.h"
#include "vfw.h"
//video dec
/*
//"rpza"
void* init_roadpizza(ImageDescriptionPtr desc,int* colormodel);
void delete_roadpizza(void* codec);
int decode_roadpizza(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE *output,long pitch);
*/

//"cvid"
void* init_cvid(ImageDescriptionPtr desc,int* colormodel);
void delete_cvid(void* codec);
int decode_cvid(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch);

//"rle "
void* init_rle(ImageDescriptionPtr desc,int* colormodel);
void delete_rle(void* codec);
int decode_rle(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch);

//"smc "
void* init_smc(ImageDescriptionPtr desc,int* colormodel);
void delete_smc(void* codec);
int decode_smc(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch);

/*
//"8BPS "
void* init_8BPS(ImageDescriptionPtr desc,int* colormodel);
void delete_8BPS(void* codec);
int decode_8BPS(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch);
*/
//"SVQ1"
void* init_svq1(ImageDescriptionPtr desc,int* colormodel);
void delete_svq1(void* codec);
int decode_svq1(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch);


//"yuv2"
void* init_yuv2(ImageDescriptionPtr desc,int* colormodel);
void delete_yuv2(void* codec);
int decode_yuv2(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch);

/*
//"jpeg"
void* init_JPEG(ImageDescriptionPtr desc,int* colormodel);
void delete_JPEG(void* codec);
int decode_JPEG(void* codec,ImageDescriptionPtr desc,BYTE *input,ULONG inputsize,BYTE  *output,long pitch);
*/




//audio dec
//"twos"
void* init_twos(WAVEFORMATEX* wfx);
void delete_twos(void* code);
int decode_twos(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);

//"sowt"
#define init_sowt init_twos
#define delete_sowt delete_twos
int decode_sowt(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);

//"alaw"
#define init_alaw init_twos
#define delete_alaw delete_twos
int decode_alaw(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);


void* init_ulaw(WAVEFORMATEX* wfx);
void delete_ulaw(void* code);
int decode_ulaw(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);

//"raw "
#define init_raw init_twos
#define delete_raw delete_twos
int decode_raw(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);

//"ima4"
void* init_ima4(WAVEFORMATEX* wfx);
void delete_ima4(void* code);
int decode_ima4(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);


//"MAC3"
void* init_mace3(WAVEFORMATEX* wfx);
void delete_mace3(void* code);
int decode_mace3(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);

//"MAC6"
#define init_mace6 init_mace3
#define delete_mace6 delete_mace3
int decode_mace6(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);


//"FAAC"
void* init_faac(WAVEFORMATEX* wfx);
void delete_faac(void* code);
int decode_faac(void* codec,WAVEFORMATEX* wfx,BYTE *input,ULONG inputsize,BYTE  *output,ULONG outsize);


//"mp4a"
void* init_mp4a(WAVEFORMATEX* wfx);
#define delete_mp4a delete_faac
#define decode_mp4a decode_faac
