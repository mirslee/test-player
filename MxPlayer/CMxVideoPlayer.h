

#ifndef __CMXVIDEOPLAYER_H__
#define __CMXVIDEOPLAYER_H__

#include "../MxCore/MxQueue.h"
#include "../MxCore/MxThread.h"
#include "MxDemuxer.h"
#include "MxVideoDecoder.h"
#include "MxPlayer.h"

#include "MxTypes.h"
#include "MxPointer.h"
#include "CMxQueue.h"

class MxThread;

class DecodeSendThread: public MxThread {
public:
    void start(MxDemuxer* demul, MxQueue<AVPacket*>* decodeSendQueue) {
        this->demul = demul;
        this->decodeSendQueue = decodeSendQueue;
        MxThread::start();
    }
    void run() {
        if (demul) {
            while (1) {
                AVPacket *packet = (AVPacket*)av_malloc(sizeof(AVPacket));
                int ret = demul->readPacket(packet);
                if (ret < 0) {
                    if (ret == AVERROR_EOF) {
                        cout << "get packet end" << endl;
                        break;
                    } else {
                        cout << "get packet failed" << endl;
                    }
                } else {
                    /*if (packet->stream_index == 1)
                        cout << "get packet: dts=" << packet->dts << " pts=" << packet->pts << endl;
                    if (packet->dts != packet->pts)
                        cout << "packet->dts" << endl;*/
                    decodeSendQueue->push(packet);
                }
            }
        }
    }
private:
    MxDemuxer* demul;
    MxQueue<AVPacket*>* decodeSendQueue;
};

class DecodeFinshedThread: public MxThread {
public:
    MxQueue<AVPacket*>* decodeSendQueue = nullptr;
    MxQueue<AVFrame*>* decodeFinshed;
    MxVideoDecoder* dec;
    
    void start(MxVideoDecoder* decoder, MxQueue<AVPacket*>* decodeSendQueue, MxQueue<AVFrame*>* decodeFinshed) {
        this->dec = decoder;
        this->decodeSendQueue = decodeSendQueue;
        this->decodeFinshed = decodeFinshed;
        MxThread::start();
    }
    void run() {
        while (1) {
            AVPacket *packet = nullptr;
            if (0 == decodeSendQueue->pop(&packet)) {
                if (!packet)
                    continue;
                
                AVFrame* frame = dec->decode(packet);
                if (frame) {
                    decodeFinshed->push(frame);
                }
                av_packet_unref(packet);
                av_free(packet);
            } else {
                av_usleep(1);
            }
        }
    }
};
class OutThread: public MxThread {
public:
    void start(MxQueue<AVFrame*>* decodeFinshed) {
        this->decodeFinshed = decodeFinshed;
        MxThread::start();
    }
    
    MxQueue<AVFrame*>* decodeFinshed;
    void run() {
        while (1) {
            AVFrame* frame = nullptr;
            if (0 == decodeFinshed->pop(&frame)) {
                if (!frame)
                    continue;
                
                if(frame->linesize[0] < 0) {
                    //frame->data[0] + frame->linesize[0]*(frame->height-1), -frame->linesize[0];
                } else {
                    //frame->data[0], fame->linesize[0]
                }
                
                cout << "best_effort_timestamp: " << frame->best_effort_timestamp << " width=" << frame->width << " height=" << frame->height << endl;
                av_frame_unref(frame);
                av_frame_free(&frame);
            } else {
                av_usleep(1);
            }
        }
    }
};

class CMxVideoPlayer: public MxVideoPlayer, public CMxObject {
    
    MX_OBJECT
public:
    CMxVideoPlayer();
    bool open(string filepath);
    
private:
    
    MxDemuxer* demul;
    MxVideoDecoder* dec;
    
    MxQueue<AVPacket*> packetQueue;
    DecodeSendThread decodeSend;
    MxQueue<AVPacket*> decodeSendQueue;
    DecodeFinshedThread decodeFinshed;
    MxQueue<AVFrame*> decodeFinshedQueue;
    OutThread outThread;
    
protected:
    static void* static_playOutFun(void* pObj) {
        ((CMxVideoPlayer*)pObj)->_playOutFun();
        return 0;
    }
    void _playOutFun();
    
    uint64 m_dwStartClock;
    int m_timestep;
    int m_framestep;
    double m_fSamplesOfFrame;
    double m_playreqframe;
    
    int64 m_curframe;
    int64 m_startframe;
    int64 m_endframe;
    double m_playspeed;
    int     m_cueframes;
    CVxQueue<EFFECTOUT_SURFACE> m_qOutput;
    
    CMxEvent m_hOutClock;
    CMxEvent m_hBeginPlay;
    bool m_bExitOut;
    bool m_displayfirst;
};

#endif /* __CMXVIDEOPLAYER_H__ */


