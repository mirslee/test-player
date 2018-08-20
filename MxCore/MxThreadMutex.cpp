//
//  MxMutex.cpp
//  MxCore
//
//  Created by sz17112850M01 on 2018/8/20.
//  Copyright © 2018年 lixiaopeng. All rights reserved.
//
#include "stdafx.h"
#include "MxThreadMutex.h"


MxThreadMutex::MxThreadMutex() {
    
    pthread_mutex_init(&mutex, nullptr);
}

MxThreadMutex::~MxThreadMutex() {
    pthread_mutex_destroy(&mutex);
}

void MxThreadMutex::lock() {
    pthread_mutex_lock(&mutex);
}

void MxThreadMutex::unlock() {
    pthread_mutex_unlock(&mutex);
}
