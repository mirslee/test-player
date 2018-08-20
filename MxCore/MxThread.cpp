//
//  MxThread.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//
#include "stdafx.h"
#include "MxThread.h"

#ifdef WIN32
#define INIT_PTHREAD(t) memset(&t,0,sizeof(t))
#define PTHREAD_IS_VALID(t) (t.p!=nullptr)
#else
#define INIT_PTHREAD(t) t = nullptr;
#define PTHREAD_IS_VALID(t) (t!=nullptr)
#endif

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
    if (thread && PTHREAD_IS_VALID(thread->thread))
        pthread_join(thread->thread, nullptr);
}

void MxThread::exit() {
	if (PTHREAD_IS_VALID(thread)) {
		pthread_exit(nullptr);
		INIT_PTHREAD(thread);
	} 
}
