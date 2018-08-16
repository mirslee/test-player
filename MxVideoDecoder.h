//
//  MxVideoDecoder.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//

#ifndef MxVideoDecoder_h
#define MxVideoDecoder_h

class MxVideoDecoder {
    
public:
    MxVideoDecoder(AVCodecContext* pCodecCtx, AVCodec *pVideoDec);
    
    AVFrame* decode(AVPacket* packet);
private:
    AVCodecContext* _pCodecCtx = nullptr;
    AVCodec* _pVideoDec;
};

#endif /* MxVideoDecoder_hpp */
