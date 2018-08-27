//
//  MxObject.cpp
//  MxCore
//
//  Created by sz17112850M01 on 2018/8/27.
//  Copyright © 2018年 lixiaopeng. All rights reserved.
//

#include "stdafx.h"
#include "MxObject.h"


int CMxObject::addRefDelgate() {
    _refCount++;
    return _refCount;
}
int CMxObject::unRefDelgate() {
    _refCount--;
    if (_refCount == 0)
    {
        _refCount++;
        delete this;
        return 0;
    }
    return _refCount;
}
int CMxObject::queryInterfaceDelgate(int iid, void** ppVoid) {
    if (IID_CMxObject == iid) {
        *ppVoid = this;
    } else {
        *ppVoid = nullptr;
    }
    return 0;
}
