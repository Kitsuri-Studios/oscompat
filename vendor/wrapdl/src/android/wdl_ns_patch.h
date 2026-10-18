#ifndef WDL_NS_PATCH_H
#define WDL_NS_PATCH_H

#include <stdbool.h>

bool wdl_ns_patch_soname(const char *path, int fd, const char *patch);

#endif
