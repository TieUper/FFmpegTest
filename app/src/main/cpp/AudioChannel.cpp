//
// Created by Administrator on 2019/11/6.
//

#include "AudioChannel.h"

AudioChannel::~AudioChannel() {

}

AudioChannel::AudioChannel(int i, AVCodecContext *pContext) : BaseChannel(i, pContext) {
    //缓冲区大小如何定
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    //2(通道数）*2（16bits = 2 字节）* 44100（采样率）
    out_buffers_size = out_channels * out_sample_size * out_sample_rate;
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));
    memset(out_buffers, 0, out_buffers_size);
}

void *task_audio_decode(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->audio_decode();
    return 0;
}

void *task_audio_play(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->audio_play();
    return 0;
}

void AudioChannel::start() {
    isPlaying = 1;
    //设置队列状态为工作状态
    packets.setWork(1);
    frames.setWork(1);
    //可以进行解码播放？
    //解码
    pthread_create(&pid_audio_decode, 0, task_audio_decode, this);
    //播放
    pthread_create(&pid_audio_play, 0, task_audio_play, this);
}

void AudioChannel::stop() {

}

/**
 * 音频解码
 */
void AudioChannel::audio_decode() {
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
        ret = avcodec_send_packet(avCodecContext, packet);
        if (ret) {
            //往解码器发送数据包失败，跳出循环
            break;
        }
        releaseAVPacket(&packet);
        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
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
        frames.push(avFrame); //PCM数据
    }//end while
    releaseAVPacket(&packet);
}

/**
 * 创建回调函数
 */
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {

    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    int pcm_size = audioChannel->getPCM();
    if (pcm_size > 0) {
        (*bq)->Enqueue(bq, audioChannel->out_buffers, pcm_size);
    }
}

/**
 * 音频播放
 */
void AudioChannel::audio_play() {
    /**
     * 1、创建引擎与接口
     */
    SLresult result;
    // 创建引擎 SLObjectItf engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 初始化引擎
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 获取引擎接口SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,
                                           &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    /**
     * 2、设置混音器
     */
    // 创建混音器SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0,
                                                 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //不启用混响可以不用获取接口
    // 获得混音器接口
    //result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
    //                                         &outputMixEnvironmentalReverb);
    //if (SL_RESULT_SUCCESS == result) {
    //设置混响 ： 默认。
    //SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
    //SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
    //const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    //(*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
    //       outputMixEnvironmentalReverb, &settings);
    //}

    /**
     * 3、创建播放器
     */
    /**
    * 配置输入声音信息
    */
    //创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    //pcm数据格式
    //SL_DATAFORMAT_PCM：数据格式为pcm格式
    //2：双声道
    //SL_SAMPLINGRATE_44_1：采样率为44100
    //SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit
    //SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit
    //SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）
    //SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};

    //数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};

    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    //需要的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    //创建播放器
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource,
                                          &audioSnk, 1,
                                          ids, req);
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    // 得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);

    /**
     * 4、设置播放回调
     */
    //获取播放器队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueue);
    //设置回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    /**
     * 5.设置播放状态
     */
    // 设置播放状态
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);

    /**
     * 6、启动回调函数
     */
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

/**
 * 获取pcm数据
 * @return 数据大小
 */
int AudioChannel::getPCM() {
    int pcm_data_size = 0;
    AVFrame *frame = 0;
    SwrContext *swrContext = swr_alloc_set_opts(0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                                out_sample_rate, avCodecContext->channel_layout,
                                                avCodecContext->sample_fmt,
                                                avCodecContext->sample_rate,
                                                0, 0);
    //    初始化重采样上下文
    swr_init(swrContext);

    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            //停止播放，跳出循环
            break;
        }
        if (!ret) {
            //取数据包失败
            continue;
        }
        //pcm数据在frame中
        //这里获得的解码后的pcm格式的音频原始数据，有可能与创建的播放器中设置的pcm格式不一样
        //重采样

        //swr_get_delay 下一个输入数据与下下个输入数据之间的
        int64_t delay = swr_get_delay(swrContext, frame->sample_rate);

        int64_t out_max_samples = av_rescale_rnd(frame->nb_samples + delay, frame->sample_rate,
                                                 out_sample_rate,
                                                 AV_ROUND_UP);

        //参数  1上下文2输出缓冲区 3 输出缓冲区能容纳的最大数据量 4 输入数据 5 输入数据个数
        int out_samples = swr_convert(swrContext, &out_buffers, out_max_samples,
                                      (const uint8_t **) (frame->data),
                                      frame->nb_samples);

        // 获取swr_convert转换后 out_samples个 *2 （16位）*2（双声道）
        pcm_data_size = out_samples * out_sample_size * out_channels;

        break;
    }//end while
    releaseAVFrame(&frame);
    return pcm_data_size;
}
