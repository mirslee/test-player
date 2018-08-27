//
//  main.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/25.
//

#include "stdafx.h"
#include "MxCore/MxSynchronize.h"
#include "./MxPlayer/clock/CMxMediaSysClock.h"
#include <QtCore/qlogging.h>

MxEvent event = nullptr;

void* thread1(void*) {

	while(1) {
		if (WAIT_OK == mxWaitObject(event, WAIT_INFINITE)) {
			qDebug("thread1: success");
		}
		else {
			qDebug("thread1: false");
			std::cout << "thread1:false";
		}
	}
    
	return 0;
}

void* thread2(void*) {
	while (1) {
		if (WAIT_OK == mxWaitObject(event, WAIT_INFINITE)) {
			qDebug("thread2: success");
		}
		else {
			qDebug("thread2: false");
		}
	}
	return 0;
}

int main(int argc, char** argv) {
    
    
    /*event = mxCreateEvent(nullptr, false, false, "111");
    pthread_t t1;
    pthread_t t2;
    pthread_create(&t1, nullptr, thread1, nullptr);
    pthread_create(&t2, nullptr, thread2, nullptr);

	
    while (1)
    {
		Sleep(1000);
		mxActiveEvent(event);
    }*/

	IVxSystemClock *clock; 
	sysclk_cinfo info;
	info.rate = 25;
	info.scale = 1;
	vxCreateSystemClock(NULL,&info, NULL, &clock);
	while (1)
	{
		sleep(10000);
		//qDebug("%ld",clock->GetClock());
	}
    return 0;
}
