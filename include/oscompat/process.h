#ifndef OSCOMPAT_PROCESS_H
#define OSCOMPAT_PROCESS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Run argv and capture its standard output into out, blocking until the child exits. */
int os_run_capture(const char* const* argv, char* out, size_t outSize, int* exitCode);

/* Run argv discarding its standard output and standard error, blocking until the child exits. */
int os_run(const char* const* argv, int* exitCode);

/* Return the identifier of the calling process. */
int os_process_id(void);

#ifdef __cplusplus
}
#endif

#endif
