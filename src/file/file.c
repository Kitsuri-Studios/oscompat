#if !defined(_WIN32)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#endif

#include <oscompat/common.h>
#include <oscompat/file.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OSCOMPAT_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int os_file_exists(const char* path) {
	DWORD attr;
	if (path == NULL) {
		errno = EINVAL;
		return 0;
	}
	attr = GetFileAttributesA(path);
	if (attr == INVALID_FILE_ATTRIBUTES) {
		os_set_system_error();
		return 0;
	}
	return 1;
}

int os_is_file(const char* path) {
	DWORD attr;
	if (path == NULL) {
		errno = EINVAL;
		return 0;
	}
	attr = GetFileAttributesA(path);
	if (attr == INVALID_FILE_ATTRIBUTES) {
		os_set_system_error();
		return 0;
	}
	if (attr & FILE_ATTRIBUTE_DIRECTORY) {
		return 0;
	}
	return 1;
}

int os_is_dir(const char* path) {
	DWORD attr;
	if (path == NULL) {
		errno = EINVAL;
		return 0;
	}
	attr = GetFileAttributesA(path);
	if (attr == INVALID_FILE_ATTRIBUTES) {
		os_set_system_error();
		return 0;
	}
	if (attr & FILE_ATTRIBUTE_DIRECTORY) {
		return 1;
	}
	return 0;
}

int os_read_file(const char* path, char** data, size_t* size) {
	HANDLE handle;
	LARGE_INTEGER length;
	char* buf;
	size_t total;
	size_t off;

	if (path == NULL || data == NULL || size == NULL) {
		errno = EINVAL;
		return -1;
	}
	*data = NULL;
	*size = 0;
	handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		os_set_system_error();
		return -1;
	}
	if (!GetFileSizeEx(handle, &length)) {
		os_set_system_error();
		CloseHandle(handle);
		return -1;
	}
	if (length.QuadPart < 0 || (uint64_t)length.QuadPart > (uint64_t)SIZE_MAX - 1) {
		CloseHandle(handle);
		errno = EOVERFLOW;
		return -1;
	}
	total = (size_t)length.QuadPart;
	buf = (char*)malloc(total + 1);
	if (buf == NULL) {
		CloseHandle(handle);
		errno = ENOMEM;
		return -1;
	}
	off = 0;
	while (off < total) {
		DWORD chunk;
		DWORD got = 0;
		if (total - off > 0x40000000u) {
			chunk = 0x40000000u;
		} else {
			chunk = (DWORD)(total - off);
		}
		if (!ReadFile(handle, buf + off, chunk, &got, NULL)) {
			os_set_system_error();
			free(buf);
			CloseHandle(handle);
			return -1;
		}
		if (got == 0) {
			break;
		}
		off += got;
	}
	buf[off] = '\0';
	CloseHandle(handle);
	*data = buf;
	*size = off;
	return 0;
}

int os_write_file(const char* path, const void* data, size_t size) {
	HANDLE handle;
	size_t off;

	if (path == NULL || (data == NULL && size > 0)) {
		errno = EINVAL;
		return -1;
	}
	handle = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		os_set_system_error();
		return -1;
	}
	off = 0;
	while (off < size) {
		DWORD chunk;
		DWORD written = 0;
		if (size - off > 0x40000000u) {
			chunk = 0x40000000u;
		} else {
			chunk = (DWORD)(size - off);
		}
		if (!WriteFile(handle, (const char*)data + off, chunk, &written, NULL)) {
			os_set_system_error();
			CloseHandle(handle);
			return -1;
		}
		if (written == 0) {
			CloseHandle(handle);
			errno = EIO;
			return -1;
		}
		off += written;
	}
	CloseHandle(handle);
	return 0;
}

int os_mkdir_p(const char* path) {
	char* copy;
	size_t len;
	size_t start = 0;
	size_t i;

	if (path == NULL || path[0] == '\0') {
		errno = EINVAL;
		return -1;
	}
	len = strlen(path);
	copy = (char*)malloc(len + 1);
	if (copy == NULL) {
		errno = ENOMEM;
		return -1;
	}
	memcpy(copy, path, len + 1);
	if (len >= 2 && (copy[0] == '/' || copy[0] == '\\') && (copy[1] == '/' || copy[1] == '\\')) {
		size_t k = 2;
		while (copy[k] != '\0' && copy[k] != '/' && copy[k] != '\\') {
			k++;
		}
		if (copy[k] != '\0') {
			k++;
			while (copy[k] != '\0' && copy[k] != '/' && copy[k] != '\\') {
				k++;
			}
		}
		start = k;
	}
	for (i = start + 1; i <= len; i++) {
		if (copy[i] == '/' || copy[i] == '\\' || copy[i] == '\0') {
			char saved = copy[i];
			copy[i] = '\0';
			if (copy[0] != '\0' && copy[i - 1] != ':') {
				DWORD attr = GetFileAttributesA(copy);
				if (attr == INVALID_FILE_ATTRIBUTES) {
					if (!CreateDirectoryA(copy, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
						os_set_system_error();
						free(copy);
						return -1;
					}
				} else if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
					free(copy);
					errno = ENOTDIR;
					return -1;
				}
			}
			copy[i] = saved;
		}
	}
	free(copy);
	return 0;
}

