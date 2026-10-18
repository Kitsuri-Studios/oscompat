#ifndef OSCOMPAT_DYNLIB_H
#define OSCOMPAT_DYNLIB_H

#include <oscompat/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OsDynlib OsDynlib;

/* Load the shared library at path, or the current process when path is NULL; return a handle or NULL on failure. */
OsDynlib* os_dynlib_open(const char* path);

/* Resolve the named symbol in lib; return the address or NULL on failure. */
void* os_dynlib_sym(OsDynlib* lib, const char* name);

/* Close lib and release its handle; return 0 on success or -1 on failure. */
int os_dynlib_close(OsDynlib* lib);

/* Return the last dynamic loading error as a string; never returns NULL. */
const char* os_dynlib_error(void);

#ifdef __cplusplus
}
#endif

#endif
