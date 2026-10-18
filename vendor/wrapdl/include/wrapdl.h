#ifndef WRAPDL_H
#define WRAPDL_H

#if !defined(_WIN32) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
typedef HMODULE wdl_h;
typedef FARPROC wdl_sym;
struct dl_phdr_info;
#else
#include <dlfcn.h>
#include <link.h>
typedef void *wdl_h;
typedef void *wdl_sym;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *file;
    void *base;
    const char *sym;
    void *addr;
    size_t sym_size;
#ifdef __ANDROID__
    const ElfW(Phdr) *phdr;
    size_t phnum;
#endif
} wdl_info;

#define WDL_DEFAULT  0x00
#define WDL_FORCE    0x01  // try dlopen if lib not mapped (android)
#define WDL_LOAD     0x02  // always dlopen (android)
#define WDL_DECOR    0x04  // add lib*.so / *.dll prefix-suffix
#define WDL_DEBUG    0x08  // symtab lookup instead of dynsym (android)
#define WDL_NO_SYM   0x10  // wdladdr: skip symbol info
#define WDL_FULLPATH 0x20  // wdliter: return full paths (android)

wdl_h wdlopen(const char *path, int flags);       // open lib, auto backend per os
int wdlclose(wdl_h h);                            // release handle
wdl_sym wdlsym(wdl_h h, const char *sym, size_t *size, int flags); // lookup symbol
int wdladdr(void *addr, wdl_info *info, void **cache, int flags);  // addr -> module/sym
void wdlclear(void **cache);                      // free wdladdr cache
int wdliter(int (*cb)(struct dl_phdr_info *, size_t, void *), void *data, int flags);
const char *wdlerror(void);

#ifdef __ANDROID__

struct android_namespace_t;

#define WDL_NS_REGULAR  0
#define WDL_NS_ISOLATED 1
#define WDL_NS_SHARED   2

wdl_h wdlopen_phdr(struct dl_phdr_info *info);    // open from phdr, no dlopen

int wdlns_ok(void);                               // linker ns bypass loaded
void *wdlns_open(const char *path, int flags, struct android_namespace_t *ns);
void *wdlns_open_unique(const char *path, const char *tmpdir, int flags,
                        struct android_namespace_t *ns);
struct android_namespace_t *wdlns_new(const char *name, const char *ld_path,
                                    const char *def_path, uint64_t type,
                                    const char *perm_path, struct android_namespace_t *parent);
struct android_namespace_t *wdlns_escape(const char *name, const char *ld_path,
                                         const char *def_path, uint64_t type,
                                         const char *perm_path,
                                         struct android_namespace_t *parent);
struct android_namespace_t *wdlns_exported(const char *name);
int wdlns_link(struct android_namespace_t *to, struct android_namespace_t *from, const char *libs);
int wdlns_linkall(struct android_namespace_t *to, struct android_namespace_t *from);
int wdlns_link_default(struct android_namespace_t *ns);

int wdlns_init(const char *self_path);            // find + unrestrict app namespace
int wdlns_paths(const char *const *paths, size_t n); // add ld search paths
int wdlns_isolated(int yes);                      // toggle isolation on main ns
struct android_namespace_t *wdlns_main(void);

#endif /* __ANDROID__ */

#ifdef __cplusplus
}
#endif

#endif
