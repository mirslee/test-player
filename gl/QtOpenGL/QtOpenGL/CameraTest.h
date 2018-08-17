#pragma once
#include "IPipeLine.h"
class CameraTest :
	public IPipeLine
{
public:
	CameraTest();
	~CameraTest();

	void prepare();
	void flush();
	void release();
private:

};

