
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

void CMxVideoPlayer::_playOutFun() {
    
    int padtime = m_timestep - 1;
    uint64 masktime = m_timestep - 1;
    masktime = ~masktime;
    
    int64 i64outframe = 0;
    m_playreqframe = m_startframe+m_cueframes*m_playspeed;
    if (m_playreqframe < 0)
        m_playreqframe = 0;
    
    uint dwDelay = (m_latency+padtime)/m_timestep;
    if (dwDelay < 2)
        dwDelay = 2;
    
    if (m_qOutput.GetMaxSize() == PIPELINE_RECORD) {
        
    }
    
    uint waittime = 100;
    while (WAIT_OK != mxWaitEvent(m_hBeginPlay,waittime)) {
        if (m_displayfirst) {
            uint64 pauseclock = (GetSystemClockTime()+dwDelay*m_timestep+padtime) & masktime;
            m_qOutput.Lock();
            EFFECTOUT_SURFACE* efx = m_qOutput.GetDataPtr();
            int size = m_qOutput.GetSize();
            if (size > 0) {
                efx[0].ullTimeStamp = pauseclock;
                __coreoutput(&efx[0], efx[0].speed);
                waittime = INFINITY;
            }
            m_qOutput.Unlock();
        }
    }
}
