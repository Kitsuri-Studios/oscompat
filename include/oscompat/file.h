#ifndef OSCOMPAT_FILE_H
#define OSCOMPAT_FILE_H

#include <oscompat/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return 1 when path exists, otherwise 0. */
int os_file_exists(const char* path);

/* Return 1 when path names a regular file, otherwise 0. */
int os_is_file(const char* path);

/* Return 1 when path names a directory, otherwise 0. */
int os_is_dir(const char* path);

/* Read the whole file at path into a malloc'd, NUL-terminated buffer; store the byte count in size and return 0 on success or -1 on failure. */
int os_read_file(const char* path, char** data, size_t* size);

/* Write size bytes from data to path, creating or truncating it; return 0 on success or -1 on failure. */
int os_write_file(const char* path, const void* data, size_t size);

/* Create every missing component of path as a directory, idempotently; return 0 on success or -1 on failure. */
int os_mkdir_p(const char* path);

/* Remove the file at path, or the whole directory tree rooted at path; return 0 on success or -1 on failure. */
int os_remove_recursive(const char* path);

/* Create a unique temporary file and write its absolute path into buf; return 0 on success or -1 on failure. */
int os_temp_file(char* buf, size_t cap);

#ifdef __cplusplus
}
#endif

#endif
