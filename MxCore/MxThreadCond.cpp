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
