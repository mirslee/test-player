#include "stdafx.h"
#include "MxSynchronize.h"


#ifdef _WIN32

#else

#include <sys/time.h>
#include <CoreServices/CoreServices.h>

struct MxEvent_t {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	bool bManualReset;
	bool signal;
};

MxEvent mxCreateEvent(pthread_condattr_t* attr, bool bManualReset, bool bInitialState, const char* pName) {
	MxEvent_t *mxevent_t = new MxEvent_t;
	//memset(mxevent_t, 0, sizeof(MxEvent_t));
    //int pthread_condattr_init(pthread_condattr_t *);
	//pthread_condattr_init(attr);
    int ret = pthread_cond_init(&mxevent_t->cond, attr);
	ret = pthread_mutex_init(&mxevent_t->mutex, nullptr);
	mxevent_t->signal = bInitialState;
    mxevent_t->bManualReset = bManualReset;
	return (MxEvent)&mxevent_t;
}

bool mxCloseEvent(MxEvent event) {
	if (!event) { return false; }
	MxEvent_t *mxevent_t = (MxEvent_t*)event;
	pthread_mutex_destroy(&mxevent_t->mutex);
	pthread_cond_destroy(&mxevent_t->cond);
	delete mxevent_t;
	return true;
}

bool mxSetEvent(MxEvent event) {
	/*if (!event) { return false; }
	MxEvent_t *mxevent_t = (MxEvent_t*)event;
	pthread_mutex_lock(&mxevent_t->mutex);
	if (mxevent_t->bManualReset && mxevent_t->signal) {
		pthread_mutex_unlock(&mxevent_t->mutex);
		return true;
	}
    bool ret = false;
    if (mxevent_t->bManualReset) {
        ret = pthread_cond_broadcast(&mxevent_t->cond) == 0;
    } else {
        ret = pthread_cond_signal(&mxevent_t->cond) == 0;
    }

	if (ret) {
		mxevent_t->signal = true;
	}
	pthread_mutex_unlock(&mxevent_t->mutex);
	return ret;*/
    
    MxEvent_t *mxevent_t = (MxEvent_t*)event;
    pthread_mutex_lock(&mxevent_t->mutex);
    bool ret = pthread_cond_broadcast(&mxevent_t->cond) == 0;
    pthread_mutex_unlock(&mxevent_t->mutex);
    return ret;
}

bool mxResetEvent(MxEvent event) {
    if (!event) { return false; }
    MxEvent_t *mxevent_t = (MxEvent_t*)event;
    pthread_mutex_lock(&mxevent_t->mutex);
    mxevent_t->signal = false;
    pthread_mutex_unlock(&mxevent_t->mutex);
    return true;
}

unsigned long mxWaitObject(MxEvent event, unsigned long dwMilliseconds) {
    MxEvent_t *mxevent_t = (MxEvent_t*)event;
    pthread_mutex_lock(&mxevent_t->mutex);
    int ret = pthread_cond_wait(&mxevent_t->cond, &mxevent_t->mutex);
    pthread_mutex_unlock(&mxevent_t->mutex);
    return ret;
//    MxEvent_t *mxevent_t = (MxEvent_t*)event;
//    pthread_mutex_lock(&mxevent_t->mutex);
//    if(mxevent_t->signal) {
//        if (!mxevent_t->bManualReset) { mxevent_t->signal = false; }/*非人工的职能有一个线程激活*/
//        pthread_mutex_unlock(&mxevent_t->mutex);
//        return WAIT_OBJECT_0;
//    }
//    if (INFINITE == dwMilliseconds) {
//        while (!mxevent_t->signal) {
//            int ret = pthread_cond_wait(&mxevent_t->cond, &mxevent_t->mutex);
//            if(0 != ret) {
//                pthread_mutex_unlock(&mxevent_t->mutex);
//                return ret;
//            }
//            if (!mxevent_t->bManualReset) { mxevent_t->signal = false; }/*非人工的职能有一个线程激活*/
//            pthread_mutex_unlock(&mxevent_t->mutex);
//            return WAIT_OBJECT_0;
//        }
//    } else if (0 == dwMilliseconds) {
//        if (mxevent_t->signal) {
//            if (!mxevent_t->bManualReset) { mxevent_t->signal = false; }/*非人工的职能有一个线程激活*/
//            pthread_mutex_unlock(&mxevent_t->mutex);
//            return WAIT_OBJECT_0;
//        } else {
//            pthread_mutex_unlock(&mxevent_t->mutex);
//            return WAIT_TIMEOUT;
//        }
//    }
//
//    dwMilliseconds *= 1000;
//    timespec abstime;
//    timeval now;
//    gettimeofday(&now, 0);
//    abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
//    abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
//    while (!mxevent_t->signal) {
//        int ret = pthread_cond_timedwait(&mxevent_t->cond, &mxevent_t->mutex, &abstime);
//        if (0 != ret) {
//            pthread_mutex_unlock(&mxevent_t->mutex);
//            return ret;
//        }
//    }
//    if (!mxevent_t->bManualReset) { mxevent_t->signal = false; }/*非人工的职能有一个线程激活*/
//    pthread_mutex_unlock(&mxevent_t->mutex);
//    return WAIT_OBJECT_0;
}

