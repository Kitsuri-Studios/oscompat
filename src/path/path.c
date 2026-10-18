#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <oscompat/path.h>
#include <oscompat/common.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if OSCOMPAT_WINDOWS
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

#if OSCOMPAT_WINDOWS
#define OSCOMPAT_PATH_SEP '\\'
#else
#define OSCOMPAT_PATH_SEP '/'
#endif

static int osIsPathSep(int c) {
#if OSCOMPAT_WINDOWS
	return c == '/' || c == '\\';
#else
	return c == '/';
#endif
}

static int osCopyString(char* buf, size_t cap, const char* value) {
	size_t len = strlen(value) + 1;
	if (len > cap) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(buf, value, len);
	return 0;
}

char* os_realpath(const char* path, char* resolved, size_t cap) {
	char tmp[OSCOMPAT_PATH_MAX];
	size_t len;
	if (path == NULL) {
		errno = EINVAL;
		return NULL;
	}
	if (resolved != NULL && cap == 0) {
		errno = EINVAL;
		return NULL;
	}
#if OSCOMPAT_WINDOWS
	{
		HANDLE handle;
		DWORD n;
		handle = CreateFileA(path, 0,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			os_set_system_error();
			return NULL;
		}
		n = GetFinalPathNameByHandleA(handle, tmp, (DWORD)OSCOMPAT_PATH_MAX,
			FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		CloseHandle(handle);
		if (n == 0) {
			os_set_system_error();
			return NULL;
		}
		if (n >= (DWORD)OSCOMPAT_PATH_MAX) {
			errno = ENAMETOOLONG;
			return NULL;
		}
		if (strncmp(tmp, "\\\\?\\UNC\\", 8) == 0) {
			memmove(tmp + 2, tmp + 8, strlen(tmp + 8) + 1);
			tmp[0] = '\\';
			tmp[1] = '\\';
		} else if (strncmp(tmp, "\\\\?\\", 4) == 0) {
			memmove(tmp, tmp + 4, strlen(tmp + 4) + 1);
		}
	}
#else
	if (realpath(path, tmp) == NULL) {
		return NULL;
	}
#endif
	len = strlen(tmp) + 1;
	if (resolved == NULL) {
		char* out = (char*)malloc(len);
		if (out == NULL) {
			errno = ENOMEM;
			return NULL;
		}
		memcpy(out, tmp, len);
		return out;
	}
	if (len > cap) {
		errno = ENAMETOOLONG;
		return NULL;
	}
	memcpy(resolved, tmp, len);
	return resolved;
}

int os_exe_path(char* buf, size_t cap) {
	char tmp[OSCOMPAT_PATH_MAX];
	size_t len;
	if (buf == NULL || cap == 0) {
		errno = EINVAL;
		return -1;
	}
#if OSCOMPAT_WINDOWS
	{
		DWORD n = GetModuleFileNameA(NULL, tmp, (DWORD)OSCOMPAT_PATH_MAX);
		if (n == 0) {
			os_set_system_error();
			return -1;
		}
		if (n >= (DWORD)OSCOMPAT_PATH_MAX) {
			errno = ENAMETOOLONG;
			return -1;
		}
		tmp[n] = '\0';
	}
#elif defined(__linux__)
	{
		ssize_t n = readlink("/proc/self/exe", tmp, sizeof(tmp) - 1);
		if (n < 0) {
			return -1;
		}
		if ((size_t)n >= sizeof(tmp) - 1) {
			errno = ENAMETOOLONG;
			return -1;
		}
		tmp[n] = '\0';
	}
#else
	errno = ENOSYS;
	return -1;
#endif
	len = strlen(tmp) + 1;
	if (len > cap) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(buf, tmp, len);
	return 0;
}

int os_exe_dir(char* buf, size_t cap) {
	char tmp[OSCOMPAT_PATH_MAX];
	if (os_exe_path(tmp, sizeof(tmp)) != 0) {
		return -1;
	}
	return os_path_dirname(tmp, buf, cap);
}

int os_path_is_absolute(const char* path) {
	if (path == NULL || path[0] == '\0') {
		return 0;
	}
#if OSCOMPAT_WINDOWS
	if (path[0] == '/' || path[0] == '\\') {
		return 1;
	}
	if (((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))
		&& path[1] == ':' && (path[2] == '/' || path[2] == '\\')) {
		return 1;
	}
	return 0;
#else
	return path[0] == '/';
#endif
}

int os_path_join(char* buf, size_t cap, const char* base, const char* part) {
	size_t blen;
	size_t plen;
	if (buf == NULL || base == NULL || part == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (os_path_is_absolute(part)) {
		return osCopyString(buf, cap, part);
	}
	blen = strlen(base);
	plen = strlen(part);
	if (blen + plen + 2 > cap) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(buf, base, blen);
	if (blen > 0 && !osIsPathSep(base[blen - 1])) {
		buf[blen] = OSCOMPAT_PATH_SEP;
		blen++;
	}
	memcpy(buf + blen, part, plen + 1);
	return 0;
}

int os_path_normalize(char* buf, size_t cap, const char* path) {
	char out[OSCOMPAT_PATH_MAX];
	size_t compSep[OSCOMPAT_PATH_MAX / 2 + 2];
	unsigned char isDotDot[OSCOMPAT_PATH_MAX / 2 + 2];
	size_t rootLen = 0;
	size_t outLen = 0;
	size_t ncomp = 0;
	int absolute = 0;
	const char* p;

	if (buf == NULL || path == NULL) {
		errno = EINVAL;
		return -1;
	}
#if OSCOMPAT_WINDOWS
	p = path;
	if (osIsPathSep(p[0]) && osIsPathSep(p[1])) {
		const char* q = p + 2;
		absolute = 1;
		out[outLen++] = OSCOMPAT_PATH_SEP;
		out[outLen++] = OSCOMPAT_PATH_SEP;
		while (*q != '\0' && !osIsPathSep(*q)) {
			out[outLen++] = *q++;
		}
		if (osIsPathSep(*q)) {
			q++;
			out[outLen++] = OSCOMPAT_PATH_SEP;
			while (*q != '\0' && !osIsPathSep(*q)) {
				out[outLen++] = *q++;
			}
		}
		rootLen = outLen;
		p = q;
	} else if (((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z')) && p[1] == ':') {
		absolute = 1;
		out[outLen++] = p[0];
		out[outLen++] = ':';
		p += 2;
		if (osIsPathSep(*p)) {
			out[outLen++] = OSCOMPAT_PATH_SEP;
			p++;
		}
		rootLen = outLen;
	} else if (osIsPathSep(*p)) {
		absolute = 1;
		out[outLen++] = OSCOMPAT_PATH_SEP;
		rootLen = outLen;
		while (osIsPathSep(*p)) {
			p++;
		}
	}
#else
	p = path;
	if (*p == '/') {
		absolute = 1;
		out[outLen++] = '/';
		if (p[1] == '/' && p[2] != '/') {
			out[outLen++] = '/';
		}
		rootLen = outLen;
		while (*p == '/') {
			p++;
		}
	}
#endif
	while (*p != '\0') {
		const char* start;
		size_t clen;
		while (*p != '\0' && osIsPathSep(*p)) {
			p++;
		}
		if (*p == '\0') {
			break;
		}
		start = p;
		while (*p != '\0' && !osIsPathSep(*p)) {
			p++;
		}
		clen = (size_t)(p - start);
		if (clen == 1 && start[0] == '.') {
			continue;
		}
		if (clen == 2 && start[0] == '.' && start[1] == '.') {
			if (ncomp > 0 && !isDotDot[ncomp - 1]) {
				ncomp--;
				outLen = compSep[ncomp];
				continue;
			}
			if (absolute) {
				continue;
			}
		}
		{
			size_t sepPos;
			if (ncomp == 0 && (rootLen == 0 || out[rootLen - 1] == OSCOMPAT_PATH_SEP)) {
				sepPos = outLen;
			} else {
				sepPos = outLen;
				out[outLen++] = OSCOMPAT_PATH_SEP;
			}
			if (outLen + clen + 1 > sizeof(out)) {
				errno = ENAMETOOLONG;
				return -1;
			}
			memcpy(out + outLen, start, clen);
			compSep[ncomp] = sepPos;
			isDotDot[ncomp] = (unsigned char)(clen == 2 && start[0] == '.');
			ncomp++;
			outLen += clen;
		}
	}
	if (outLen == 0) {
		out[outLen++] = '.';
	}
	if (outLen + 1 > cap) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(buf, out, outLen);
	buf[outLen] = '\0';
	return 0;
}

int os_path_dirname(const char* path, char* buf, size_t cap) {
	size_t len;
	size_t end;
	size_t i;
	size_t slash = 0;
	int hasSlash = 0;
	if (path == NULL || buf == NULL) {
		errno = EINVAL;
		return -1;
	}
	len = strlen(path);
	if (len == 0) {
		return osCopyString(buf, cap, ".");
	}
	end = len;
	while (end > 1 && osIsPathSep(path[end - 1])) {
		end--;
	}
	if (end == 1 && osIsPathSep(path[0])) {
		return osCopyString(buf, cap, "/");
	}
	for (i = 0; i < end; i++) {
		if (osIsPathSep(path[i])) {
			slash = i;
			hasSlash = 1;
		}
	}
	if (!hasSlash) {
		return osCopyString(buf, cap, ".");
	}
	while (slash > 0 && osIsPathSep(path[slash - 1])) {
		slash--;
	}
	if (slash == 0) {
		return osCopyString(buf, cap, "/");
	}
	if (slash + 1 > cap) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(buf, path, slash);
	buf[slash] = '\0';
	return 0;
}

int os_path_basename(const char* path, char* buf, size_t cap) {
	size_t len;
	size_t end;
	size_t i;
	size_t start = 0;
	if (path == NULL || buf == NULL) {
		errno = EINVAL;
		return -1;
	}
	len = strlen(path);
	if (len == 0) {
		return osCopyString(buf, cap, ".");
	}
	end = len;
	while (end > 1 && osIsPathSep(path[end - 1])) {
		end--;
	}
	if (end == 1 && osIsPathSep(path[0])) {
		return osCopyString(buf, cap, "/");
	}
	for (i = 0; i < end; i++) {
		if (osIsPathSep(path[i])) {
			start = i + 1;
		}
	}
	if (end - start + 1 > cap) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(buf, path + start, end - start);
	buf[end - start] = '\0';
	return 0;
}

int os_home_dir(char* buf, size_t cap) {
	if (buf == NULL || cap == 0) {
		errno = EINVAL;
		return -1;
	}
#if OSCOMPAT_WINDOWS
	{
		char tmp[OSCOMPAT_PATH_MAX];
		DWORD n = GetEnvironmentVariableA("USERPROFILE", tmp, (DWORD)OSCOMPAT_PATH_MAX);
		if (n == 0 || n >= (DWORD)OSCOMPAT_PATH_MAX) {
			const char* drive = getenv("HOMEDRIVE");
			const char* hpath = getenv("HOMEPATH");
			if (drive == NULL || hpath == NULL) {
				errno = ENOENT;
				return -1;
			}
			if (strlen(drive) + strlen(hpath) + 1 > sizeof(tmp)) {
				errno = ENAMETOOLONG;
				return -1;
			}
			strcpy(tmp, drive);
			strcat(tmp, hpath);
		}
		return osCopyString(buf, cap, tmp);
	}
#else
	{
		const char* home = getenv("HOME");
		if (home == NULL || home[0] == '\0') {
			struct passwd* pw = getpwuid(getuid());
			if (pw == NULL || pw->pw_dir == NULL || pw->pw_dir[0] == '\0') {
				errno = ENOENT;
				return -1;
			}
			return osCopyString(buf, cap, pw->pw_dir);
		}
		return osCopyString(buf, cap, home);
	}
#endif
}

int os_temp_dir(char* buf, size_t cap) {
	char tmp[OSCOMPAT_PATH_MAX];
	size_t len;
	if (buf == NULL || cap == 0) {
		errno = EINVAL;
		return -1;
	}
#if OSCOMPAT_WINDOWS
	{
		DWORD n = GetTempPathA((DWORD)OSCOMPAT_PATH_MAX, tmp);
		if (n == 0 || n >= (DWORD)OSCOMPAT_PATH_MAX) {
			os_set_system_error();
			return -1;
		}
		tmp[n] = '\0';
	}
#else
	{
		const char* dir = getenv("TMPDIR");
		if (dir == NULL || dir[0] == '\0') {
			dir = "/tmp";
		}
		if (strlen(dir) + 1 > sizeof(tmp)) {
			errno = ENAMETOOLONG;
			return -1;
		}
		strcpy(tmp, dir);
	}
#endif
	len = strlen(tmp);
	while (len > 1 && osIsPathSep(tmp[len - 1])) {
		len--;
	}
	tmp[len] = '\0';
	return osCopyString(buf, cap, tmp);
}
