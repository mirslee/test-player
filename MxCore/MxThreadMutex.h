#ifndef MxMutex_hpp
#define MxMutex_hpp

class MXCORE_API MxThreadMutex {
    
public:
    MxThreadMutex();
    ~MxThreadMutex();
    
    void lock();
    void unlock();
    
public:
    pthread_mutex_t mutex;
};

#endif /* MxMutex_h */
