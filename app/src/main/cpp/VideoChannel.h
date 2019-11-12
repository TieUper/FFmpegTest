//
// Created by Administrator on 2019/11/6.
//

#ifndef FFMPEGTEST_VIDEOCHANNEL_H
#define FFMPEGTEST_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "macro.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

typedef void (*RenderCallback)(uint8_t *, int, int, int);
class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id, AVCodecContext *avCodecContext, int fps);

    ~VideoChannel();

    void start();

    void stop();

    bool isPlaying;

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback callback);

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback  renderCallback;
    int fps;
};


#endif //FFMPEGTEST_VIDEOCHANNEL_H
