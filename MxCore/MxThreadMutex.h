//
//  MxMutex.hpp
//  MxCore
//
//  Created by sz17112850M01 on 2018/8/20.
//  Copyright © 2018年 lixiaopeng. All rights reserved.
//

#ifndef MxMutex_hpp
#define MxMutex_hpp

class MXCORE_API MxThreadMutex {
    
public:
    MxThreadMutex();
    ~MxThreadMutex();
    
    void lock();
    void unlock();
    
public:
    pthread_mutex_t mutex;
};

#endif /* MxMutex_h */
