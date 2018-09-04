//
//  MxPlayer.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//

#ifndef __MXPLAYER_H__
#define __MXPLAYER_H__

#include "MxObject.h"

struct MXPLAER_API MxPlayer: public MxObject {
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
};

MXPLAER_API MxPlayer* creatMxPlayer();


struct MXPLAER_API MxVideoPlayer: public MxObject {
    
};

struct MXPLAER_API MxAudioPlayer: public MxObject {
    
};

#endif /* __MXPLAYER_H__ */
