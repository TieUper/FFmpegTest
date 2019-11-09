//
// Created by Administrator on 2019/11/6.
//

#ifndef FFMPEGTEST_AUDIOCHANNEL_H
#define FFMPEGTEST_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int i, AVCodecContext *pContext);

    ~AudioChannel();

    void start();

    void stop();
};


#endif //FFMPEGTEST_AUDIOCHANNEL_H
