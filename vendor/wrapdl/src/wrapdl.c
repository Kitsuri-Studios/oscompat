#include "wrapdl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define wdl_strdup _strdup
#else
#include <dlfcn.h>
#include <link.h>
#define wdl_strdup strdup
#endif

static char *wdl_deco_path(const char *path) {
    if (!path) return NULL;

#ifdef _WIN32
    const char *pre = "";
    const char *suf = ".dll";
#elif defined(__APPLE__)
    const char *pre = "lib";
    const char *suf = ".dylib";
#else
    const char *pre = "lib";
    const char *suf = ".so";
#endif

    size_t n = strlen(path), pl = strlen(pre), sl = strlen(suf);
    if (sl && n >= sl && !strcmp(path + n - sl, suf))
        return wdl_strdup(path);

    const char *base = strrchr(path, '/');
#ifdef _WIN32
    const char *bw = strrchr(path, '\\');
    if (!base || (bw && bw > base)) base = bw;
#endif

    char *out = malloc(n + pl + sl + 1);
    if (!out) return NULL;

    if (base && pl) {
        size_t d = (size_t)(base - path) + 1;
        memcpy(out, path, d);
        memcpy(out + d, pre, pl);
        strcpy(out + d + pl, base + 1);
        strcat(out, suf);
    } else {
        out[0] = '\0';
        if (pl) strcpy(out, pre);
        strcat(out, path);
        strcat(out, suf);
    }
    return out;
}

#ifdef __ANDROID__

#include "wdl_int.h"

static int wdl_to_flags(int flags, const char *path) {
    int f = WDL_DEFAULT;
    if (flags & WDL_LOAD) f |= WDL_ALWAYS_FORCE_LOAD;
    else if (flags & WDL_FORCE) f |= WDL_TRY_FORCE_LOAD;
    else if (path && path[0] == '/') f |= WDL_TRY_FORCE_LOAD;
    return f;
}

static __thread char wdl_err_android[256];

wdl_h wdlopen(const char *path, int flags) {
    char *tmp = NULL;
    if (flags & WDL_DECOR) {
        tmp = wdl_deco_path(path);
        path = tmp;
    }
    dlerror();
    wdl_h h = (wdl_h)wdl_open(path, wdl_to_flags(flags, path));
    if (!h) {
        const char *e = dlerror();
        if (e) snprintf(wdl_err_android, sizeof(wdl_err_android), "%s", e);
        else snprintf(wdl_err_android, sizeof(wdl_err_android), "library not loaded");
    } else {
        wdl_err_android[0] = '\0';
    }
    free(tmp);
    return h;
}

wdl_h wdlopen_phdr(struct dl_phdr_info *info) {
    return (wdl_h)wdl_open2(info);
}

int wdlclose(wdl_h h) {
    wdl_close((void *)h);
    return 0;
}

wdl_sym wdlsym(wdl_h h, const char *sym, size_t *size, int flags) {
    if (flags & WDL_DEBUG)
        return (wdl_sym)wdl_i_dsym((void *)h, sym, size);
    return (wdl_sym)wdl_i_sym((void *)h, sym, size);
}

int wdladdr(void *addr, wdl_info *info, void **cache, int flags) {
    wdl_info_t wi;
    int r = wdl_addr4(addr, &wi, cache, (flags & WDL_NO_SYM) ? WDL_NON_SYM : WDL_DEFAULT);
    if (!r || !info) return r;
    info->file = wi.dli_fname;
    info->base = wi.dli_fbase;
    info->sym = wi.dli_sname;
    info->addr = wi.dli_saddr;
    info->sym_size = wi.dli_ssize;
    info->phdr = wi.dlpi_phdr;
    info->phnum = wi.dlpi_phnum;
    return r;
}

void wdlclear(void **cache) {
    wdl_addr_clean(cache);
}

int wdliter(int (*cb)(struct dl_phdr_info *, size_t, void *), void *data, int flags) {
    return wdl_iterate_phdr(cb, data, (flags & WDL_FULLPATH) ? WDL_FULL_PATHNAME : WDL_DEFAULT);
}

