//
//  MxVideoPlayer.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//
#include "stdafx.h"
#include "MxVideoPlayer.h"

MxVideoPlayer::MxVideoPlayer()
{
}

bool MxVideoPlayer::open(string filepath)
{
    demul = new MxDemuxer();
    //demul->packetQueue = packetQueue;
    bool ret = demul->open(filepath);
    if (ret) {
        dec = demul->createVideoDecoder();
    }
    
    if (dec) {
        decodeSend.start(demul,&decodeSendQueue);
        decodeFinshed.start(dec,&decodeSendQueue,&decodeFinshedQueue);
        outThread.start(&decodeFinshedQueue);
    }

	return true;
}
