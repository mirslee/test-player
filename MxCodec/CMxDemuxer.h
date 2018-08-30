

#ifndef __CMXDEMUXER_H__
#define __CMXDEMUXER_H__

#include "MxCodec.h"

class CMxDemuxer: public MxDemuxer, public CMxObject {
    MX_OBJECT
public:
    CMxDemuxer();
};

CMxDemuxer c;

#endif /* __CMXDEMUXER_H__ */
