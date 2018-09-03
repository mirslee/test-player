#ifndef __CMXQUEUE_H__
#define __CMXQUEUE_H__

#include "MxSynchronize.h"

typedef int(*compareq)(void* t0, void* T1);
template <class T>
class CMxQueue
{
public:
	CMxQueue(int dwWaitTime = 10000);
	CMxQueue(compareq comp, int dwWaitTime = 10000);
	virtual ~CMxQueue();
protected:
	compareq compare;
	T*	m_pData;
	int m_maxSize;
	int m_prerollSize;
	int m_nFront;
	int m_nLast;
	int m_nSize;

	int m_dwWaitTime;
	CMxEvent m_hDataReady;
	CMxEvent m_hDataFull;
	bool m_bWait;
	CMxMutex m_ptmutex;
	bool m_bModify;
public:
	void SetWaitTime(int dwWaitTime) { m_dwWaitTime = dwWaitTime; }
	void SetWaitTime(compareq comp, int dwWaitTime) { compare = comp; m_dwWaitTime = dwWaitTime; }
	void SetMaxSize(int size);
	int  GetMaxSize() { return m_maxSize; };
	int  GetSize() { return m_nSize; };

	void SetPrerollSize(int size) { m_prerollSize = size; }
	void Clear();

	bool Push(T& pObject);
	bool PushSort(T& pObject);
	bool Pop(T* pObject, bool bWait = false);
	bool Pop(T* pObject, int waittime);
	bool Pop();
	bool GetFront(T* pObject, bool bWait = false);
	bool GetFront(T* pObject, int waittime);
	bool GetLast(T* pObject, bool bClear = true);
	bool PopLast();

	bool IsEmpty() { return 	(m_nSize == 0); }
	bool IsFull() { return (m_nSize == m_maxSize) ? true : false; }
	CMxEvent GetFullEvent() { return m_hDataFull; }

	CMxMutex* GetMutex() { return &m_ptmutex; }
	bool TryLock() { return mxMutexTrylock(&m_ptmutex); }
	void Lock() { mxMutexLock(&m_ptmutex); }
	void Unlock() { mxMutexUnlock(&m_ptmutex); }
	int GetTotalData(T* p);
	T* GetDataPtr() { return m_pData; }
	int GetHeadPosition();
	T* GetHead();
	T* GetNext(int& next);
	void RemoveAt(int pos);

	bool IsModify() { bool ret = m_bModify; m_bModify = false; return ret; }
};

template <class T>
CMxQueue<T>::CMxQueue(int dwWait)
{
	m_dwWaitTime = dwWait;
	m_hDataReady = mxCreateEvent(nullptr, true, false, nullptr);
	m_hDataFull = mxCreateEvent(nullptr, true, false, nullptr);
	mxMutexInit(&m_ptmutex);
	m_bWait = false;
	m_prerollSize = 0;
	m_maxSize = 0;
	m_nFront = m_nLast = 0;
	m_pData = nullptr;
	m_nSize = 0;
	m_bModify = false;
	compare = nullptr;
}

template <class T>
CMxQueue<T>::CMxQueue(compareq comp, int dwWait)
{
	compare = comp;
	m_dwWaitTime = dwWait;
	m_hDataReady = mxCreateEvent(nullptr, true, false, nullptr);
	m_hDataFull = mxCreateEvent(nullptr, true, false, nullptr);
	mxMutexInit(&m_ptmutex);
	m_bWait = false;
	m_prerollSize = 0;
	m_maxSize = 0;
	m_nFront = m_nLast = 0;
	m_pData = nullptr;
	m_nSize = 0;
	m_bModify = false;
}

template <class T>
CMxQueue<T>::~CMxQueue()
{
	mxCloseEvent(m_hDataReady);
	mxCloseEvent(m_hDataFull);
	if (m_pData)
	{
		for (int i = 0; i < m_maxSize; i++)
		{
			(m_pData + i)->~T();
		}
		_vxfree(m_pData);
	}
	mxMutexDestroy(&m_ptmutex);
}

