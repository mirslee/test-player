#include "stdafx.h"
#include "CMxPlayerThreadController.h"

CMxPlayerThreadController::CMxPlayerThreadController(CMxPlayer* player)
{
    _pPlayer = player;
    _event = player->_pClock->CreateClockEvent();
    _isExit = false;
    pthread_create(&m_hCtrlThread,&attr,static_threadFun,this);
}

void CMxPlayerThreadController::_threadFun() {
    
    //"主控线程"
    int padtime = _pPlayer->_timeStep - 1;
    int64 timemask = padtime;
    timemask = ~timemask;
    
    const sysclk_cinfo* info = _pPlayer->__pClock->GetCreateInfo();
    int uvflash = 160*info->rate/info->scale/(1000*info->scale);
    
    int64 lastclock = _pPlayer->__pClock->GetTime();
    while (!_isExit) {
        if (WAIT_OK != mxWaitEvent(_event, 1000)) {
            continue;
        }
        
        uint64 ullSystemClock = _player->__pClock->GetTime();
        if (_player->_isPlaying) {
            if (ullSystemClock >= m_endclock) { //播放完了
                if (_pPlayer->_isLoop) { //循环播放
                    CMxMutexLocker locker(&_pPlayer->__csLock);
                    bool ret = true;
                    MxCommand cmd = {MXCT_STOP,_pPlayer->_cmdEvent,{0,true},&ret};
                    while (!_player->__cmdQueue.Push(cmd)) {
                        continue;
                    }
                    mxWaitEvent(_pPlayer->_cmdEvent, INFINITY);
                    _pPlayer->m_state = CMxPlayer::driver_Stopped;
                    mxSetEvent(_pPlayer->_stopEvent);
                    //从头开始播放
                } else {
                    int cut = fabs(_pPlayer->m_playspeed-1) < 0.0001 ? 1 : 0;
                    _pPlayer->_frame = (int64)((_pPlayer->_endClock-_pPlayer->_startClock)/_pPlayer->_timeStep*_pPlayer->m_playspeed+_pPlayer->_start);
                    //seek
                    _pPlayer->_isPlaying = false;
                    
                    CMxMutexLocker locker(&_pPlayer->__csLock);
                    bool ret = true;
                    MxCommand cmd = {MXCT_STOP,_pPlayer->_cmdEvent,{0,true},&ret};
                    while (!_player->__cmdQueue.Push(cmd)) {
                        continue;
                    }
                    _pPlayer->m_state = CMxPlayer::driver_Stopped;
                    mxSetEvent(_pPlayer->_stopEvent);
                }
            } else if (ullSystemClock >= _pPlayer->_startClock) {
                if (!(ullSystemClock & padtime)) {
                    
                }
            }
        }
    }
}
