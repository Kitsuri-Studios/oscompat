#if !defined(_WIN32)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif
#endif

#include "test_util.h"

#include <oscompat/dynlib.h>

#include <errno.h>
#include <stdio.h>

int main(void) {
	OsDynlib* lib = NULL;
	const char* symbolName;
	void* symbol;
	int opened = 0;

	TEST_ASSERT(os_dynlib_error() != NULL);

	TEST_ASSERT(os_dynlib_open("/nonexistent-oscompat-xyz.so") == NULL);

	errno = 0;
	TEST_ASSERT(os_dynlib_sym(NULL, "anything") == NULL);
	TEST_INT_EQ(EINVAL, errno);

	errno = 0;
	TEST_ASSERT(os_dynlib_close(NULL) == -1);
	TEST_INT_EQ(EINVAL, errno);

#if OSCOMPAT_WINDOWS
	symbolName = "GetLastError";
	lib = os_dynlib_open("kernel32.dll");
	opened = lib != NULL;
#else
	symbolName = "malloc";
	lib = os_dynlib_open(NULL);
	if (lib == NULL) {
		lib = os_dynlib_open("libc.so.6");
	}
	opened = lib != NULL;
#endif

	if (opened) {
		symbol = os_dynlib_sym(lib, symbolName);
		TEST_ASSERT(symbol != NULL);

		errno = 0;
		TEST_ASSERT(os_dynlib_sym(lib, NULL) == NULL);
		TEST_INT_EQ(EINVAL, errno);

		TEST_ASSERT(os_dynlib_sym(lib, "oscompat_no_such_symbol_xyz") == NULL);
		TEST_ASSERT(os_dynlib_error() != NULL);
		TEST_INT_EQ(0, os_dynlib_close(lib));
	} else {
		printf("SKIP dynlib: no self handle or system library could be opened\n");
	}

	return os_test_summary("dynlib");
}
