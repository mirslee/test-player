
#ifndef __MXINTERFACE_H__
#define __MXINTERFACE_H__

#include "MxTypes.h"

#define FEEXTINFO_AFD		0x1
#define FEEXTINFO_TC		0x2
#define FEEXTINFO_OFFSET	0x4
//frame的附加数据结合
typedef struct
{
	DWORD infotype;//附加数据有效的部分，FEEXTINFO_*的合集
	DWORD afd;
	DWORD tc;
	uint64 fileoffset;
	DWORD reserves[6];
}VXFEXTINFO;

#define IID_CMxObject 0x00001
#define LIID_IVxReadStream			0xe0000005
#define LIID_IVxAudioReadStream		0xe0000007
#define LIID_IVxVideoReadStream		0xe0000006
#define LIID_IVxSubtitleReadStream		0xe000000f
#define LIID_IVxReadStream2			0xe0000020
#define LIID_IVxSource 0xe0001000
#define LIID_IVxFastIO 0xe0001001


#endif //__MXINTERFACE_H__
