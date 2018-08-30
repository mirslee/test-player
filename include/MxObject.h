#pragma once

#include "MxInterface.h"

struct MxObject {
    virtual int addRef() = 0;
    virtual int unRef() = 0;
    virtual int queryInterface(int iid, void** ppVoid) = 0;
};

class CMxObject {
public:
    CMxObject() {_refCount = 0;}
    virtual ~CMxObject() {}
    virtual int addRefDelgate() {
        _refCount++;
        return _refCount;
    }
    virtual int unRefDelgate() {
        _refCount--;
        if (_refCount == 0)
        {
            _refCount++;
            delete this;
            return 0;
        }
        return _refCount;
    }
    virtual int queryInterfaceDelgate(int iid, void** ppVoid) {
        if (IID_CMxObject == iid) {
            *ppVoid = this;
        } else {
            *ppVoid = nullptr;
        }
        return 0;
    }
protected:
    volatile int _refCount;
};

#define MX_OBJECT \
public: \
int addRef() {return this->addRefDelgate();} \
int unRef() {return this->unRefDelgate();}\
int queryInterface(int iid, void** ppVoid) {return this->queryInterfaceDelgate(iid,ppVoid);}


