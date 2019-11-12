//
// Created by Administrator on 2019/11/6.
//


#include "VideoChannel.h"


VideoChannel::~VideoChannel() {

}

VideoChannel::VideoChannel(int id, AVCodecContext *avCodecContext, int fps) : BaseChannel(id, avCodecContext) {
    this->fps = fps;
}

void *task_video_decode(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();
    return 0;
}

void *task_video_play(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();
    return 0;
}

void VideoChannel::start() {
    isPlaying = 1;
    //设置队列状态为工作状态
    packets.setWork(1);
    frames.setWork(1);
    //可以进行解码播放？
    //解码
    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    //播放
    pthread_create(&pid_video_play, 0, task_video_play, this);
}



void VideoChannel::stop() {

}

/**
 * 真正的视频解码
 */
void VideoChannel::video_decode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        int ret = packets.pop(packet);
        if (!isPlaying) {
            //停止播放了，跳出循环
            break;
        }
        if (!ret) {
            //取数据包失败
            continue;
        }
        //拿到了视频数据包(编码压缩了的)，需要吧数据包给解码器进行解码
        ret = avcodec_send_packet(avCodecContext,packet);
        if (ret) {
            //往解码器发送数据包失败，跳出循环
            break;
        }
        releaseAVPacket(&packet);
        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext,avFrame);
        if (ret == AVERROR(EAGAIN)) {
            //重来
            continue;
        } else if (ret != 0) {
            break;
        }
        //数据收发正常，成功获取到了解码后的视频原始数据包 AVFrame 格式yuv
        //对frame进行处理（渲染播放）
        while (isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        frames.push(avFrame);
    }//end while
    releaseAVPacket(&packet);
}

/**
 * 真正的视频播放
 */
void VideoChannel::video_play() {

    AVFrame *avFrame = 0;
    //对原始数据进行格式转换：yuv -> rgba
    uint8_t *dst_data[4];
    int dst_linesize[4];

    SwsContext *swsContext = sws_getContext(avCodecContext->width,avCodecContext->height,avCodecContext->pix_fmt,
            avCodecContext->width,avCodecContext->height,AV_PIX_FMT_RGBA,
            SWS_BILINEAR,NULL,NULL,NULL);
    av_image_alloc(dst_data,dst_linesize,avCodecContext->width,avCodecContext->height,AV_PIX_FMT_RGBA,1);
    //根据fps(传入的流的平均帧率来控制每一帧的延时时间)
    //sleep: fps > 时间
    //单位是秒
    double delay_time_per_frame = 1.0 / fps;
    while (isPlaying) {
        int ret = frames.pop(avFrame);
        if (!isPlaying) {
            //停止播放了，跳出循环 释放frame
            break;
        }
        if (!ret) {
            //取frame失败
            continue;
        }
        //取到了yuv原始数据，下面进行格式转换
        sws_scale(swsContext, avFrame->data, avFrame->linesize, 0, avCodecContext->height, dst_data,
                  dst_linesize);
        //进行休眠
        //每一帧还有自己的额外延时时间
        double extra_delay = avFrame->repeat_pict / (2*fps);
        double real_delay = extra_delay + delay_time_per_frame;
        //单位是微秒
        av_usleep(real_delay * 1000000);
        //dst_data:AV_PIX_FMT_RGBA格式里的数据
        //渲染，回调出去-> native-lib里
        //渲染一副图像需要什么信息：（宽高->图像的尺寸）（图像的内容（数据）怎么画）
        //需要：1 data 2 linesize 3 width 4 height
        renderCallback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        releaseAVFrame(&avFrame);
    }
    releaseAVFrame(&avFrame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}
