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