template <class T>
void CMxQueue<T>::SetMaxSize(int size)
{
	if (m_maxSize < size)
	{
		if (m_pData)
		{
			for (int i = 0; i < m_maxSize; i++)
				(m_pData + i)->~T();
			_vxfree(m_pData);
		}
		m_pData = (T*)_vxmallocz(sizeof(T)*size);
		for (int i = 0; i < size; i++)
		{
#ifdef _WIN32			
#pragma push_macro("new")
#undef new
			::new((void*)(m_pData + i)) T;
#pragma pop_macro("new")
#else
			::new((void*)(m_pData + i)) T;
#endif
		}
	}
	m_maxSize = size;
}

template <class T>
void CMxQueue<T>::Clear()
{
	CMxMutexLocker locker(&m_ptmutex);
	m_nFront = m_nLast = 0;
	m_nSize = 0;
	mxResetEvent(m_hDataFull);
	mxResetEvent(m_hDataReady);
}

template <class T>
bool CMxQueue<T>::Push(T& object)
{
	CMxMutexLocker locker(&m_ptmutex);
	if (IsFull())
	{
		return false;
	}

	m_pData[m_nLast++] = object;
	if (m_nLast == m_maxSize)
	{
		m_nLast = 0;
	}
	m_nSize++;

	m_bModify = true;
	if (m_bWait)
	{
		mxSetEvent(m_hDataReady);
		m_bWait = false;
	}

	if (m_prerollSize && (m_nSize >= m_prerollSize) && (mxWaitEvent(m_hDataFull, 0) != WAIT_OK))
	{
		mxSetEvent(m_hDataFull);
	}
	return true;
}

template <class T>
bool CMxQueue<T>::PushSort(T& object)
{
	CMxMutexLocker locker(&m_ptmutex);
	if (IsFull())
	{
		return false;
	}

	if (m_nLast > m_nFront)
	{
		bool bAppend = false;
		for (int i = m_nLast - 1; i >= m_nFront; i--)
		{
			if (compare(&object, &m_pData[i]) > 0)
			{
				if (i == (m_nLast - 1))
				{
					m_pData[m_nLast] = object;
				}
				else
				{
					memmove(m_pData + i + 2, m_pData + i + 1, (m_nLast - (i + 1)) * sizeof(T));
					m_pData[i + 1] = object;
				}
				bAppend = TRUE;
				break;
			}
		}
		if (!bAppend)
		{
			memmove(m_pData + m_nFront + 1, m_pData + m_nFront, (m_nLast - m_nFront) * sizeof(T));
			m_pData[m_nFront] = object;
		}
	}
	else if (m_nLast < m_nFront)
	{
		bool bAppend = false;
		for (int i = m_nLast - 1; i >= 0; i--)
		{
			if (compare(&object, &m_pData[i])>0)
			{
				if (i == (m_nLast - 1))
				{
					m_pData[m_nLast] = object;
				}
				else
				{
					memmove(m_pData + i + 2, m_pData + i + 1, (m_nLast - (i + 1)) * sizeof(T));
					m_pData[i + 1] = object;
				}
				bAppend = TRUE;
				break;
			}
		}
		if (!bAppend)
		{
			for (int i = m_maxSize - 1; i >= m_nFront; i--)
			{
				if (compare(&object, &m_pData[i]) > 0)
				{
					memmove(m_pData + 1, m_pData, m_nLast * sizeof(T));
					if (i == (m_maxSize - 1))
					{
						m_pData[0] = object;
					}
					else
					{
						m_pData[0] = m_pData[m_maxSize - 1];
						memmove(m_pData + i + 2, m_pData + i + 1, (m_maxSize - 1 - (i + 1)) * sizeof(T));
						m_pData[i + 1] = object;
					}
					bAppend = TRUE;
					break;
				}
			}
			if (!bAppend)
			{
				memmove(m_pData + 1, m_pData, m_nLast * sizeof(T));
				m_pData[0] = m_pData[m_maxSize - 1];
				memmove(m_pData + m_nFront + 1, m_pData + m_nFront, (m_maxSize - 1 - m_nFront) * sizeof(T));
				m_pData[m_nFront] = object;
			}
		}
	}
	else
	{
		m_pData[m_nFront] = object;
	}

	m_nLast++;
	if (m_nLast == m_maxSize)
	{
		m_nLast = 0;
	}
	m_nSize++;

	m_bModify = true;
	if (m_bWait)
	{
		mxSetEvent(m_hDataReady);
		m_bWait = false;
	}
	if (m_prerollSize && (m_nSize >= m_prerollSize) && (mxWaitEvent(m_hDataFull, 0) != WAIT_OK))
	{
		mxSetEvent(m_hDataFull);
	}
	return true;
}

