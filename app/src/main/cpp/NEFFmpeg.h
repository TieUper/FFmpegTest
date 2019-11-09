//
// Created by Administrator on 2019/11/6.
//

#ifndef FFMPEGTEST_NEFFMPEG_H
#define FFMPEGTEST_NEFFMPEG_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "macro.h"
#include <pthread.h>
#include <cstring>

extern "C"{
#include <libavformat/avformat.h>
};

class NEFFmpeg {
public:
    NEFFmpeg(JavaCallHelper *javaCallHelper, char *dataSource);

    ~NEFFmpeg();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void setRenderCallback(RenderCallback renderCallback);

private:
    JavaCallHelper *javaCallHelper = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    char *dataSource;
    bool isPlaying;
    pthread_t pid_prepare;
    pthread_t pid_start;
    AVFormatContext *formatContext = 0;
    RenderCallback renderCallback;
};


#endif //FFMPEGTEST_NEFFMPEG_H