static void osClearReadonly(const char* path) {
	DWORD attr = GetFileAttributesA(path);
	if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_READONLY)) {
		SetFileAttributesA(path, attr & ~(DWORD)FILE_ATTRIBUTE_READONLY);
	}
}

int os_remove_recursive(const char* path) {
	DWORD attr;

	if (path == NULL) {
		errno = EINVAL;
		return -1;
	}
	attr = GetFileAttributesA(path);
	if (attr == INVALID_FILE_ATTRIBUTES) {
		os_set_system_error();
		return -1;
	}
	if (attr & FILE_ATTRIBUTE_DIRECTORY) {
		char pattern[OSCOMPAT_PATH_MAX];
		WIN32_FIND_DATAA entry;
		HANDLE find;
		if (snprintf(pattern, sizeof(pattern), "%s\\*", path) >= (int)sizeof(pattern)) {
			errno = ENAMETOOLONG;
			return -1;
		}
		find = FindFirstFileA(pattern, &entry);
		if (find != INVALID_HANDLE_VALUE) {
			do {
				char child[OSCOMPAT_PATH_MAX];
				if (strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0) {
					continue;
				}
				if (snprintf(child, sizeof(child), "%s\\%s", path, entry.cFileName) >= (int)sizeof(child)) {
					FindClose(find);
					errno = ENAMETOOLONG;
					return -1;
				}
				if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (os_remove_recursive(child) != 0) {
						FindClose(find);
						return -1;
					}
				} else {
					osClearReadonly(child);
					if (!DeleteFileA(child)) {
						os_set_system_error();
						FindClose(find);
						return -1;
					}
				}
			} while (FindNextFileA(find, &entry));
			FindClose(find);
		} else if (GetLastError() != ERROR_NO_MORE_FILES && GetLastError() != ERROR_FILE_NOT_FOUND) {
			os_set_system_error();
			return -1;
		}
		osClearReadonly(path);
		if (!RemoveDirectoryA(path)) {
			os_set_system_error();
			return -1;
		}
		return 0;
	}
	osClearReadonly(path);
	if (!DeleteFileA(path)) {
		os_set_system_error();
		return -1;
	}
	return 0;
}

int os_temp_file(char* buf, size_t cap) {
	char tmpDir[MAX_PATH + 1];
	char name[MAX_PATH + 1];
	DWORD len;

	if (buf == NULL || cap == 0) {
		errno = EINVAL;
		return -1;
	}
	len = GetTempPathA((DWORD)sizeof(tmpDir), tmpDir);
	if (len == 0 || len > (DWORD)sizeof(tmpDir)) {
		os_set_system_error();
		return -1;
	}
	if (GetTempFileNameA(tmpDir, "osc", 0, name) == 0) {
		os_set_system_error();
		return -1;
	}
	len = GetFullPathNameA(name, (DWORD)cap, buf, NULL);
	if (len == 0 || len >= (DWORD)cap) {
		DeleteFileA(name);
		errno = ERANGE;
		return -1;
	}
	return 0;
}

#else

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int os_file_exists(const char* path) {
	struct stat info;
	if (path == NULL) {
		errno = EINVAL;
		return 0;
	}
	return stat(path, &info) == 0 ? 1 : 0;
}

int os_is_file(const char* path) {
	struct stat info;
	if (path == NULL) {
		errno = EINVAL;
		return 0;
	}
	if (stat(path, &info) != 0) {
		return 0;
	}
	return S_ISREG(info.st_mode) ? 1 : 0;
}

int os_is_dir(const char* path) {
	struct stat info;
	if (path == NULL) {
		errno = EINVAL;
		return 0;
	}
	if (stat(path, &info) != 0) {
		return 0;
	}
	return S_ISDIR(info.st_mode) ? 1 : 0;
}

int os_read_file(const char* path, char** data, size_t* size) {
	FILE* fp;
	char* buf;
	size_t cap;
	size_t len;

	if (path == NULL || data == NULL || size == NULL) {
		errno = EINVAL;
		return -1;
	}
	*data = NULL;
	*size = 0;
	fp = fopen(path, "rb");
	if (fp == NULL) {
		return -1;
	}
	cap = 4096;
	len = 0;
	buf = (char*)malloc(cap + 1);
	if (buf == NULL) {
		fclose(fp);
		errno = ENOMEM;
		return -1;
	}
	for (;;) {
		size_t got;
		if (len == cap) {
			size_t nextCap = cap * 2;
			char* grown = (char*)realloc(buf, nextCap + 1);
			if (grown == NULL) {
				free(buf);
				fclose(fp);
				errno = ENOMEM;
				return -1;
			}
			buf = grown;
			cap = nextCap;
		}
		got = fread(buf + len, 1, cap - len, fp);
		len += got;
		if (got == 0) {
			if (ferror(fp)) {
				int saved = errno;
				free(buf);
				fclose(fp);
				errno = saved != 0 ? saved : EIO;
				return -1;
			}
			break;
		}
	}
	buf[len] = '\0';
	fclose(fp);
	*data = buf;
	*size = len;
	return 0;
}

