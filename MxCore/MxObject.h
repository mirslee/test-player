#pragma once

class MxObject {

};

class MxSharedObject: public MxObject {
	virtual int addRef() = 0;
	virtual int unRef() = 0;
};


