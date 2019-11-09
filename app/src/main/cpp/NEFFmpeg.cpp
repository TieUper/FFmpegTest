//
// Created by Administrator on 2019/11/6.
//


#include "NEFFmpeg.h"

NEFFmpeg::NEFFmpeg(JavaCallHelper *javaCallHelper, char *dataSource) {
    this->javaCallHelper = javaCallHelper;
    //dataSource 在jni中被释放掉，导致dataSource变成悬空指针（指向一块儿已经释放掉的内存）
    //解决： 内存拷贝
    //strlen 获取字符串长度  strcpy:拷贝字符串

    //c字符串以 \0 结尾  ep:  java: "hello" c : "hello\0" 长度要加一
    this->dataSource = new char[strlen(dataSource) + 1];
    strcpy(this->dataSource, dataSource);

}

NEFFmpeg::~NEFFmpeg() {
    DELETE(dataSource);
    DELETE(javaCallHelper);
}

/**
 * 准备线程pid_prepare真正执行函数
 * @param args
 * @return
 */
void *task_prepare(void* args){
    NEFFmpeg *ffmpeg = static_cast<NEFFmpeg *>(args);
    //打开输入  2 url-> dataSource
    ffmpeg->_prepare();

    return 0; //一定要返回0！！！
}


/**
 * 准备线程pid_prepare真正执行函数
 * @param args
 * @return
 */
void *task_start(void* args){
    NEFFmpeg *ffmpeg = static_cast<NEFFmpeg *>(args);
    //打开输入  2 url-> dataSource
    ffmpeg->_start();

    return 0; //一定要返回0！！！
}
/**
 * 播放准备
 */
void NEFFmpeg::prepare() {
    //线程问题， 子线程进行解码操作
    //文件: io流问题
    //直播： 网络
    //创建子线程   第三个参数 线程工作要执行的方法 第四个 线程要传的参数
    pthread_create(&pid_prepare, 0, task_prepare,this);
}

/**
 * 打开输入源
 */
void NEFFmpeg::_prepare() {

    //1 AVFormatContext **ps
    formatContext = avformat_alloc_context();
    //3 输入格式 0 自动
    AVDictionary *dictionary = 0;
    //设置超时时间为10s  单位 微秒
    av_dict_set(&dictionary, "timeout", "10000000", 0);
    // 1) 打开媒体
    int ret = avformat_open_input(&formatContext, dataSource, 0, &dictionary);
    av_dict_free(&dictionary);
    if (ret) {
        //失败，回调给java
        LOGE("打开媒体失败: %s", av_err2str(ret));
        // javaCallHelper jni 回调java方法
//        javaCallHelper->onError(ret);
    }
    //2) 查找媒体中音/视频流信息
    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        //失败
        return;
    }
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //3)获取媒体流（音频或视频)
        AVStream *stream = formatContext->streams[i];
        //4)获取编解码这段流的参数
        AVCodecParameters *codecParameters = stream->codecpar;
        //5)通过参数中的id(编解码的方式)，来查找当前流的解码器
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if(!codec) {
            return;
        }
        //6)创建解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        //7)设置解码器上下文的参数
        ret = avcodec_parameters_to_context(codecContext,codecParameters);
        if (ret < 0) {
            return;
        }
        //8)打开解码器
        ret = avcodec_open2(codecContext,codec,0);
        if (ret) {
            return;
        }
        //判断流类型(音频还是视频)
        if(codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频 AudioChannel
            audioChannel = new AudioChannel(i, codecContext);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频 VideoChannel
            videoChannel = new VideoChannel(i, codecContext);
            videoChannel->setRenderCallback(renderCallback);
        }
    }

    if (!audioChannel && !videoChannel) {
        //既没有音频也没有视频
        return;
    }
    //准备完成，反射通知java
    javaCallHelper->onPrepared(THREAD_CHILD);
}

/**
 * 开始播放
 */
void NEFFmpeg::start() {
    isPlaying = 1;
    videoChannel->start();
    pthread_create(&pid_start, 0, task_start,this);
}

/**
 * 真正执行解码播放
 */
void NEFFmpeg::_start() {
    while (isPlaying) {
        AVPacket *packet = av_packet_alloc();

        int ret = av_read_frame(formatContext, packet);
        if (!ret) {
            if (videoChannel && packet->stream_index == videoChannel->id) {
                //往视频编码数据包队列中添加数据
                videoChannel->packets.push(packet);
            } else if (audioChannel && packet->stream_index == audioChannel->id) {

            }
        } else if (ret == AVERROR_EOF) {
            //表示读完了 考虑读完了，是否播完了
//            LOGE("完成");
        }else {
            LOGE("读取音视频数据包失败");
            break;
        }
    }

    isPlaying = 0;
    //停止解码播放
    videoChannel->stop();
    videoChannel->stop();
}

void NEFFmpeg::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}
