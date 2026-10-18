#include <oscompat/string.h>
#include <oscompat/common.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if OSCOMPAT_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

static void osMarkAllocFailure(void) {
#if OSCOMPAT_WINDOWS
	if (errno == 0) {
		os_set_system_error();
	}
#endif
}

static int osLowerByte(unsigned char c) {
	if (c >= (unsigned char)'A' && c <= (unsigned char)'Z') {
		return (int)c + ((int)'a' - (int)'A');
	}
	return (int)c;
}

static size_t osStrnlen(const char* s, size_t max) {
	const char* end = (const char*)memchr(s, '\0', max);
	if (end != NULL) {
		return (size_t)(end - s);
	}
	return max;
}

void* os_memmem(const void* hay, size_t hayLen, const void* needle, size_t needleLen) {
	const unsigned char* h = (const unsigned char*)hay;
	const unsigned char* n = (const unsigned char*)needle;
	size_t i;

	if (needleLen == 0) {
		return (void*)hay;
	}
	if (needleLen > hayLen) {
		return NULL;
	}
	for (i = 0; i + needleLen <= hayLen; i++) {
		if (h[i] == n[0] && memcmp(h + i + 1, n + 1, needleLen - 1) == 0) {
			return (void*)(h + i);
		}
	}
	return NULL;
}

void* os_memrchr(const void* s, int c, size_t n) {
	const unsigned char* p = (const unsigned char*)s;
	unsigned char target = (unsigned char)c;

	while (n > 0) {
		n--;
		if (p[n] == target) {
			return (void*)(p + n);
		}
	}
	return NULL;
}

char* os_strchrnul(const char* s, int c) {
	char target = (char)c;

	while (*s != '\0' && *s != target) {
		s++;
	}
	return (char*)s;
}

int os_strcasecmp(const char* a, const char* b) {
	const unsigned char* pa = (const unsigned char*)a;
	const unsigned char* pb = (const unsigned char*)b;

	for (;;) {
		int ca = osLowerByte(*pa);
		int cb = osLowerByte(*pb);
		if (ca != cb) {
			return ca - cb;
		}
		if (ca == 0) {
			return 0;
		}
		pa++;
		pb++;
	}
}

int os_strncasecmp(const char* a, const char* b, size_t n) {
	const unsigned char* pa = (const unsigned char*)a;
	const unsigned char* pb = (const unsigned char*)b;
	size_t i;

	for (i = 0; i < n; i++) {
		int ca = osLowerByte(pa[i]);
		int cb = osLowerByte(pb[i]);
		if (ca != cb) {
			return ca - cb;
		}
		if (ca == 0) {
			return 0;
		}
	}
	return 0;
}

char* os_strdup(const char* s) {
	size_t len = strlen(s);
	char* out = (char*)malloc(len + 1);

	if (out == NULL) {
		osMarkAllocFailure();
		return NULL;
	}
	memcpy(out, s, len + 1);
	return out;
}

char* os_strndup(const char* s, size_t n) {
	size_t len = osStrnlen(s, n);
	char* out = (char*)malloc(len + 1);

	if (out == NULL) {
		osMarkAllocFailure();
		return NULL;
	}
	memcpy(out, s, len);
	out[len] = '\0';
	return out;
}

size_t os_strlcpy(char* dst, const char* src, size_t size) {
	size_t srclen = strlen(src);

	if (size > 0) {
		size_t copylen = srclen;
		if (copylen > size - 1) {
			copylen = size - 1;
		}
		memcpy(dst, src, copylen);
		dst[copylen] = '\0';
	}
	return srclen;
}

size_t os_strlcat(char* dst, const char* src, size_t size) {
	size_t srclen = strlen(src);
	size_t dstlen = osStrnlen(dst, size);
	size_t copylen;

	if (dstlen == size) {
		return size + srclen;
	}
	copylen = size - dstlen - 1;
	if (copylen > srclen) {
		copylen = srclen;
	}
	memcpy(dst + dstlen, src, copylen);
	dst[dstlen + copylen] = '\0';
	return dstlen + srclen;
}

char* os_strsep(char** stringp, const char* delim) {
	char* start;
	char* end;

	if (stringp == NULL || *stringp == NULL) {
		return NULL;
	}
	start = *stringp;
	end = strpbrk(start, delim);
	if (end != NULL) {
		*end = '\0';
		*stringp = end + 1;
	} else {
		*stringp = NULL;
	}
	return start;
}

char* os_strtok_r(char* str, const char* delim, char** saveptr) {
	char* end;

	if (saveptr == NULL) {
		return NULL;
	}
	if (str == NULL) {
		str = *saveptr;
	}
	if (str == NULL) {
		return NULL;
	}
	str += strspn(str, delim);
	if (*str == '\0') {
		*saveptr = str;
		return NULL;
	}
	end = strpbrk(str, delim);
	if (end == NULL) {
		*saveptr = str + strlen(str);
	} else {
		*end = '\0';
		*saveptr = end + 1;
	}
	return str;
}

#if OSCOMPAT_WINDOWS

void os_explicit_bzero(void* p, size_t n) {
	SecureZeroMemory(p, n);
}

#else

void os_explicit_bzero(void* p, size_t n) {
	static void* (*volatile osMemsetPtr)(void*, int, size_t) = memset;
	osMemsetPtr(p, 0, n);
}

#endif