int os_write_file(const char* path, const void* data, size_t size) {
	FILE* fp;

	if (path == NULL || (data == NULL && size > 0)) {
		errno = EINVAL;
		return -1;
	}
	fp = fopen(path, "wb");
	if (fp == NULL) {
		return -1;
	}
	if (size > 0 && fwrite(data, 1, size, fp) != size) {
		int saved = errno;
		fclose(fp);
		errno = saved != 0 ? saved : EIO;
		return -1;
	}
	if (fclose(fp) != 0) {
		return -1;
	}
	return 0;
}

int os_mkdir_p(const char* path) {
	char* copy;
	size_t len;
	size_t i;
	struct stat info;

	if (path == NULL || path[0] == '\0') {
		errno = EINVAL;
		return -1;
	}
	len = strlen(path);
	copy = (char*)malloc(len + 1);
	if (copy == NULL) {
		errno = ENOMEM;
		return -1;
	}
	memcpy(copy, path, len + 1);
	for (i = 1; i <= len; i++) {
		if (copy[i] == '/' || copy[i] == '\0') {
			char saved = copy[i];
			copy[i] = '\0';
			if (copy[0] != '\0') {
				if (stat(copy, &info) != 0) {
					if (mkdir(copy, 0777) != 0 && errno != EEXIST) {
						int savedErr = errno;
						free(copy);
						errno = savedErr;
						return -1;
					}
				} else if (!S_ISDIR(info.st_mode)) {
					free(copy);
					errno = ENOTDIR;
					return -1;
				}
			}
			copy[i] = saved;
		}
	}
	free(copy);
	return 0;
}

int os_remove_recursive(const char* path) {
	struct stat info;

	if (path == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (lstat(path, &info) != 0) {
		return -1;
	}
	if (S_ISDIR(info.st_mode)) {
		DIR* dir = opendir(path);
		struct dirent* entry;
		size_t plen;
		if (dir == NULL) {
			return -1;
		}
		plen = strlen(path);
		while ((entry = readdir(dir)) != NULL) {
			char* child;
			size_t nlen;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
			nlen = strlen(entry->d_name);
			child = (char*)malloc(plen + nlen + 2);
			if (child == NULL) {
				closedir(dir);
				errno = ENOMEM;
				return -1;
			}
			memcpy(child, path, plen);
			if (plen > 0 && path[plen - 1] == '/') {
				memcpy(child + plen, entry->d_name, nlen + 1);
			} else {
				child[plen] = '/';
				memcpy(child + plen + 1, entry->d_name, nlen + 1);
			}
			if (os_remove_recursive(child) != 0) {
				int saved = errno;
				free(child);
				closedir(dir);
				errno = saved;
				return -1;
			}
			free(child);
		}
		closedir(dir);
		if (rmdir(path) != 0) {
			return -1;
		}
		return 0;
	}
	if (unlink(path) != 0) {
		return -1;
	}
	return 0;
}

static int osAbsolutePath(const char* path, char* out, size_t cap) {
	char cwd[OSCOMPAT_PATH_MAX];
	size_t len;
	if (path[0] == '/') {
		len = strlen(path);
		if (len + 1 > cap) {
			errno = ERANGE;
			return -1;
		}
		memcpy(out, path, len + 1);
		return 0;
	}
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		return -1;
	}
	if (snprintf(out, cap, "%s/%s", cwd, path) >= (int)cap) {
		errno = ERANGE;
		return -1;
	}
	return 0;
}

int os_temp_file(char* buf, size_t cap) {
	const char* dir;
	char tmpl[OSCOMPAT_PATH_MAX];
	char abs[OSCOMPAT_PATH_MAX];
	int fd;

	if (buf == NULL || cap == 0) {
		errno = EINVAL;
		return -1;
	}
	dir = getenv("TMPDIR");
	if (dir == NULL || dir[0] == '\0') {
		dir = "/tmp";
	}
	if (snprintf(tmpl, sizeof(tmpl), "%s/oscompat_XXXXXX", dir) >= (int)sizeof(tmpl)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	fd = mkstemp(tmpl);
	if (fd < 0) {
		return -1;
	}
	if (close(fd) != 0) {
		int saved = errno;
		unlink(tmpl);
		errno = saved;
		return -1;
	}
	if (osAbsolutePath(tmpl, abs, sizeof(abs)) != 0) {
		int saved = errno;
		unlink(tmpl);
		errno = saved;
		return -1;
	}
	if (strlen(abs) + 1 > cap) {
		unlink(abs);
		errno = ERANGE;
		return -1;
	}
	memcpy(buf, abs, strlen(abs) + 1);
	return 0;
}

#endif
