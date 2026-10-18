#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <oscompat/env.h>
#include <oscompat/common.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if OSCOMPAT_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
extern char** environ;
#endif

const char* os_getenv(const char* name) {
	if (name == NULL) {
		return NULL;
	}
	return getenv(name);
}

#if OSCOMPAT_WINDOWS
static char* osWinEnvBlock = NULL;
static int osWinEnvCount = 0;

static void osWinEnvReset(void) {
	if (osWinEnvBlock != NULL) {
		FreeEnvironmentStringsA(osWinEnvBlock);
		osWinEnvBlock = NULL;
		osWinEnvCount = 0;
	}
}

static int osWinEnvLoad(void) {
	char* cursor;
	if (osWinEnvBlock != NULL) {
		return 0;
	}
	osWinEnvBlock = GetEnvironmentStringsA();
	if (osWinEnvBlock == NULL) {
		os_set_system_error();
		return -1;
	}
	osWinEnvCount = 0;
	cursor = osWinEnvBlock;
	while (*cursor != '\0') {
		if (*cursor != '=') {
			osWinEnvCount++;
		}
		cursor += strlen(cursor) + 1;
	}
	return 0;
}
#endif

int os_setenv(const char* name, const char* value, int overwrite) {
	if (name == NULL || value == NULL || name[0] == '\0') {
		errno = EINVAL;
		return -1;
	}
#if OSCOMPAT_WINDOWS
	if (!overwrite && getenv(name) != NULL) {
		return 0;
	}
	if (_putenv_s(name, value) != 0) {
		os_set_system_error();
		return -1;
	}
	osWinEnvReset();
	return 0;
#else
	if (setenv(name, value, overwrite) != 0) {
		return -1;
	}
	return 0;
#endif
}

int os_unsetenv(const char* name) {
	if (name == NULL || name[0] == '\0') {
		errno = EINVAL;
		return -1;
	}
#if OSCOMPAT_WINDOWS
	if (_putenv_s(name, "") != 0) {
		os_set_system_error();
		return -1;
	}
	osWinEnvReset();
	return 0;
#else
	if (unsetenv(name) != 0) {
		return -1;
	}
	return 0;
#endif
}

int os_environ_count(void) {
#if OSCOMPAT_WINDOWS
	if (osWinEnvLoad() != 0) {
		return 0;
	}
	return osWinEnvCount;
#else
	int count = 0;
	if (environ == NULL) {
		return 0;
	}
	while (environ[count] != NULL) {
		count++;
	}
	return count;
#endif
}

const char* os_environ_get(int index) {
	if (index < 0) {
		return NULL;
	}
#if OSCOMPAT_WINDOWS
	{
		char* cursor;
		int seen = 0;
		if (osWinEnvLoad() != 0) {
			return NULL;
		}
		if (index >= osWinEnvCount) {
			return NULL;
		}
		cursor = osWinEnvBlock;
		while (*cursor != '\0') {
			if (*cursor != '=') {
				if (seen == index) {
					return cursor;
				}
				seen++;
			}
			cursor += strlen(cursor) + 1;
		}
		return NULL;
	}
#else
	{
		int i;
		if (environ == NULL) {
			return NULL;
		}
		for (i = 0; i <= index; i++) {
			if (environ[i] == NULL) {
				return NULL;
			}
		}
		return environ[index];
	}
#endif
}
