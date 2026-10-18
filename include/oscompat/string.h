#ifndef OSCOMPAT_STRING_H
#define OSCOMPAT_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the first occurrence of needle inside hay, or NULL when absent. */
void* os_memmem(const void* hay, size_t hayLen, const void* needle, size_t needleLen);

/* Returns the last occurrence of byte c in the first n bytes of s, or NULL. */
void* os_memrchr(const void* s, int c, size_t n);

/* Returns a pointer to the first byte equal to c, or the terminating NUL. */
char* os_strchrnul(const char* s, int c);

/* Compares two strings case-insensitively as unsigned bytes. */
int os_strcasecmp(const char* a, const char* b);

/* Compares at most n bytes of two strings case-insensitively. */
int os_strncasecmp(const char* a, const char* b, size_t n);

/* Duplicates a string with malloc, or returns NULL on allocation failure. */
char* os_strdup(const char* s);

/* Duplicates at most n bytes of a string with malloc, or NULL on failure. */
char* os_strndup(const char* s, size_t n);

/* Copies src into dst with NUL termination and returns the length of src. */
size_t os_strlcpy(char* dst, const char* src, size_t size);

/* Appends src to dst with NUL termination and returns the intended length. */
size_t os_strlcat(char* dst, const char* src, size_t size);

/* Extracts the next delimiter-separated token, returning NULL when exhausted. */
char* os_strsep(char** stringp, const char* delim);

/* Reentrant delimiter tokenizer storing its cursor in saveptr. */
char* os_strtok_r(char* str, const char* delim, char** saveptr);

/* Overwrites n bytes with zero without being removed by the optimizer. */
void os_explicit_bzero(void* p, size_t n);

#ifdef __cplusplus
}
#endif

#endif
