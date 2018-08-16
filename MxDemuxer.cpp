//
//  MxDemultiplexer.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//

#include "MxDemuxer.h"
#include "Template.h"

MxDemuxer::MxDemuxer()
{
}


MxDemuxer::~MxDemuxer()
{
    clear();
}

void MxDemuxer::clear() {
    DELETE_Ptr(_pFormatCtx);
}

bool MxDemuxer::open(string filepath)
{
    clear();
    _pFormatCtx = avformat_alloc_context();
    if (0 != avformat_open_input(&_pFormatCtx,filepath.c_str(), nullptr, nullptr)) {
        return false;
    }
    
    videoStream = -1;
    for (unsigned int i = 0; i < _pFormatCtx->nb_streams; i++) {
        if (_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }
    }
    
    if (-1 == videoStream)
       return false;
    
    bool ret = createVideoDecoder();
    return ret;
}

MxVideoDecoder* MxDemuxer::createVideoDecoder() {
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(nullptr);
    if (!pCodecCtx)
        return nullptr;
    
    int ret = avcodec_parameters_to_context(pCodecCtx, _pFormatCtx->streams[videoStream]->codecpar);
    if (ret < 0)
        return nullptr;
    
    pCodecCtx->pkt_timebase = _pFormatCtx->streams[videoStream]->time_base;
    AVCodec *pVideoDec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (!pVideoDec)
        return nullptr;

    if (0 != avcodec_open2(pCodecCtx, pVideoDec, nullptr)) {
        return nullptr;
    }
    
    return new MxVideoDecoder(pCodecCtx,pVideoDec);
}

int MxDemuxer::readPacket(AVPacket *packet) {
    /*int ret = av_read_frame(_pFormatCtx, packet);
    if (ret < 0) {
        if (ret == AVERROR_EOF)
            return -1;
    }
    
    if (packet->stream_index == videoStream) {
        return true;
    }*/
    
    return av_read_frame(_pFormatCtx, packet);
}










