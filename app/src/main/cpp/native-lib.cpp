#include <jni.h>
#include <string>
#include "NEFFmpeg.h"
#include <android/native_window_jni.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

JavaVM *javaVm = 0;

NEFFmpeg *ffmpeg;

ANativeWindow* window = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//静态初始化mutex

jint JNI_OnLoad(JavaVM *vm, void *reserved){
    javaVm = vm;
    return JNI_VERSION_1_4;
}

void renderFrame(uint8_t *src_data, int src_lineSize, int width, int height){
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    //设置窗口属性
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //把buffer中的数据进行赋值(修改)
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_lineSize = window_buffer.stride * 4; //ARGB
    //逐行拷贝
    for (int i = 0; i < window_buffer.height; i++) {
        memcpy(dst_data + i * dst_lineSize, src_data + i * src_lineSize, dst_lineSize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_ffmpegtest_NEPlayer_prepareNative(JNIEnv *env, jobject thiz, jstring source) {
    const char *dataSource = env->GetStringUTFChars(source, 0);
    JavaCallHelper *javaCallHelper = new JavaCallHelper(javaVm,env,thiz);
    ffmpeg = new NEFFmpeg(javaCallHelper, const_cast<char *>(dataSource));
    ffmpeg->setRenderCallback(renderFrame);
    ffmpeg->prepare();
    env->ReleaseStringUTFChars(source, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_ffmpegtest_NEPlayer_startNative(JNIEnv *env, jobject thiz) {
    ffmpeg->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_ffmpegtest_NEPlayer_setSurfaceNative(JNIEnv *env, jobject thiz, jobject surface) {
    pthread_mutex_lock(&mutex);
    //先释放之前的显示窗口
    if(window) {
        ANativeWindow_release(window);
        window = 0;
    }
    //创建新的窗口用于视频显示
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}