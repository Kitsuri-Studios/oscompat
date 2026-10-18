#ifndef OSCOMPAT_PATH_H
#define OSCOMPAT_PATH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Resolve path to an absolute canonical path, returning a malloc'd string when resolved is NULL. */
char* os_realpath(const char* path, char* resolved, size_t cap);

/* Write the full path of the running executable into buf. */
int os_exe_path(char* buf, size_t cap);

/* Write the directory containing the running executable into buf. */
int os_exe_dir(char* buf, size_t cap);

/* Return 1 when path is absolute on the host platform, otherwise 0. */
int os_path_is_absolute(const char* path);

/* Join base and part with a single separator into buf. */
int os_path_join(char* buf, size_t cap, const char* base, const char* part);

/* Collapse dot segments and duplicate separators in path into buf. */
int os_path_normalize(char* buf, size_t cap, const char* path);

/* Write the directory portion of path into buf. */
int os_path_dirname(const char* path, char* buf, size_t cap);

/* Write the final component of path into buf. */
int os_path_basename(const char* path, char* buf, size_t cap);

/* Write the current user home directory into buf. */
int os_home_dir(char* buf, size_t cap);

/* Write the system temporary directory into buf. */
int os_temp_dir(char* buf, size_t cap);

#ifdef __cplusplus
}
#endif

#endif