unsigned long mxWaitObjects(unsigned long  nCount, MxEvent *event, unsigned long  dwMilliseconds) {
    if (INFINITE == dwMilliseconds) {
        for (unsigned long i = 0; i < nCount; i++) {
            MxEvent_t *mxevent_t = (MxEvent_t*)event[i];
            if (!mxevent_t) { continue; }
            pthread_mutex_lock(&mxevent_t->mutex);
            if (mxevent_t->signal) {
                if (!mxevent_t->bManualReset) { mxevent_t->signal = false; }/*非人工的职能有一个线程激活*/
                pthread_mutex_unlock(&mxevent_t->mutex);
                continue;
            }
            int ret = 0;
            while (!mxevent_t->signal) {
                ret = pthread_cond_wait(&mxevent_t->cond, &mxevent_t->mutex);
                if (0 != ret) {
                    pthread_mutex_unlock(&mxevent_t->mutex);
                    return ret;
                }
            }
            if (!mxevent_t->bManualReset) { mxevent_t->signal = false; }/*非人工的职能有一个线程激活*/
            pthread_mutex_unlock(&mxevent_t->mutex);
        }
    } else {
        dwMilliseconds *= 1000;
        timespec abstime;
        timeval now;
        gettimeofday(&now, 0);
        abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
        abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
        for (unsigned long i = 0; i < nCount; i++) {
            MxEvent_t *mxevent_t = (MxEvent_t*)event[i];
            if (!mxevent_t) {
                continue;
            }
            pthread_mutex_lock(&mxevent_t->mutex);
            if (0 == dwMilliseconds) {
                if (!mxevent_t->signal) {
                    pthread_mutex_unlock(&mxevent_t->mutex);
                    return WAIT_TIMEOUT;
                }
                if (!mxevent_t->bManualReset) {
                    mxevent_t->signal = false;
                }
                pthread_mutex_unlock(&mxevent_t->mutex);
                continue;
            }
            
            while (!mxevent_t->signal) {
                int ret = pthread_cond_timedwait(&mxevent_t->cond, &mxevent_t->mutex, &abstime);
                if (0 != ret) {
                    pthread_mutex_unlock(&mxevent_t->mutex );
                    return ret;
                }
            }
            if (!mxevent_t->bManualReset) {
                mxevent_t->signal = false;
            }
            pthread_mutex_unlock(&mxevent_t->mutex);
        }
    }
    return 0;
}

unsigned long GetTickCount() {
    int upt = AbsoluteToDuration(UpTime());
    if (upt < 0) {
        upt = (-upt) / 1000;
    }
    return upt;
}

void mxSleep(unsigned int millseconds) {
    AbsoluteTime wakeup = AddDurationToAbsolute(millseconds,UpTime());
    MPDelayUntil(&wakeup);
}







#endif
