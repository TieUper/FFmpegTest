//
// Created by Administrator on 2019/11/6.
//

#ifndef FFMPEGTEST_MACRO_H
#define FFMPEGTEST_MACRO_H

#include <android/log.h>

//定义释放的宏函数
#define DELETE(object) if(object){delete object; object = 0;}

//定义日志打印宏函数
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"NEFFMPEG",__VA_ARGS__)

//标记线程模式
#define THREAD_MAIN 1
#define THREAD_CHILD 2


#endif //FFMPEGTEST_MACRO_H
