//
// Created by Administrator on 2019/11/6.
//

#ifndef FFMPEGTEST_BASECHANNEL_H
#define FFMPEGTEST_BASECHANNEL_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
};

#include "safe_queue.h"
/**
 *  VideoChannel 和 AudioChannel的父类   析构函数必须带 virtual
 */
class BaseChannel {

public:
    BaseChannel(int id,AVCodecContext *codecContext) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
        this->id = id;
        this->avCodecContext = codecContext;
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

    /**
     * 释放AVPacket
     */
    static void releaseAVPacket(AVPacket **packet){
        if (packet) {
            av_packet_free(packet);
            *packet = 0;
        }
    }

    /**
     * 释放AVFrame
     */
    static void releaseAVFrame(AVFrame **frame){
        if (frame) {
            av_frame_free(frame);
            *frame = 0;
        }
    }

    /**
     * 纯虚函数（抽象方法）
     */
     virtual void start() = 0;
    virtual void stop() = 0;

    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    //标识
    int id;
    AVCodecContext *avCodecContext;
};


#endif //FFMPEGTEST_BASECHANNEL_H
