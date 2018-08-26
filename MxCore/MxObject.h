#pragma once

class MxObject {
	//
};

class CMxSharedObject: public MxObject {
public:
	int addRef() {
		_refCount++;
		return _refCount;
		/*InterlockedIncrement(&_refCount);
		return max(m_refCount, 1);*/
	}
	int unRef() {
		//int refCount = InterlockedDecrement(&_refCount);
		_refCount--;
		if (_refCount == 0)
		{
			_refCount++;
			delete this;
			//return 0;
		}
		/*else
		{
			max(m_refCount, 1);
		}*/
		return _refCount;
	}
protected:
	volatile int _refCount;
};


