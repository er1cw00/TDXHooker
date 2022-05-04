//
// Created by eric.woo on 2022/4/24.
//
#include <jni.h>
#include <assert.h>
#include <android/log.h>
#include "native_wrapper.h"
#include "native_signal.h"

static char* __className = "com/wadahana/tdxhooker/NativeHooker";

static int __wrapper_test(JNIEnv *env, jobject thiz);
static int __wrapper_hook(JNIEnv *env, jobject thiz);

static JNIEnv* __getEnv(JavaVM* jvm, bool* attached);
static int __registerNativeMethods(JNIEnv* env,
                                   const char* className,
                                   JNINativeMethod* methods,
                                   int numMethods);

static JNINativeMethod __nativeMethods[] = {
        {"test", "()I", (void *)__wrapper_test },
        {"hook", "()I", (void *)__wrapper_hook },
};


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    bool attached;
    __android_log_print(ANDROID_LOG_INFO, "TDXHooker", "JNI_OnLoad");
    JNIEnv* env = __getEnv(vm, &attached);
    if (env == NULL) {
        return -1;
    }
    assert(!attached);
    signal_setup();
    __registerNativeMethods(env,
                            __className,
                            __nativeMethods,
                            sizeof(__nativeMethods)/sizeof(__nativeMethods[0]));
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    bool attached;
    JNIEnv* env = __getEnv(vm, &attached);
    assert(!attached);
    signal_dispose();
    __android_log_print(ANDROID_LOG_INFO, "TDXHooker", "JNI_OnUnload");
}

static int __wrapper_test(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "TDXHooker", "Test");
    return 0;
}
static int __wrapper_hook(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "TDXHooker", "Hook");
    native_hook();
    return 0;
}

JNIEnv* __getEnv(JavaVM* jvm, bool* attached) {
    JNIEnv* env = NULL;
    *attached = false;
    int ret = jvm->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (ret == JNI_EDETACHED) {
        if (0 != jvm->AttachCurrentThread(&env, NULL)) {
            return NULL;
        }
        *attached = true;
        return env;
    }
    if (ret != JNI_OK) {
        return NULL;
    }
    return env;
}

static int __registerNativeMethods(JNIEnv* env,
                                   const char* className,
                                   JNINativeMethod* methods,
                                   int numMethods) {
    __android_log_print(ANDROID_LOG_INFO, "NativeHooker", "RegisterNatives start for \'%s\'", className);
    jclass clazz = env->FindClass(className);
    if (clazz == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, "NativeHooker", "unable to find class \'%s\'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "NativeHooker", "RegisterNatives failed for \'%s\'", className );
        return JNI_FALSE;
    }
    return JNI_TRUE;
}
