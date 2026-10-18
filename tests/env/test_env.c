#include "test_util.h"

#include <oscompat/env.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void testRoundTrip(void) {
	TEST_INT_EQ(0, os_setenv("OSCOMPAT_TEST_VAR", "alpha", 1));
	TEST_ASSERT(os_getenv("OSCOMPAT_TEST_VAR") != NULL);
	TEST_STR_EQ("alpha", os_getenv("OSCOMPAT_TEST_VAR"));
	TEST_INT_EQ(0, os_setenv("OSCOMPAT_TEST_VAR", "beta", 0));
	TEST_STR_EQ("alpha", os_getenv("OSCOMPAT_TEST_VAR"));
	TEST_INT_EQ(0, os_setenv("OSCOMPAT_TEST_VAR", "beta", 1));
	TEST_STR_EQ("beta", os_getenv("OSCOMPAT_TEST_VAR"));
	TEST_INT_EQ(0, os_unsetenv("OSCOMPAT_TEST_VAR"));
	TEST_ASSERT(os_getenv("OSCOMPAT_TEST_VAR") == NULL);
}

static void testMissing(void) {
	TEST_ASSERT(os_getenv("OSCOMPAT_TEST_MISSING_VAR") == NULL);
	TEST_INT_EQ(0, os_unsetenv("OSCOMPAT_TEST_MISSING_VAR"));
}

static void testEnviron(void) {
	int count = os_environ_count();
	int i;
	TEST_ASSERT(count >= 1);
	for (i = 0; i < count; i++) {
		const char* entry = os_environ_get(i);
		TEST_ASSERT(entry != NULL);
		if (entry != NULL) {
			TEST_ASSERT(strchr(entry, '=') != NULL);
		}
	}
	TEST_ASSERT(os_environ_get(-1) == NULL);
	TEST_ASSERT(os_environ_get(count) == NULL);
	TEST_ASSERT(os_environ_get(count + 1000) == NULL);
}

int main(void) {
	testRoundTrip();
	testMissing();
	testEnviron();
	return os_test_summary("env");
}