template <class T>
bool CMxQueue<T>::Pop(T* pObject, bool bWait)
{
	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize > 0)
		{
			*pObject = m_pData[m_nFront++];
			if (m_nFront == m_maxSize)
			{
				m_nFront = 0;
			}
			m_nSize--;
			if (!m_nSize)
			{
				mxResetEvent(m_hDataReady);
			}
			return true;
		}
		else
		{
			if (bWait)
			{
				m_bWait = true;
			}
			else
			{
				return false;
			}
		}
	}

	//exit the lock
	int dwObject = mxWaitEvent(m_hDataReady, m_dwWaitTime); //wait for 10 seconds
	if (dwObject == WAIT_TIMEOUT) return false;
	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize == 0)
		{
			return false;
		}
		*pObject = m_pData[m_nFront++];
		if (m_nFront == m_maxSize)
		{
			m_nFront = 0;
		}
		m_nSize--;
		if (!m_nSize)
		{
			mxResetEvent(m_hDataReady);
		}
		return true;
	}
}

template <class T>
bool CMxQueue<T>::Pop(T* pObject, int waittime)
{
	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize > 0)
		{
			*pObject = m_pData[m_nFront++];
			if (m_nFront == m_maxSize)
			{
				m_nFront = 0;
			}
			m_nSize--;
			if (!m_nSize)
			{
				mxResetEvent(m_hDataReady);
			}
			return true;
		}
		else
		{
			m_bWait = true;
		}
	}

	//exit the lock
	int dwObject = mxWaitEvent(m_hDataReady, waittime); //wait for 10 seconds
	if (dwObject == WAIT_TIMEOUT)
	{
		return false;
	}

	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize == 0)
		{
			return false;
		}
		*pObject = m_pData[m_nFront++];
		if (m_nFront == m_maxSize)
		{
			m_nFront = 0;
		}
		m_nSize--;
		if (!m_nSize)
		{
			mxResetEvent(m_hDataReady);
		}
		return true;
	}
}

template <class T>
bool CMxQueue<T>::Pop()
{
	CMxMutexLocker locker(&m_ptmutex);
	if (IsEmpty())
	{
		return false;
	}
	m_nFront++;
	if (m_nFront == m_maxSize)
	{
		m_nFront = 0;
	}
	m_nSize--;
	if (!m_nSize)
	{
		mxResetEvent(m_hDataReady);
	}
	return true;
}

template <class T>
bool CMxQueue<T>::GetFront(T* pObject, bool bWait)
{
	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize > 0)
		{
			*pObject = m_pData[m_nFront];
			return true;
		}

		if (bWait)
		{
			m_bWait = true;
		}
		else
		{
			return false;
		}
	}
	//exit the lock
	if (mxWaitEvent(m_hDataReady, m_dwWaitTime) == WAIT_TIMEOUT)
	{
		return false;
	}

	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize == 0)
		{
			return false;
		}
		*pObject = m_pData[m_nFront];
		return true;
	}
}

