#ifndef WDL_INT_H
#define WDL_INT_H

#include <dlfcn.h>
#include <link.h>
#include <stddef.h>

typedef struct {
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_saddr;
    size_t dli_ssize;
    const ElfW(Phdr) *dlpi_phdr;
    size_t dlpi_phnum;
} wdl_info_t;

#define WDL_DEFAULT           0x00
#define WDL_TRY_FORCE_LOAD    0x01
#define WDL_ALWAYS_FORCE_LOAD 0x02
#define WDL_NON_SYM           0x01
#define WDL_FULL_PATHNAME     0x01
#define WDL_DI_DLINFO         1

void *wdl_open(const char *filename, int flags);
void *wdl_open2(struct dl_phdr_info *info);
void *wdl_close(void *handle);
void *wdl_i_sym(void *handle, const char *symbol, size_t *symbol_size);
void *wdl_i_dsym(void *handle, const char *symbol, size_t *symbol_size);
int wdl_addr4(void *addr, wdl_info_t *info, void **cache, int flags);
void wdl_addr_clean(void **cache);
int wdl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *), void *data, int flags);
int wdl_i_info(void *handle, int request, void *info);

#endif
