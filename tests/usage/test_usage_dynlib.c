#if !defined(_WIN32)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif
#endif

#include "test_util.h"

#include <oscompat/dynlib.h>

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
	OsDynlib* lib;
	int (*add)(int, int);
	const char* (*message)(void);

	if (argc < 2) {
		printf("SKIP usage_dynlib: plugin path not provided\n");
		return os_test_summary("usage_dynlib");
	}

	TEST_ASSERT(os_dynlib_open("/nonexistent-oscompat-plugin") == NULL);

	lib = os_dynlib_open(argv[1]);
	TEST_ASSERT(lib != NULL);
	if (lib == NULL) {
		return os_test_summary("usage_dynlib");
	}

	add = (int (*)(int, int))os_dynlib_sym(lib, "oscompat_usage_add");
	TEST_ASSERT(add != NULL);
	if (add != NULL) {
		TEST_INT_EQ(7, add(3, 4));
		TEST_INT_EQ(0, add(-1, 1));
	}

	message = (const char* (*)(void))os_dynlib_sym(lib, "oscompat_usage_message");
	TEST_ASSERT(message != NULL);
	if (message != NULL) {
		TEST_STR_EQ("oscompat", message());
	}

	TEST_ASSERT(os_dynlib_sym(lib, "oscompat_usage_missing") == NULL);
	TEST_INT_EQ(0, os_dynlib_close(lib));

	return os_test_summary("usage_dynlib");
}
