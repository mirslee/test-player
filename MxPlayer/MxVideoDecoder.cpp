//
//  MxVideoDecoder.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//
#include "stdafx.h"
#include "MxVideoDecoder.h"

MxVideoDecoder::MxVideoDecoder(AVCodecContext* pCodecCtx, AVCodec *pVideoDec) {
    _pVideoDec = pVideoDec;
    _pCodecCtx = pCodecCtx;
}


AVFrame* MxVideoDecoder::decode(AVPacket* packet) {
    if (0 == avcodec_send_packet(_pCodecCtx, packet)) {
        AVFrame *frame = av_frame_alloc();
        
        int ret = avcodec_receive_frame(_pCodecCtx, frame);
        if (0 == ret)
            return frame;
        else {
            av_frame_unref(frame);
            av_frame_free(&frame);
        }
    }
    
    return nullptr;
}