template <class T>
bool CMxQueue<T>::GetFront(T* pObject, int waittime)
{
	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize > 0)
		{
			*pObject = m_pData[m_nFront];
			return true;
		}
		else
		{
			m_bWait = true;
		}
	}
	//exit the lock
	if (mxWaitEvent(m_hDataReady, waittime) == WAIT_TIMEOUT)
	{
		return false;
	}

	{
		CMxMutexLocker locker(&m_ptmutex);
		if (m_nSize == 0)
		{
			return false;
		}
		*pObject = m_pData[m_nFront];
		return true;
	}
}

template <class T>
bool CMxQueue<T>::GetLast(T* pObject, bool bClear)
{
	CMxMutexLocker locker(&m_ptmutex);
	if (m_nSize > 0)
	{
		int nLast;
		if (m_nLast == 0)
		{
			nLast = m_maxSize - 1;
		}
		else
		{
			nLast = m_nLast - 1;
		}
		*pObject = m_pData[nLast];
		if (bClear)
		{
			m_nFront = m_nLast = 0;
			m_nSize = 0;
			mxResetEvent(m_hDataFull);
			mxResetEvent(m_hDataReady);
		}
		return true;
	}
	return false;
}

template <class T>
bool CMxQueue<T>::PopLast()
{
	CMxMutexLocker locker(&m_ptmutex);
	if (IsEmpty())
	{
		return false;
	}
	m_nLast--;
	if (m_nLast < 0)
	{
		m_nLast = m_maxSize - 1;
	}
	m_nSize--;
	if (!m_nSize) mxResetEvent(m_hDataReady);
	return true;
}

template <class T>
int CMxQueue<T>::GetTotalData(T* p)
{
	CMxMutexLocker locker(&m_ptmutex);
	if (m_nSize == 0) return m_nSize;
	if (m_nFront >= m_nLast)
	{
		memcpy(p, m_pData + m_nFront, sizeof(T)*(m_maxSize - m_nFront));
		memcpy(p + (m_maxSize - m_nFront), m_pData, sizeof(T)*m_nLast);
	}
	else
	{
		memcpy(p, m_pData + m_nFront, sizeof(T)*m_nSize);
	}
	return m_nSize;
}

template <class T>
int CMxQueue<T>::GetHeadPosition()
{
	return (m_nSize == 0) ? -1 : 0;
}

template <class T>
T* CMxQueue<T>::GetHead()
{
	if (m_nSize == 0) return nullptr;
	return &m_pData[m_nFront];
}

template <class T>
T* CMxQueue<T>::GetNext(int& next)
{
	if (next >= m_nSize)
	{
		return nullptr;
	}
	T* ret = &m_pData[(next + m_nFront) % m_maxSize];
	next = (next < (m_nSize - 1)) ? (next + 1) : -1;
	return ret;
}

template <class T>
void CMxQueue<T>::RemoveAt(int pos)
{
	if (pos < 0 || pos >= m_nSize)
	{
		return;
	}

	int del = (pos + m_nFront) % m_maxSize;
	if (del < m_nLast)
	{
		memmove(m_pData + del, m_pData + del + 1, (m_nLast - (del + 1)) * sizeof(T));
		m_nLast--;
	}
	else
	{
		memmove(m_pData + del, m_pData + (del + 1) % m_maxSize, (m_maxSize - (del + 1)) * sizeof(T));
		if (m_nLast > 0)
		{
			memcpy(m_pData + (m_maxSize - 1), m_pData, sizeof(T));
			memmove(m_pData, m_pData + 1, (m_nLast - 1) * sizeof(T));
			m_nLast--;
		}
		else
		{
			m_nLast = m_maxSize - 1;
		}
	}
	m_nSize--;
}

#endif //__CMXQUEUE_H__