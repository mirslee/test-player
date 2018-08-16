//
//  MxQueue.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//

#ifndef MxQueue_h
#define MxQueue_h

template <class T>
class MxQueue  {
    
public:
    MxQueue() {
        pthread_mutex_init(&mutex, nullptr);
    }
    
    void push(T & value) {
        pthread_mutex_lock(&mutex);
        {
            stdQueue.push(value);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    int pop(T* pValue) {
        int ret = -1;
        pthread_mutex_lock(&mutex);
        {
            if (!stdQueue.empty()) {
                *pValue = stdQueue.front();
                stdQueue.pop();
                ret = 0;
            }
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }
    
private:
    std::queue<T>  stdQueue;
    pthread_mutex_t mutex;
};


#endif /* MxQueue_hpp */
