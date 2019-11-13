//
// Created by Administrator on 2019/11/6.
//

#ifndef FFMPEGTEST_AUDIOCHANNEL_H
#define FFMPEGTEST_AUDIOCHANNEL_H


#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
extern "C"{
#include <libswresample/swresample.h>
};

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int i, AVCodecContext *pContext);

    ~AudioChannel();

    void start();

    void stop();

    bool isPlaying;

    void audio_decode();

    void audio_play();

    int getPCM();

    uint8_t *out_buffers = 0;
    //通道数
    int out_channels = 0;
    int out_sample_size = 0;
    int out_sample_rate = 0;
    int out_buffers_size = 0;

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    //引擎
    SLObjectItf engineObject = 0;
    //引擎接口
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerInterface = 0;
    //播放器队列
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

    SwrContext *swrContext;
};


#endif //FFMPEGTEST_AUDIOCHANNEL_H
