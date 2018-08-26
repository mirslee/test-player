//
//  VideoDecoder.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/14.
//

#include "stdafx.h"
#include "VideoDecoder.h"

bool VideoDecoder::openFile(string path) {
    
    if (path == "") {
        return false;
    }
    
    avformat_network_init();
    av_register_all();
    
	return true;
}
