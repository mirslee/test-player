//
//  MxDemultiplexer.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//

#ifndef MxDemuxer_h
#define MxDemuxer_h
#include "MxQueue.h"
#include "MxVideoDecoder.h"

class MxDemuxer {
    
public:
    MxDemuxer();
    ~MxDemuxer();
    
    bool open(string filepath);
    int readPacket(AVPacket *packet);
    
    MxVideoDecoder* createVideoDecoder();

    MxQueue<AVPacket> packetQueue;
    
protected:
    
    int videoStream;
    AVFormatContext* _pFormatCtx = nullptr;
    
private:
    void clear();
    
};

#endif /* MxDemultiplexer_hpp */
