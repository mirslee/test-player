//
//  MxThread.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//
#include "stdafx.h"
#include "MxThread.h"

MxThread::MxThread() {
	INIT_PTHREAD(thread);
}

void MxThread::start() {
    int ret = pthread_create(&thread,nullptr,&MxThread::_run,(void*)this);
}

void* MxThread::_run(void* pThis) {
    ((MxThread*)pThis)->run();
	return nullptr;
}

void MxThread::run() {
    
}

void MxThread::join(MxThread* thread) {
    if (thread)
        pthread_join(thread->thread, nullptr);
}

void MxThread::exit() {
	if (PTHREAD_IS_VALID(thread)) {
		pthread_exit(nullptr);
		INIT_PTHREAD(thread);
	} 
}
