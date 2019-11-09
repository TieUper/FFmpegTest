//
// Created by Administrator on 2019/11/6.
//

#include "JavaCallHelper.h"

JavaCallHelper::JavaCallHelper(JavaVM *javaVm_, JNIEnv *env_, jobject instance_) {
    this->javaVm = javaVm_;
    this->env = env_;
    //一旦涉及到jobject 跨方法、跨线程，需要创建全局引用
//    this->instance = instance_; //不能直接赋值
    this->instance = env->NewGlobalRef(instance_);
    jclass clazz = env->GetObjectClass(instance);
    //cd 进入 class所在目录 执行: javap -s 全限定名， 查看输出的 descriptor

    //()V 括号里是传参数 V 是返回值Void
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
}
JavaCallHelper::~JavaCallHelper() {
    javaVm = 0;
    env->DeleteGlobalRef(instance);
    instance = 0;
}

void JavaCallHelper::onPrepared(int threadMode) {
    if(threadMode == THREAD_MAIN){
        //主线程
        env->CallVoidMethod(instance, jmd_prepared);
    }else {
        //子线程
        //当前子线程的 JNIEnv
        JNIEnv *env_child;
        javaVm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_prepared);
        javaVm->DetachCurrentThread();
    }
}

void JavaCallHelper::onError(int threadMode, int code) {

}

