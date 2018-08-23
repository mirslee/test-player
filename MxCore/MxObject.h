#pragma once

class MxObject {

};

class MxSharedObject {
	virtual int addRef() = 0;
	virtual int unRef() = 0;
};