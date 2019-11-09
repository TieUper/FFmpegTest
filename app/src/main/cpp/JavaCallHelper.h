//
// Created by Administrator on 2019/11/6.
//

#ifndef FFMPEGTEST_JAVACALLHELPER_H
#define FFMPEGTEST_JAVACALLHELPER_H


#include <jni.h>
#include "macro.h"

class JavaCallHelper {

public:
    JavaCallHelper(JavaVM *javaVm_,JNIEnv *env_, jobject instance_);

    ~JavaCallHelper();

    void onPrepared(int threadMode);

    void onError(int threadMode,int code);

private:
    JavaVM *javaVm;
    JNIEnv *env;
    jobject instance;
    jmethodID jmd_prepared;

};


#endif //FFMPEGTEST_JAVACALLHELPER_H
