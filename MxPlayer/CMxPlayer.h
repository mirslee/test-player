

#ifndef __CMXPLAYER_H__
#define __CMXPLAYER_H__

#include "MxTypes.h"
#include "MxPlayer.h"
#include "MxPointer.h"
#include "CMxQueue.h"
#include "CMxPlayerThreadController.h"
#include "../CMxMediaSysClock.h"

enum MxCommandType {
    MXCT_PLAY,
    MXCT_PAUSE,
    MXCT_STOP,
};

struct MxCommand {
    MxCommandType type;
    CMxEvent finish;
    mxuvoidptr params[8];
    bool* ret;
};

class CMxPlayer: public MxPlayer, public CMxObject {
    
    MX_OBJECT
    friend class CMxPlayerThreadController;
public:
    
    enum
    {
        driver_Error = -1,
        driver_NoInit,
        driver_Cue,
        driver_Running,
        driver_Stopped,
        driver_Unknow,
    } m_state;
    
    CMxPlayer();
    virtual ~CMxPlayer();
    
    virtual void play() {}
    virtual void pause() {}
    virtual void stop() {}
    
protected:
    CMxMutex _csLock;
    CMxEvent _cmdEvent;
    CMxEvent _stopEvent;
    
    double m_playspeed;//播放速度
    
    int64 _duration;
    int64 _start;
    int64 _end;
    int64 _frame;
    
    uint64 _startClock;
    uint64 _endClock;
    
    int _timeStep;
    
    bool _isPlaying;
    bool _isLoop;
    
    CMxObjectPointer<MxSystemClock> _pClock;
    CMxObjectPointer<MxVideoPlayer> _pVideoPlayer;
    CMxObjectPointer<MxAudioPlayer> _pAudioPlayer;
    
private:
    pthread_t _cmdThread;
    static void* __static_cmdThreadFun(void* pThis) {
        ((CMxPlayer*)pThis)->__cmdThreadFun();
        return nullptr;
    }
    void __cmdThreadFun();
	CMxQueue<MxCommandType> __cmdQueue;
    
protected:
    
};

#endif /* __CMXPLAYER_H__ */
