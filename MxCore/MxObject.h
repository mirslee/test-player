#pragma once

struct MxObject {
    virtual int addRef() = 0;
    virtual int unRef() = 0;
    virtual int queryInterface(int iid, void** ppVoid) = 0;
};

class CMxObject {
public:
    int addRefDelgate();
    int unRefDelgate();
    int queryInterfaceDelgate(int iid, void** ppVoid);
protected:
    volatile int _refCount;
    
};

#define MX_OBJECT \
public: \
int addRef() {return this->addRefDelgate();} \
int unRef() {return this->unRefDelgate();}\
int queryInterface(int iid, void** ppVoid) {return this->queryInterfaceDelgate(iid,ppVoid);}
    

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
    int queryInterface(int iid, void** ppVoid) {
        return 0;
    }
protected:
	volatile int _refCount;
};


