//
//  MxLog.cpp
//  MxCore
//
//  Created by sz17112850M01 on 2018/8/27.
//  Copyright © 2018年 lixiaopeng. All rights reserved.
//

#include "stdafx.h"
#include "MxLog.h"

void mx_debug(const char *format, ...) {

    va_list args;
    char message[2048] = {0};

    va_start(args, format);
    vsnprintf(message, 2048, format, args);
    va_end(args);
    
    int len = (int)strlen(message);
    if (message[len - 2] == '\n'&&message[len - 1] == '\n')
        message[len - 1] = 0;
    
#ifdef _WIN32
    OutputDebugString(reinterpret_cast<const wchar_t*>(message.utf16()));
    return;
#endif
    fprintf(stdout, "%s", message);
    fflush(stdout);
}

