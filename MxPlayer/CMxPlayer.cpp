
#include "stdafx.h"
#include "CMxPlayer.h"
#include "MxLog.h"


CMxPlayer::CMxPlayer() {
    pthread_create(&_cmdThread, nullptr, __static_cmdThreadFun, this);
}

CMxPlayer::~CMxPlayer() {
    printf("......");
}

void CMxPlayer::__cmdThreadFun() {
    
    for (;;) {
        PlayerCommand cmd;
		if (!__cmdQueue.Pop(&cmd,true))
			continue;
        
        switch (cmd) {
            case  PC_play:
                mx_debug("PC_play");
                break;
            case PC_pause:
                mx_debug("PC_pause");
                break;
            case PC_stop:
                mx_debug("PC_stop");
                break;
            case PC_stopToFirst:
                mx_debug("PC_stopToFirst");
                break;
            default:
                break;
        }
    }
}

MXPLAER_API MxPlayer* creatMxPlayer() {
    return new CMxPlayer();
}
