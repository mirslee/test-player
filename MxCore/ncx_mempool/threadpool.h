#ifndef __BAY_THREADPOOL_H__
#define __BAY_THREADPOOL_H__

#include "vxbase.h"
#include "vxtempl.h"

void *thread_routine(void *args);

class CVxThreadPool : public CVxObject,public IVxThreadPool
{
    friend void *thread_routine(void *args);
private:
	struct task_t{		//任务结构体
		TPRUNFUNC run; //任务回调函数
		void *args;     //任务函数参数
		VXEVENT finish;	//任务结束通知
	};
public:
	CVxThreadPool(int _maxThreads = 36);
	virtual ~CVxThreadPool();
public:
	DECLARE_IVXOBJECT
    //添加任务接口
	VXBOOL AddTask(TPRUNFUNC run, void *args, VXEVENT finish);
protected:
	void OnDelete();
private:
    void startTask();

private:
	//任务准备就绪或线程池销毁通知
	VXSEM_T m_sem;
	VXTHREAD_MUTEX m_csLock;
	static void *ThreadPoolFunc(void *args){ _vxSetThreadName("线程池线程");((CVxThreadPool *)args)->thread_routine(); return NULL; }
	void thread_routine();
    CVxQueue<task_t>	 m_taskQueue; //任务队列
	pthread_t* m_threads;
    int m_maxThreads;        //线程池最多允许的线程数
	int m_counter;           //线程池当前线程数
	int m_idle;              //线程池空闲线程数
	bool m_quit;              //线程池销毁标志
};

#endif // __BAY_THREADPOOL_H__
