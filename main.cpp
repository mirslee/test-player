//
//  main.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/25.
//

#include "stdafx.h"
#include "MxSynchronize.h"
#include "./MxPlayer/clock/CMxMediaSysClock.h"
#include <QtCore/qlogging.h>
#include "MxMemory.h"
#ifdef __APPLE__
#include <unistd.h>
#endif
#include "MxTypes.h"
#include "MainWidget.h"

CMxEvent event = nullptr;

void* thread1(void*) {

	while(1) {
		if (WAIT_OK == mxWaitEvent(event, WAIT_INFINITE)) {
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
		if (WAIT_OK == mxWaitEvent(event, WAIT_INFINITE)) {
			qDebug("thread2: success");
		}
		else {
			qDebug("thread2: false");
		}
	}
	return 0;
}

#include <sys/time.h>
uint64_t usTime()
{
    struct timeval tv;
    uint64_t usec;
    
    gettimeofday(&tv, NULL);
    
    usec = ((uint64_t)tv.tv_sec)*1000000LL;
    usec += tv.tv_usec;
    
    return usec;
}

int main(int argc, char** argv) {
    
    //同步测试
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
    
    /*CMxMutex mutex;
    mxMutexInit(&mutex);
    CMxMutexLocker locker(&mutex);

    //时钟测试
	MxSystemClock *clock;
	sysclk_cinfo info;
	info.rate = 25;
	info.scale = 1;
	mxCreateSystemClock(NULL,&info, NULL, &clock);
	while (1)
	{
		sleep(10000);
		//qDebug("%ld",clock->GetClock());
	}*/
    
    //内存测试
    
    /*uint64_t us_begin;
    uint64_t us_end;
    uint64_t t1, t2;
    
    us_begin  = usTime();
    for(int i = 0; i < 1000000; i++)  {
        void* p = mx_pool_alloc(10);
        mx_pool_free(p);
    }
    us_end  = usTime();
    t1 = (us_end - us_begin);
    
    us_begin  = usTime();
    for(int i = 0; i < 1000000; i++)  {
        void* p = malloc(10);
        free(p);
    }
    us_end  = usTime();
    t2 = (us_end - us_begin);*/
    
    QApplication a(argc,argv);
    MainWidget w;
    w.resize(300, 300);
    w.show();
    a.exec();
    
    return 0;
}
