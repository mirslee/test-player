#ifndef MxThread_h
#define MxThread_h

class MXCORE_API MxThread {
    
public:
    MxThread();
    void start();
    virtual void run();
    
    void join(MxThread* thread);
    void exit();
    
public:
    pthread_t thread;
    
private:
    static void* _run(void* pThis);
};


extern MXGLTOOL_API int nMxGLTool;

#endif /* MxThread_hpp */
