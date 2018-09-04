#ifndef __CMXPLAYERTHREADCONTROLLER_H__
#define __CMXPLAYERTHREADCONTROLLER_H__

#include "CMxPlayer.h"

class CMxPlayerThreadController {
public:
    CMxPlayerThreadController(CMxPlayer* player);
    
protected:
    CMxPlayer   *_pPlayer;
    bool        _isExit;
    pthread_t   _thread;
    CMxEvent    _event;
    void        _threadFun();
    
    static void* static_threadFun(void* pObj) {
        ((CMxPlayerThreadController*)pObj)->_threadFun();
        return 0;
    }
}

#endif /* __CMXPLAYERTHREADCONTROLLER_H__ */
