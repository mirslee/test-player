

#ifndef __CMXPLAYER_H__
#define __CMXPLAYER_H__

#include "MxPlayer.h"
#include "MxPointer.h"
#include "CMxQueue.h"

enum PlayerCommand {
    PC_play,
    PC_pause,
    PC_stop,
    PC_stopToFirst,
};

class CMxPlayer: public MxPlayer, public CMxObject {
    
    MX_OBJECT
public:
    CMxPlayer();
    virtual ~CMxPlayer();
    
    virtual void play() {PlayerCommand cmd = PC_play; __cmdQueue.Push(cmd);}
    virtual void pause() {PlayerCommand cmd = PC_pause; __cmdQueue.Push(cmd);}
    virtual void stop() {PlayerCommand cmd = PC_stop; __cmdQueue.Push(cmd);}
    
protected:
    CMxObjectPointer<MxVideoPlayer> _pVideoPlayer;
    CMxObjectPointer<MxAudioPlayer> _pAudioPlayer;
    
private:
    pthread_t _cmdThread;
    static void* __static_cmdThreadFun(void* pThis) {
        ((CMxPlayer*)pThis)->__cmdThreadFun();
        return nullptr;
    }
    void __cmdThreadFun();
	CMxQueue<PlayerCommand> __cmdQueue;
};

#endif /* __CMXPLAYER_H__ */
