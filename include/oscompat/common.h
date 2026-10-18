#ifndef OSCOMPAT_COMMON_H
#define OSCOMPAT_COMMON_H

#include <stddef.h>
#include <stdint.h>

#ifndef OSCOMPAT_PATH_MAX
#define OSCOMPAT_PATH_MAX 4096
#endif

#if defined(_WIN32)
#define OSCOMPAT_WINDOWS 1
#else
#define OSCOMPAT_WINDOWS 0
#endif

#if defined(_MSC_VER)
#define OSCOMPAT_MSVC 1
#else
#define OSCOMPAT_MSVC 0
#endif

#if defined(__GNUC__) || defined(__clang__)
#define OSCOMPAT_LIKELY(x) __builtin_expect(!!(x), 1)
#define OSCOMPAT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define OSCOMPAT_LIKELY(x) (x)
#define OSCOMPAT_UNLIKELY(x) (x)
#endif

#define OSCOMPAT_UNUSED(x) ((void)(x))

#ifdef __cplusplus
extern "C" {
#endif

/* Translate the platform's last error into errno. POSIX leaves errno untouched; Win32 maps GetLastError. */
void os_set_system_error(void);

#ifdef __cplusplus
}
#endif

#endif
