#include <oscompat/common.h>

#include <errno.h>

#if OSCOMPAT_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static int osErrnoFromWin32(DWORD code) {
	switch (code) {
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_INVALID_DRIVE:
		return ENOENT;
	case ERROR_ACCESS_DENIED:
	case ERROR_SHARING_VIOLATION:
	case ERROR_LOCK_VIOLATION:
		return EACCES;
	case ERROR_FILE_EXISTS:
	case ERROR_ALREADY_EXISTS:
		return EEXIST;
	case ERROR_INVALID_PARAMETER:
		return EINVAL;
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:
		return ENOMEM;
	case ERROR_DIRECTORY:
		return ENOTDIR;
	case ERROR_DISK_FULL:
		return ENOSPC;
	case ERROR_DIR_NOT_EMPTY:
		return ENOTEMPTY;
	case ERROR_BROKEN_PIPE:
	case ERROR_NO_DATA:
		return EPIPE;
	case ERROR_OPERATION_ABORTED:
		return EINTR;
	case ERROR_FILENAME_EXCED_RANGE:
		return ENAMETOOLONG;
	case ERROR_NOT_SUPPORTED:
		return ENOSYS;
	default:
		return EIO;
	}
}

void os_set_system_error(void) {
	errno = osErrnoFromWin32(GetLastError());
}

#else

void os_set_system_error(void) {
}

#endif
