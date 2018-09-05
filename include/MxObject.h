#pragma once

#include "MxGlobal.h"
#include "MxError.h"
#include "MxInterface.h"

struct MxObject {
    virtual long addRef() = 0;
    virtual long unRef() = 0;
    virtual long queryInterface(long iid, void** ppVoid) = 0;
};

class CMxObject {
public:
    CMxObject() {_refCount = 0;}
    virtual ~CMxObject() {}
    virtual long addRefDelgate() {
        _refCount++;
        return _refCount;
    }
    virtual long unRefDelgate() {
        _refCount--;
        if (_refCount == 0)
        {
            _refCount++;
            delete this;
            return 0;
        }
        return _refCount;
    }
    virtual long queryInterfaceDelgate(long iid, void** ppVoid) {
        if (IID_CMxObject == iid) {
            *ppVoid = this;
        } else {
            *ppVoid = nullptr;
        }
        return 0;
    }
protected:
    volatile long _refCount;
};

#define MX_OBJECT \
public: \
long addRef() {return this->addRefDelgate();} \
long unRef() {return this->unRefDelgate();}\
long queryInterface(long iid, void** ppVoid) {return this->queryInterfaceDelgate(iid,ppVoid);}


mxinline long mxGetInterface(MxObject* pObj, void **ppVoid)
{
	*ppVoid = pObj;
	pObj->addRef();
	return MX_NOERROR;
}