//
//  stdafx.h
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/14.
//
#include <string>
#include <vector>
#include <queue>
#include <iostream>
using namespace std;

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QMacCocoaViewContainer>

#ifndef stdafx_h
#define stdafx_h

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
    
#ifdef __cplusplus
}
#endif



#endif /* stdafx_h */
