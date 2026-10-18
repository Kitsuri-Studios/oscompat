#ifndef OSCOMPAT_ENV_H
#define OSCOMPAT_ENV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Return the value of environment variable name, or NULL when it is unset. */
const char* os_getenv(const char* name);

/* Set environment variable name to value, honouring overwrite for existing variables. */
int os_setenv(const char* name, const char* value, int overwrite);

/* Remove environment variable name from the process environment. */
int os_unsetenv(const char* name);

/* Return the number of variables in the process environment. */
int os_environ_count(void);

/* Return the environment entry at index as a NAME=VALUE string, or NULL. */
const char* os_environ_get(int index);

#ifdef __cplusplus
}
#endif

#endif
