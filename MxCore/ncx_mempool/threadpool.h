#ifndef __BAY_THREADPOOL_H__
#define __BAY_THREADPOOL_H__

#include "vxbase.h"
#include "vxtempl.h"

void *thread_routine(void *args);

class CVxThreadPool : public CVxObject,public IVxThreadPool
{
    friend void *thread_routine(void *args);
private:
	struct task_t{		//����ṹ��
		TPRUNFUNC run; //����ص�����
		void *args;     //����������
		VXEVENT finish;	//�������֪ͨ
	};
public:
	CVxThreadPool(int _maxThreads = 36);
	virtual ~CVxThreadPool();
public:
	DECLARE_IVXOBJECT
    //�������ӿ�
	VXBOOL AddTask(TPRUNFUNC run, void *args, VXEVENT finish);
protected:
	void OnDelete();
private:
    void startTask();

private:
	//����׼���������̳߳�����֪ͨ
	VXSEM_T m_sem;
	VXTHREAD_MUTEX m_csLock;
	static void *ThreadPoolFunc(void *args){ _vxSetThreadName("�̳߳��߳�");((CVxThreadPool *)args)->thread_routine(); return NULL; }
	void thread_routine();
    CVxQueue<task_t>	 m_taskQueue; //�������
	pthread_t* m_threads;
    int m_maxThreads;        //�̳߳����������߳���
	int m_counter;           //�̳߳ص�ǰ�߳���
	int m_idle;              //�̳߳ؿ����߳���
	bool m_quit;              //�̳߳����ٱ�־
};

#endif // __BAY_THREADPOOL_H__
