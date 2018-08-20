//
//  MxThreadCond.hpp
//  MxCore
//
//  Created by sz17112850M01 on 2018/8/20.
//  Copyright © 2018年 lixiaopeng. All rights reserved.
//

#ifndef MxThreadCond_hpp
#define MxThreadCond_hpp

class MxThreadMutex;
class MXCORE_API MxThreadCond {
    
public:
    MxThreadCond();
    ~MxThreadCond();
    
    void wait(MxThreadMutex *lockedMutex);
    
    void wakeOne();
    void wakeAll();
    
    void notify_one() { wakeOne(); }
    void notify_all() { wakeAll(); }
    
private:
    pthread_cond_t cond;
};

#endif /* MxThreadCond_hpp */
