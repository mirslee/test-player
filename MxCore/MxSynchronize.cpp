#include "stdafx.h"
#include "MxSynchronize.h"
#include <time.h>

struct MxEvent_t {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	bool broadcast;
	bool active;
};

MxEvent mxCreateEvent(pthread_condattr_t* attr, bool broadcast, bool active, const char* pName) {
	MxEvent_t *mxevent_t = new MxEvent_t;
	int ret = pthread_cond_init(&mxevent_t->cond, attr);
	ret = pthread_mutex_init(&mxevent_t->mutex, nullptr);
	mxevent_t->active = active;
	mxevent_t->broadcast = broadcast;
	return (MxEvent)mxevent_t;
}

void mxCloseEvent(MxEvent event) {
	if (!event)
		return;
	MxEvent_t *mxevent_t = (MxEvent_t*)event;
	pthread_mutex_destroy(&mxevent_t->mutex);
	pthread_cond_destroy(&mxevent_t->cond);
	delete mxevent_t;
	mxevent_t = nullptr;
}

bool mxActiveEvent(MxEvent event) {
	if (!event) { return false; }
	MxEvent_t *mxevent_t = (MxEvent_t*)event;
	pthread_mutex_lock(&mxevent_t->mutex);
	if (mxevent_t->active){
		pthread_mutex_unlock(&mxevent_t->mutex);
		return true;
	}

	bool ret = false;
	if (mxevent_t->broadcast) {
		ret = pthread_cond_broadcast(&mxevent_t->cond) == 0;
	}
	else {
		ret = pthread_cond_signal(&mxevent_t->cond) == 0;
	}
	if (ret) {
		mxevent_t->active = true;
	}
	pthread_mutex_unlock(&mxevent_t->mutex);
	return ret;
}

bool mxInactiveEvent(MxEvent event) {
	if (!event) { return false; }
	MxEvent_t *mxevent_t = (MxEvent_t*)event;
	pthread_mutex_lock(&mxevent_t->mutex);
	mxevent_t->active = false;
	pthread_mutex_unlock(&mxevent_t->mutex);
	return true;
}

unsigned long mxWaitObject(MxEvent event, unsigned long dwMilliseconds) {
	if (!event) { return -1; }
	MxEvent_t *mxevent_t = (MxEvent_t*)event;
	pthread_mutex_lock(&mxevent_t->mutex);
	if (mxevent_t->active){
		if (!mxevent_t->broadcast){
			mxevent_t->active = false;
		}
		pthread_mutex_unlock(&mxevent_t->mutex);
		return WAIT_OK;
	}
	if (WAIT_INFINITE == dwMilliseconds) {
		while (!mxevent_t->active) {
			int ret = pthread_cond_wait(&mxevent_t->cond, &mxevent_t->mutex);
			if (0 != ret) {
				pthread_mutex_unlock(&mxevent_t->mutex);
				return ret;
			}
		}
		if (!mxevent_t->broadcast) {
			mxevent_t->active = false;
		}
		pthread_mutex_unlock(&mxevent_t->mutex);
		return WAIT_OK;
	} else if (0 == dwMilliseconds) {
		if (mxevent_t->active) {
			if (!mxevent_t->broadcast) {
				mxevent_t->active = false;
			}
			pthread_mutex_unlock(&mxevent_t->mutex);
			return WAIT_OK;
		}
		else {
			pthread_mutex_unlock(&mxevent_t->mutex);
			return WAIT_TIMEOUT;
		}
	}

	dwMilliseconds *= 1000;
	timespec abstime;
	
#ifdef _WIN32
	LARGE_INTEGER now;
	LARGE_INTEGER freq;
	QueryPerformanceCounter(&now);
	QueryPerformanceFrequency(&freq);
	abstime.tv_sec = now.QuadPart / freq.QuadPart;
	abstime.tv_nsec = now.QuadPart - abstime.tv_sec*freq.QuadPart;
#else
	timeval now;
	gettimeofday(&now, 0);
	abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
	abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
#endif

	while (!mxevent_t->active) {
		int ret = pthread_cond_timedwait(&mxevent_t->cond, &mxevent_t->mutex, &abstime);
		if (0 != ret) {
			pthread_mutex_unlock(&mxevent_t->mutex);
			return ret;
		}
	}
	if (!mxevent_t->broadcast) {
		mxevent_t->active = false;
	}
	pthread_mutex_unlock(&mxevent_t->mutex);
	return WAIT_OK;
}

unsigned long mxWaitObjects(unsigned long  nCount, MxEvent *event, unsigned long  dwMilliseconds) {
	
	if (INFINITE == dwMilliseconds) {
		for (unsigned long i = 0; i < nCount; i++) {
			MxEvent_t *mxevent_t = (MxEvent_t*)event[i];
			if (!mxevent_t) { continue; }
			pthread_mutex_lock(&mxevent_t->mutex);
			if (mxevent_t->active) {
				if (!mxevent_t->broadcast) { 
					mxevent_t->active = false; 
				}
				pthread_mutex_unlock(&mxevent_t->mutex);
				continue;
			}
			int ret = 0;
			while (!mxevent_t->active) {
				ret = pthread_cond_wait(&mxevent_t->cond, &mxevent_t->mutex);
				if (0 != ret) {
					pthread_mutex_unlock(&mxevent_t->mutex);
					return ret;
				}
			}
			if (!mxevent_t->broadcast) {
				mxevent_t->active = false;
			}
			pthread_mutex_unlock(&mxevent_t->mutex);
		}
	} else if (0 == dwMilliseconds) {
		for (unsigned long i = 0; i < nCount; i++) {
			MxEvent_t *mxevent_t = (MxEvent_t*)event[i];
			if (!mxevent_t) { continue; }
			pthread_mutex_lock(&mxevent_t->mutex);
			if (mxevent_t->active) {
				if (!mxevent_t->broadcast) {
					mxevent_t->active = false;
				}
				pthread_mutex_unlock(&mxevent_t->mutex);
				continue;
			} else {
				pthread_mutex_unlock(&mxevent_t->mutex);
				return WAIT_TIMEOUT;
			}
		}
	}
	else {
		dwMilliseconds *= 1000;
		timespec abstime;
#ifdef _WIN32
		LARGE_INTEGER now;
		LARGE_INTEGER freq;
		QueryPerformanceCounter(&now);
		QueryPerformanceFrequency(&freq);
		abstime.tv_sec = now.QuadPart / freq.QuadPart;
		abstime.tv_nsec = now.QuadPart - abstime.tv_sec*freq.QuadPart;
#else
		timeval now;
		gettimeofday(&now, 0);
		abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
		abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
#endif
		for (unsigned long i = 0; i < nCount; i++) {
			MxEvent_t *mxevent_t = (MxEvent_t*)event[i];
			if (!mxevent_t) {
				continue;
			}
			pthread_mutex_lock(&mxevent_t->mutex);
			while (!mxevent_t->active) {
				int ret = pthread_cond_timedwait(&mxevent_t->cond, &mxevent_t->mutex, &abstime);
				if (0 != ret) {
					pthread_mutex_unlock(&mxevent_t->mutex);
					return ret;
				}
			}
			if (!mxevent_t->broadcast) {
				mxevent_t->active = false;
			}
			pthread_mutex_unlock(&mxevent_t->mutex);
		}
	}
	return WAIT_OK;
}