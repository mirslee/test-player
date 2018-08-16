//
//  MxThread.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//

#include "MxThread.h"

MxThread::MxThread() {
    thread = nullptr;
}

void MxThread::start() {
    int ret = pthread_create(&thread,nullptr,&MxThread::_run,(void*)this);
}

void* MxThread::_run(void* pThis) {
    ((MxThread*)pThis)->run();
}

void MxThread::run() {
    
}

void MxThread::join(MxThread* thread) {
    if (thread)
        pthread_join(thread->thread, nullptr);
}

void MxThread::exit() {
    if (thread)
        pthread_exit(nullptr);
}
