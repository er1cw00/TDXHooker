//
// Created by eric.woo on 2022/4/30.
//

#include <unistd.h>
#include <jni.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#include "elf_hooker.h"
#include "elf_log.h"

#include "native_wrapper.h"

typedef void* (*fn_dlopen)(const char* filename, int flags);
typedef void* (*fn_android_dlopen_ext) (const char* filename, int flags, void* extinfo);
typedef int (*fn_connect)(int sockfd, const struct sockaddr * serv_addr, socklen_t addrlen);

static fn_dlopen               __origin_dlopen              = NULL;
static fn_android_dlopen_ext   __origin_android_dlopen_ext  = NULL;
static fn_connect              __origin_connect             = NULL;
static bool                    __initialized                = false;
static elf_hooker              __hooker;

static void * __hooked_dlopen(const char* filename, int flags);
static void * __hooked_android_dlopen_ext(const char* filename, int flag, void * extinfo);
static int __hooked_connect(int sockfd, const struct sockaddr * dst_addr, socklen_t dst_addr_len);
static bool __hook_filter_cb(const char* module_name);
static void __hook_lib(void * handle, const char * filename);
static void __hook_socket(elf_hooker * hooker, elf_module * module);

int native_hook() {
    log_info("hook");
    if (!__hooker.load()) {
        return -1;
    }
    __hooker.set_prehook_cb(__hook_filter_cb);
    struct elf_rebinds rebinds[3] = {
            {"dlopen",              (void *)__hooked_dlopen,             (void **)&__origin_dlopen},
            {"android_dlopen_ext",  (void *)__hooked_android_dlopen_ext, (void **)&__origin_android_dlopen_ext},
            {NULL, NULL, NULL},
    };
    __hooker.hook_all_modules(rebinds);
    return 0;
}

static void * __hooked_dlopen(const char* filename, int flags) {
    void* caller_addr = __builtin_return_address(0);
    log_info("__hooked_dlopen -> flag(%d),caller_addr(%p),soname(%s) \n", flags, caller_addr, filename == NULL ? "" : filename);
    void * handle = NULL;
    if (__hooker.get_sdk_version() >= 24) {
        handle = __hooker.dlopen_ext(filename, flags, NULL, caller_addr);
    } else {
        handle = __hooker.dlopen(filename, flags);
    }
    __hook_lib(handle, filename);
    return handle;
}

static void * __hooked_android_dlopen_ext(const char* filename, int flags, void * extinfo) {
    void* caller_addr = __builtin_return_address(0);
    log_info("__hooked_android_dlopen_ext -> flag(%d),caller_addr(%p),soname(%s) \n", flags, caller_addr, filename == NULL ? "" : filename);
    void * handle = NULL;
    if (__hooker.get_sdk_version() >= 24) {
        handle = __hooker.dlopen_ext(filename, flags, extinfo, caller_addr);
    } else {
        if (__origin_android_dlopen_ext) {
            handle = __origin_android_dlopen_ext(filename, flags, extinfo);
        }
    }
    __hook_lib(handle, filename);
    return handle;
}
static int __hooked_connect(int sockfd, const struct sockaddr * dst_addr, socklen_t dst_addr_len) {
    if (dst_addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *paddr = (struct sockaddr_in6 *)dst_addr;
        if (htons(paddr->sin6_port) == 7615) {
            log_info("find ipv6 dst port");
        }
    } else if (dst_addr->sa_family == AF_INET) {
        struct sockaddr_in * paddr = (struct sockaddr_in *)dst_addr;
        if (htons(paddr->sin_port) == 7615) {
            log_info("find ipv4 dst port");
        }
    }
    return __origin_connect(sockfd, dst_addr, dst_addr_len);
}

static bool __hook_filter_cb(const char* module_name) {
    if (module_name == NULL || module_name[0] == '\0') {
        return false;
    }

    int len = strlen(module_name);
    char ch = module_name[len - 1];
    if (len >= 4) {
        if (ch == ']') {
            if (len >= 6 && memcmp(module_name + len - 6, "[vdso]", 6) == 0) {
                return false;
            }
        } else if (ch == 'o') {
            if (len >= 8 && memcmp(&module_name[len - 8], "libdl.so", 8) == 0) {
                return false;
            }
            if (len >= 15 && memcmp(&module_name[len - 15], "liblbeclient.so", 15) == 0) {
                return false;
            }
        } else if (ch == 'x') {
            // .dex .odex
            char ch1 = module_name[len - 2];
            if (ch1 == 'e') {
                if (len >= 4 && memcmp(module_name + len - 4, ".dex", 4) == 0) {
                    return false;
                }
                if (len >= 5 && memcmp(module_name + len - 5, ".odex", 5) == 0) {
                    return false;
                }
            }
        } else if (ch == 't') {
            //.oat .art
            if (len >= 4) {
                char * p = (char *)&module_name[len - 4];
                if (memcmp(p, ".oat", 4) == 0 || memcmp(p, ".art", 4) == 0) {
                    return false;
                }
            }
        } else if (ch == 'r') {
            //linker
            if (len >= 6 && memcmp(module_name + len - 6, "linker", 6) == 0) {
                return false;
            }
        } else if (ch == '4') {
            if (len >= 8 && memcmp(module_name + len - 8, "linker64", 8) == 0) {
                return false;
            }
        }
    }
    return true;
}
static void __hook_lib(void * handle, const char * filename) {
    if (!__hook_filter_cb(filename) || handle == NULL) {
        return;
    }
    void * base_addr = NULL;
    void * soinfo = (void *)__hooker.soinfo_from_handle(handle);
    if (soinfo) {
        base_addr = (void *)__hooker.base_addr_from_soinfo(soinfo);
    }
    elf_module module((ElfW(Addr))base_addr, filename);

    void * unused_pointer = NULL;
    __hooker.hook(&module, "dlopen", (void*)__hooked_dlopen, &unused_pointer);
    __hooker.hook(&module, "android_dlopen_ext", (void*)__hooked_android_dlopen_ext, &unused_pointer);

    __hook_socket(&__hooker, &module);
    return ;
}

void __hook_socket(elf_hooker * hooker, elf_module * module) {
    if (module) {
        hooker->hook(module,
                     "connect",
                     (void *) __hooked_connect,
                     (void **) &__origin_connect);
    } else {
        struct elf_rebinds rebinds[2] = {
                {"connect",     (void *)__hooked_connect,   (void **)&__origin_connect},
                {NULL,          NULL,                       NULL},
        };
        hooker->hook_all_modules(rebinds);
    }
}
