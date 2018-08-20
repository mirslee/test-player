#include "stdafx.h"
#include "threadpool.h"

CVxThreadPool* g_tpool = NULL;

CVxThreadPool::CVxThreadPool(int _maxThreads)
: CVxObject("CVxThreadPool",NULL)
, m_maxThreads(_maxThreads)
, m_counter(0)
, m_idle(0)
, m_quit(false)
{
	VXTHREAD_MUTEX_INIT(&m_csLock);
	m_threads = new pthread_t[_maxThreads];
	VXSEM_INIT(&m_sem, 0, _maxThreads);
	m_taskQueue.SetMaxSize(_maxThreads);
}
// 线程入口
// 这其实就相当于一个消费者线程, 不断的消费任务(执行任务)

void CVxThreadPool::thread_routine()
{
     while (true)
    {
		task_t t;
		if (!m_taskQueue.Pop(&t))
		{
			{
				CVxLock lock(&m_csLock);
				m_idle++;
			}
			VXSEM_WAIT(m_sem);
			if (m_quit) break;
			CVxLock lock(&m_csLock);
			m_idle--;

			if (!m_taskQueue.Pop(&t))
				continue;
		}
  		//处理任务
        t.run(t.args);
		if(t.finish) vxSetEvent(t.finish);
	}
}

//添加任务函数, 类似于一个生产者, 不断的将任务生成, 挂接到任务队列上, 等待消费者线程进行消费
VXBOOL CVxThreadPool::AddTask(TPRUNFUNC run, void *args, VXEVENT finish)
{
	CVxLock lock(&m_csLock); //注意需要使用互斥量保护共享变量
	/** 1. 生成任务并将任务添加到"任务队列"队尾 **/
	task_t newTask = { run, args, finish };
	if (!m_taskQueue.Push(newTask))
		return FALSE;
    /** 2. 让线程开始执行任务 **/
    startTask();
	return TRUE;
}

//线程启动函数
void CVxThreadPool::startTask()
{
    // 如果有等待线程, 则唤醒其中一个, 让它来执行任务
	if (m_idle > 0)
	{
		VXSEM_POST(m_sem);
	}
	else if (m_counter < m_maxThreads) // 没有等待线程, 而且当前先线程总数尚未达到阈值, 我们就需要创建一个新的线程
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        int max_priority = sched_get_priority_max(SCHED_RR);;
        struct sched_param param = {0};
        pthread_attr_getschedparam(&attr,&param);
        param.sched_priority = (max_priority+param.sched_priority)/2;
        pthread_attr_setschedparam(&attr,&param);
        pthread_create(&m_threads[m_counter], &attr, ThreadPoolFunc, this);
        pthread_attr_destroy(&attr);
		m_counter++;
    }
}

CVxThreadPool::~CVxThreadPool()
{
	VXSEM_DESTROY(m_sem);
	delete[] m_threads;
	VXTHREAD_MUTEX_DESTROY(&m_csLock);
	g_tpool = NULL;
}


void CVxThreadPool::OnDelete()
{
	CVxLock lock(&m_csLock);
	m_quit = true;
	if (m_counter > 0)
	{
		//对于处于等待状态, 则给他们发送通知,
		//这些处于等待状态的线程, 则会接收到通知,
		//然后直接退出
		for (int i = 0; i < m_idle; i++)
			VXSEM_POST(m_sem);

		//对于正处于执行任务的线程, 他们接收不到这些通知,
		//则需要等待他们执行完任务
		for (int i = 0; i < m_counter; i++)
			pthread_join(m_threads[i], NULL);
	}
}


VX_EXT_API VXRESULT vxGetThreadPool(int _maxThreads, IVxThreadPool** tpool)
{
	if (!g_tpool)
		g_tpool = new CVxThreadPool(_maxThreads);
	return GetVxInterface(static_cast<IVxThreadPool*>(g_tpool), (void**)tpool);
}