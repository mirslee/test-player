//
//  main.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/25.
//

#include "MxCore/MxSynchronize.h"

MxEvent event = nullptr;


int ticketcount = 10000;
void* thread1(void*) {

    /*if (WAIT_OBJECT_0 == mxWaitObject(event, INFINITE)) {
        std::cout << "thread1:" << ticketcount;
    }*/
    MxEvent_t *mxevent_t = (MxEvent_t*)event;
    pthread_mutex_lock(&mxevent_t->mutex);
    int ret = pthread_cond_wait(&mxevent_t->cond, &mxevent_t->mutex);
    pthread_mutex_unlock(&mxevent_t->mutex);
}

void* thread2(void*) {
    /*if (WAIT_OBJECT_0 == mxWaitObject(event, INFINITE)) {
        std::cout << "thread2:" << ticketcount;
    }*/
    MxEvent_t *mxevent_t = (MxEvent_t*)event;
    pthread_mutex_lock(&mxevent_t->mutex);
    int ret = pthread_cond_wait(&mxevent_t->cond, &mxevent_t->mutex);
    pthread_mutex_unlock(&mxevent_t->mutex);
}

int main(int argc, char** argv) {
    
    
    event = mxCreateEvent(nullptr, false, false, "111");
    pthread_t t1;
    pthread_t t2;
    pthread_create(&t1, nullptr, thread1, nullptr);
    pthread_create(&t2, nullptr, thread2, nullptr);

    mxSleep(3000);
    mxSetEvent(event);
    mxSleep(3000);
    mxSetEvent(event);
    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);
    return 0;
}

/*#include <pthread.h>
#include <iostream>
#include <unistd.h>


using namespace std;

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond;

struct MXEVENT {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    bool bManualReset;
    bool signal;
};

MxEvent eee = mxCreateEvent(NULL, false, false, "111");
static void* func_1(void* arg)
{
    cout << "func_1 start" << endl;
    
    pthread_mutex_lock(&((MXEVENT*)eee)->mutex);
    cout << "func_1 lock mtx" << endl;
    
    cout << "func_1 wait cond" << endl;
    int ret = pthread_cond_wait(&((MXEVENT*)eee)->cond, &((MXEVENT*)eee)->mutex);
    
    cout << "func_1 unlock mtx" << endl;
    pthread_mutex_unlock(&((MXEVENT*)eee)->mutex);
    
    cout << "func_1 end" << endl;
    sleep(5);
    
    return NULL;
}

static void* func_2(void* arg)
{
    cout << "func_2 start" << endl;
    
    pthread_mutex_lock(&((MXEVENT*)eee)->mutex);
    cout << "func_2 lock mtx" << endl;
    
    cout << "func_2 wait cond" << endl;
    int ret =  pthread_cond_wait(&((MXEVENT*)eee)->cond, &((MXEVENT*)eee)->mutex);
    
    cout << "func_2 unlock mtx" << endl;
    pthread_mutex_unlock(&((MXEVENT*)eee)->mutex);
    
    cout << "func_2 end" << endl;
    sleep(5);
    
    return NULL;
}


int main()
{
    pthread_t tid1, tid2;
    pthread_cond_init(&((MXEVENT*)eee)->cond, NULL);
    pthread_mutex_init(&((MXEVENT*)eee)->mutex, NULL);
    
    cout << "main create thread" << endl;
    pthread_create(&tid1, NULL, func_1, NULL);
    pthread_create(&tid2, NULL, func_2, NULL);
    
    sleep(3);
    cout << "main boradcast signal" << endl;
    pthread_mutex_lock(&((MXEVENT*)eee)->mutex);
    pthread_cond_broadcast(&((MXEVENT*)eee)->cond);
    pthread_mutex_unlock(&((MXEVENT*)eee)->mutex);
    
    cout << "main join thread" << endl;
    
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    cout << "main end" << endl;
    return 0;
}*/

