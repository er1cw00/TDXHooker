//
// Created by eric.woo on 2022/5/3.
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <android/log.h>
static struct sigaction __origin_sa[NSIG];

static void __signal_handler(int sig, siginfo_t * info, void * content) {
    if (sig == SIGSEGV) {
        __android_log_print(ANDROID_LOG_INFO, "TDXHooker", ">>>>>> SIGSEGV <<<<<<");
        return;
    } else if (sig == SIGSYS) {
        __android_log_print(ANDROID_LOG_INFO, "TDXHooker", ">>>>>> SIGSYS <<<<<<");
        return;
    }
}

void signal_setup() {
    struct sigaction sa;
    memset(&__origin_sa, 0, sizeof(struct sigaction) * NSIG);
    memset(&sa, 0, sizeof(struct sigaction));

    sa.sa_sigaction = __signal_handler;
    sa.sa_flags = SA_NODEFER;
    sigaction(SIGSEGV, &sa, &__origin_sa[SIGSEGV]);
    sigaction(SIGSYS, &sa, &__origin_sa[SIGSYS]);
    return;
}

void signal_dispose() {
    sigaction(SIGSEGV, &__origin_sa[SIGSEGV], NULL);
    sigaction(SIGSYS, &__origin_sa[SIGSYS], NULL);
    return;
}