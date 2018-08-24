#include "MxSynchronize.hpp"


#ifdef _WIN32

#else

struct MxEvent_t {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	bool bManualReset;
	bool signal;
};

MxEvent mxCreateEvent(const pthread_condattr_t* attr, bool bManualReset, bool bInitialState, const char* pName) {
	MxEvent_t *mxevent_t = new MxEvent_t;
	memset(event, 0, sizeof(MxEvent));
	pthread_condattr_init(&mxevent_t->cond, attr);
	pthread_mutex_init(&mxevent_t->mutex, nullptr);
	mxevent_t->signal = bInitialState;
	mxevent_t->bManualReset = bManualReset；
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
	if (!event) { return false; }
	MxEvent_t *mxevent_t = (MxEvent_t*)event;
	pthread_mutex_lock(&mxevent_t->mutex);
	if (mxevent_t->bManualReset && mxevent_t->signal) {
		pthread_mutex_lock(&mxevent_t->mutex);
		return true;
	}
	bool ret = pthread_cons_broadcast(&mxevent_t->cond) == 0;
	if (ret) {
		mxevent_t->signal = true;
	}
	pthread_mutex_unlock(&mxevent_t->mutex);
	return ret;
}


#endif