//
//  MxThreadCond.cpp
//  MxCore
//
//  Created by sz17112850M01 on 2018/8/20.
//  Copyright © 2018年 lixiaopeng. All rights reserved.
//

#include "stdafx.h"
#include "MxThreadCond.h"
#include "MxThreadMutex.h"


MxThreadCond::MxThreadCond() {
    
    pthread_cond_init(&cond, nullptr);
}

MxThreadCond::~MxThreadCond() {
    pthread_cond_destroy(&cond);
}


void MxThreadCond::wait(MxThreadMutex *lockedMutex) {
    pthread_cond_wait(&cond, &lockedMutex->mutex);
}

void MxThreadCond::wakeOne() {
    
}

void MxThreadCond::wakeAll() {
    
}
