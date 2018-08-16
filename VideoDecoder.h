//
//  VideoDecoder.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/14.
//

#ifndef VideoDecoder_hpp
#define VideoDecoder_hpp

#include <stdio.h>

enum FrameType {
    AudioFrameType,
    VideoFrame,
};

class BuriedPoint {
public:
    long long beginOpen;              // 开始试图去打开一个直播流的绝对时间
    float successOpen;                // 成功打开流花费时间
    float firstScreenTimeMills;       // 首屏时间
    float failOpen;                   // 流打开失败花费时间
    float failOpenType;               // 流打开失败类型
    int retryTimes;                   // 打开流重试次数
    float duration;                   // 拉流时长
    //NSMutableArray* bufferStatusRecords; // 拉流状态
};

class Frame {
public:
    FrameType type;
    float position;
    float duration;
};

class VideoDecoder {
public:
    AVFormatContext*            _formatCtx;
    bool                        _isOpenInputSuccess;
    
    BuriedPoint*                _buriedPoint;
    
    int                         totalVideoFramecount;
    long long                   decodeVideoFrameWasteTimeMills;
    
    vector<int>                    _videoStreams;
    vector<int>                       _audioStreams;
    int                   _videoStreamIndex;
    int                   _audioStreamIndex;
    AVCodecContext*             _videoCodecCtx;
    AVCodecContext*             _audioCodecCtx;
    float                     _videoTimeBase;
    float                     _audioTimeBase;
    
    
    AVFrame*                    _videoFrame;
    AVFrame*                    _audioFrame;
    
    float                     _fps;
    
    float                     _decodePosition;
    
    bool                        _isSubscribe;
    bool                        _isEOF;
    
    SwrContext*                 _swrContext;
    void*                       _swrBuffer;
    int                  _swrBufferSize;
    
    AVPicture                   _picture;
    bool                        _pictureValid;
    struct SwsContext*          _swsContext;
    
    int                         _subscribeTimeOutTimeInSecs;
    int                         _readLastestFrameTime;
    
    bool                        _interrupted;
    
    int                         _connectionRetry;
    
    bool openFile(string path);
    
};

#endif /* VideoDecoder_hpp */
