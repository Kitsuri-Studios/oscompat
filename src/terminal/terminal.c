#if !defined(_WIN32)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#endif

#include <oscompat/common.h>
#include <oscompat/terminal.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if OSCOMPAT_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>

int os_isatty(int fd) {
	return _isatty(fd) ? 1 : 0;
}

int os_tty_size(int fd, int* cols, int* rows) {
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE handle;
	if (cols == NULL || rows == NULL) {
		errno = EINVAL;
		return -1;
	}
	handle = (HANDLE)_get_osfhandle(fd);
	if (handle == INVALID_HANDLE_VALUE) {
		os_set_system_error();
		return -1;
	}
	if (!GetConsoleScreenBufferInfo(handle, &info)) {
		os_set_system_error();
		return -1;
	}
	*cols = (int)(info.srWindow.Right - info.srWindow.Left + 1);
	*rows = (int)(info.srWindow.Bottom - info.srWindow.Top + 1);
	return 0;
}

#else

#include <unistd.h>
#include <sys/ioctl.h>

int os_isatty(int fd) {
	return isatty(fd) ? 1 : 0;
}

int os_tty_size(int fd, int* cols, int* rows) {
	struct winsize ws;
	if (cols == NULL || rows == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (ioctl(fd, TIOCGWINSZ, &ws) != 0) {
		return -1;
	}
	*cols = (int)ws.ws_col;
	*rows = (int)ws.ws_row;
	return 0;
}

#endif

int os_color_enabled(int fd) {
	const char* noColor;
	const char* term;
	if (!os_isatty(fd)) {
		return 0;
	}
	noColor = getenv("NO_COLOR");
	if (noColor != NULL && noColor[0] != '\0') {
		return 0;
	}
	term = getenv("TERM");
	if (term != NULL && strcmp(term, "dumb") == 0) {
		return 0;
	}
	return 1;
}
