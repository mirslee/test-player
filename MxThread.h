//
//  MxThread.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//

#ifndef MxThread_h
#define MxThread_h

class MxThread {
    
public:
    MxThread();
    void start();
    virtual void run();
    
    void join(MxThread* thread);
    void exit();
    
public:
    pthread_t thread;
    
private:
    static void* _run(void* pThis);
};


#endif /* MxThread_hpp */