const char *wdlerror(void) {
    const char *e = dlerror();
    if (e) return e;
    return wdl_err_android[0] ? wdl_err_android : NULL;
}

#else

#ifdef _WIN32
static char wdl_err[512];
#else
static __thread char wdl_err[512];
#endif

static void wdl_fail(const char *msg) {
    if (msg) {
        strncpy(wdl_err, msg, sizeof(wdl_err) - 1);
        wdl_err[sizeof(wdl_err) - 1] = '\0';
    } else {
        wdl_err[0] = '\0';
    }
}

#ifdef _WIN32
static wdl_h wdl_plat_open(const char *p) {
    wdl_h h = LoadLibraryA(p);
    if (!h) {
        char msg[256];
        DWORD err = GetLastError();
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, msg, sizeof(msg), NULL))
            wdl_fail(msg);
        else
            wdl_fail("LoadLibrary failed");
    }
    return h;
}
static wdl_sym wdl_plat_sym(wdl_h h, const char *s) {
    wdl_sym r = (wdl_sym)GetProcAddress(h, s);
    if (!r) wdl_fail("GetProcAddress failed");
    return r;
}
static void wdl_plat_close(wdl_h h) {
    if (!FreeLibrary(h)) wdl_fail("FreeLibrary failed");
}
#else
static wdl_h wdl_plat_open(const char *p) {
    dlerror();
    wdl_h h = dlopen(p, RTLD_NOW | RTLD_LOCAL);
    if (!h) wdl_fail(dlerror() ? dlerror() : "dlopen failed");
    return h;
}
static wdl_sym wdl_plat_sym(wdl_h h, const char *s) {
    dlerror();
    wdl_sym r = (wdl_sym)dlsym(h, s);
    if (!r) wdl_fail(dlerror() ? dlerror() : "dlsym failed");
    return r;
}
static void wdl_plat_close(wdl_h h) {
    if (dlclose(h)) wdl_fail(dlerror() ? dlerror() : "dlclose failed");
}
#endif

wdl_h wdlopen(const char *path, int flags) {
    if (!path) { wdl_fail("null path"); return NULL; }
    char *tmp = NULL;
    if (flags & WDL_DECOR) { tmp = wdl_deco_path(path); path = tmp; }
    wdl_h h = wdl_plat_open(path);
    free(tmp);
    (void)flags;
    return h;
}

int wdlclose(wdl_h h) {
    if (!h) return -1;
    wdl_plat_close(h);
    return 0;
}

wdl_sym wdlsym(wdl_h h, const char *sym, size_t *size, int flags) {
    (void)flags;
    if (size) *size = 0;
    if (!h || !sym) return NULL;
    return wdl_plat_sym(h, sym);
}

int wdladdr(void *addr, wdl_info *info, void **cache, int flags) {
    (void)cache;
    if (!addr || !info) return 0;
#ifndef _WIN32
    Dl_info di;
    if (!dladdr(addr, &di)) return 0;
    info->file = di.dli_fname;
    info->base = di.dli_fbase;
    if (!(flags & WDL_NO_SYM)) {
        info->sym = di.dli_sname;
        info->addr = di.dli_saddr;
    } else {
        info->sym = NULL;
        info->addr = NULL;
    }
    info->sym_size = 0;
    return 1;
#else
    (void)flags;
    wdl_fail("wdladdr unsupported");
    return 0;
#endif
}

void wdlclear(void **cache) {
    (void)cache;
}

int wdliter(int (*cb)(struct dl_phdr_info *, size_t, void *), void *data, int flags) {
    (void)flags;
#ifndef _WIN32
    return dl_iterate_phdr(cb, data);
#else
    (void)cb; (void)data;
    wdl_fail("wdliter unsupported");
    return -1;
#endif
}

const char *wdlerror(void) {
    return wdl_err[0] ? wdl_err : NULL;
}

#endif
