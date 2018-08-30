
#include "stdafx.h"
#include "CMxVideoPlayer.h"

CMxVideoPlayer::CMxVideoPlayer()
{
}

bool CMxVideoPlayer::open(string filepath)
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
