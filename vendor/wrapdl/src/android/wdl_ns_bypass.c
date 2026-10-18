#include "wrapdl.h"
#include "../wdl_int.h"

#if defined(__ANDROID__)

#include <android/api-level.h>
#include <android/dlext.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "wdl_ns_patch.h"

typedef struct android_namespace_t *(*wdl_ld_create_ns_t)(const char *, const char *, const char *,
                                                          uint64_t, const char *,
                                                          struct android_namespace_t *, const void *);
typedef void *(*wdl_ld_dlopen_t)(const char *, int, const void *);

static wdl_ld_create_ns_t ld_create_ns;
static wdl_ld_dlopen_t ld_dlopen;
static bool ns_ready;

static struct android_namespace_t *(*fn_exported)(const char *);
static bool (*fn_linkall)(struct android_namespace_t *, struct android_namespace_t *);
static bool (*fn_link)(struct android_namespace_t *, struct android_namespace_t *, const char *);

int wdlns_ok(void) {
    return ns_ready ? 1 : 0;
}

struct android_namespace_t *wdlns_new(const char *name, const char *ld_path, const char *def_path,
                                    uint64_t type, const char *perm_path,
                                    struct android_namespace_t *parent) {
    return ld_create_ns(name, ld_path, def_path, type, perm_path, parent,
                        __builtin_return_address(0));
}

struct android_namespace_t *wdlns_escape(const char *name, const char *ld_path, const char *def_path,
                                         uint64_t type, const char *perm_path,
                                         struct android_namespace_t *parent) {
    return ld_create_ns(name, ld_path, def_path, type, perm_path, parent, (void *)&dlopen);
}

struct android_namespace_t *wdlns_exported(const char *name) {
    return fn_exported ? fn_exported(name) : NULL;
}

int wdlns_linkall(struct android_namespace_t *to, struct android_namespace_t *from) {
    return fn_linkall && fn_linkall(to, from);
}

int wdlns_link(struct android_namespace_t *to, struct android_namespace_t *from, const char *libs) {
    return fn_link && fn_link(to, from, libs);
}

int wdlns_link_default(struct android_namespace_t *ns) {
    static struct android_namespace_t *def_copy;
    if (!def_copy)
        def_copy = wdlns_escape("default_copy", NULL, NULL, WDL_NS_SHARED, NULL, NULL);
    return def_copy ? wdlns_linkall(ns, def_copy) : 0;
}

void *wdlns_open(const char *path, int flags, struct android_namespace_t *ns) {
    android_dlextinfo ext = {.flags = ANDROID_DLEXT_USE_NAMESPACE, .library_namespace = ns};
    dlerror();
    void *h = android_dlopen_ext(path, flags, &ext);
    if (h || !path) return h;
    const char *base = strrchr(path, '/');
    if (!base) return NULL;
    dlerror();
    return android_dlopen_ext(base + 1, flags, &ext);
}

#ifndef __NR_memfd_create
#if defined(__aarch64__)
#define __NR_memfd_create 279
#elif defined(__arm__)
#define __NR_memfd_create 385
#elif defined(__x86_64__)
#define __NR_memfd_create 319
#endif
#endif

void *wdlns_open_unique(const char *path, const char *tmpdir, int flags,
                        struct android_namespace_t *ns) {
    static char buf[4096];
    static uint16_t id;
    int fd;

    if (tmpdir) {
        snprintf(buf, sizeof(buf), "%s/%u_patched.so", tmpdir, (unsigned)id);
        fd = open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    } else {
        errno = 0;
        fd = (int)syscall(__NR_memfd_create, path, 0);
        if (errno == ENOSYS || fd < 0) return NULL;
    }
    if (fd < 0) return NULL;

    char patch[4];
    snprintf(patch, sizeof(patch), "%03u", (unsigned)id++);
    if (!wdl_ns_patch_soname(path, fd, patch)) {
        close(fd);
        return NULL;
    }

    android_dlextinfo ext = {
        .flags = ANDROID_DLEXT_USE_NAMESPACE | ANDROID_DLEXT_USE_LIBRARY_FD,
        .library_fd = fd,
        .library_namespace = ns,
    };
    snprintf(buf, sizeof(buf), "/proc/self/fd/%d", fd);
    return android_dlopen_ext(buf, flags, &ext);
}

static void *align_page(void *p) {
    long ps = sysconf(_SC_PAGESIZE);
    return (void *)((uintptr_t)p & ~((uintptr_t)ps - 1));
}

static wdl_ld_dlopen_t resolve_loader_dlopen(void) {
#if defined(__aarch64__)
    typedef union { uint32_t raw; struct { int32_t off : 26; uint8_t sig : 6; } b; } bl_t;
    long ps = sysconf(_SC_PAGESIZE);
    mprotect(align_page((void *)&dlopen), (size_t)ps, PROT_READ | PROT_WRITE | PROT_EXEC);
    bl_t *bl = (bl_t *)&dlopen;
    while (bl->b.sig != 0x25) bl++;
    return (wdl_ld_dlopen_t)(bl + bl->b.off);
#else
    return NULL;
#endif
}

__attribute__((constructor)) static void wdlns_boot(void) {
    if (android_get_device_api_level() < 28) return;

    ld_dlopen = resolve_loader_dlopen();
    if (!ld_dlopen) return;

    long ps = sysconf(_SC_PAGESIZE);
    mprotect(align_page((void *)&ld_dlopen), (size_t)ps, PROT_READ | PROT_WRITE | PROT_EXEC);

    void *ld = ld_dlopen("ld-android.so", RTLD_LAZY, (void *)&dlopen);
    if (!ld) return;

    fn_linkall = (bool (*)(struct android_namespace_t *, struct android_namespace_t *))
        dlsym(ld, "__loader_android_link_namespaces_all_libs");
    fn_link = (bool (*)(struct android_namespace_t *, struct android_namespace_t *, const char *))
        dlsym(ld, "__loader_android_link_namespaces");
    if (!fn_linkall || !fn_link) return;

    void *libdl = ld_dlopen("libdl_android.so", RTLD_LAZY, (void *)&dlopen);
    if (!libdl) return;

    ld_create_ns = (wdl_ld_create_ns_t)dlsym(libdl, "__loader_android_create_namespace");
    fn_exported = (struct android_namespace_t *(*)(const char *))
        dlsym(libdl, "__loader_android_get_exported_namespace");
    if (ld_create_ns && fn_exported) ns_ready = true;
}

#endif
