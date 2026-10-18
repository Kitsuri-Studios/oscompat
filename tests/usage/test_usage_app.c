#include "test_util.h"

#include <oscompat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int osMakeScratch(char* buf, size_t cap) {
	char seed[OSCOMPAT_PATH_MAX];
	size_t len;
	if (os_temp_file(seed, sizeof(seed)) != 0) {
		return -1;
	}
	if (os_remove_recursive(seed) != 0) {
		return -1;
	}
	if (os_mkdir_p(seed) != 0) {
		return -1;
	}
	len = strlen(seed);
	if (len + 1 > cap) {
		os_remove_recursive(seed);
		return -1;
	}
	memcpy(buf, seed, len + 1);
	return 0;
}

static void testFileRoundTrip(void) {
	char scratch[OSCOMPAT_PATH_MAX];
	char nested[OSCOMPAT_PATH_MAX];
	char file[OSCOMPAT_PATH_MAX];
	char* data = NULL;
	size_t size = 0;

	TEST_INT_EQ(0, osMakeScratch(scratch, sizeof(scratch)));
	TEST_INT_EQ(1, os_is_dir(scratch));

	TEST_INT_EQ(0, os_path_join(nested, sizeof(nested), scratch, "config"));
	TEST_INT_EQ(0, os_mkdir_p(nested));
	TEST_INT_EQ(1, os_is_dir(nested));
	TEST_INT_EQ(0, os_mkdir_p(nested));

	TEST_INT_EQ(0, os_path_join(file, sizeof(file), nested, "app.conf"));
	TEST_INT_EQ(0, os_write_file(file, "key=value\n", 10));
	TEST_INT_EQ(1, os_is_file(file));

	TEST_INT_EQ(0, os_read_file(file, &data, &size));
	TEST_ASSERT(data != NULL);
	TEST_INT_EQ(10, (long long)size);
	TEST_STR_EQ("key=value\n", data);
	free(data);

	TEST_INT_EQ(0, os_remove_recursive(scratch));
	TEST_INT_EQ(0, os_file_exists(scratch));
}

static void testPathUsage(void) {
	char exe[OSCOMPAT_PATH_MAX];
	char dir[OSCOMPAT_PATH_MAX];
	char joined[OSCOMPAT_PATH_MAX];
	char normalized[OSCOMPAT_PATH_MAX];
	char absolute[OSCOMPAT_PATH_MAX];

	TEST_INT_EQ(0, os_exe_path(exe, sizeof(exe)));
	TEST_ASSERT(os_path_is_absolute(exe));
	TEST_INT_EQ(0, os_exe_dir(dir, sizeof(dir)));
	TEST_ASSERT(os_path_is_absolute(dir));
	TEST_ASSERT(strlen(dir) < strlen(exe));

	TEST_INT_EQ(0, os_path_join(joined, sizeof(joined), dir, "bin"));
	TEST_ASSERT(os_path_is_absolute(joined));

	TEST_INT_EQ(0, os_path_normalize(normalized, sizeof(normalized), "a/./b/../c"));
#if OSCOMPAT_WINDOWS
	TEST_STR_EQ("a\\c", normalized);
#else
	TEST_STR_EQ("a/c", normalized);
#endif

	TEST_INT_EQ(0, os_path_normalize(absolute, sizeof(absolute), "/x/y/../z"));
	TEST_ASSERT(absolute[0] == '/' || absolute[0] == '\\');
	TEST_ASSERT(os_path_is_absolute(absolute));
}

static void testEnvUsage(void) {
	int i;
	int count;
	int found = 0;

	TEST_INT_EQ(0, os_setenv("OSCOMPAT_USAGE_VAR", "on", 1));
	TEST_STR_EQ("on", os_getenv("OSCOMPAT_USAGE_VAR"));

	count = os_environ_count();
	TEST_ASSERT(count >= 1);
	for (i = 0; i < count; i++) {
		const char* entry = os_environ_get(i);
		TEST_ASSERT(entry != NULL);
		if (entry != NULL && strncmp(entry, "OSCOMPAT_USAGE_VAR=", 19) == 0) {
			found = 1;
		}
	}
	TEST_ASSERT(found);

	TEST_INT_EQ(0, os_unsetenv("OSCOMPAT_USAGE_VAR"));
	TEST_ASSERT(os_getenv("OSCOMPAT_USAGE_VAR") == NULL);
}

static void testProcessUsage(void) {
	char out[64];
	int code = -1;

#if OSCOMPAT_WINDOWS
	const char* argv[] = { "cmd", "/c", "<nul set /p =oscompat", NULL };
	const char* failArgv[] = { "cmd", "/c", "exit 3", NULL };
#else
	const char* argv[] = { "sh", "-c", "printf oscompat", NULL };
	const char* failArgv[] = { "sh", "-c", "exit 3", NULL };
#endif

	TEST_INT_EQ(0, os_run_capture(argv, out, sizeof(out), &code));
	TEST_STR_EQ("oscompat", out);
	TEST_INT_EQ(0, code);

	TEST_INT_EQ(0, os_run(failArgv, &code));
	TEST_INT_EQ(3, code);

	TEST_ASSERT(os_process_id() > 0);
}

static void testTimeUsage(void) {
	uint64_t start = os_time_monotonic_ms();
	uint64_t end;
	uint64_t realtime = os_time_realtime_ms();

	os_sleep_ms(10);
	end = os_time_monotonic_ms();
	TEST_ASSERT(end >= start);
	TEST_ASSERT(end - start >= 5);
	TEST_ASSERT(realtime > 1600000000000ULL);
}

static void testTerminalUsage(void) {
	int cols = 0;
	int rows = 0;
	int tty = os_isatty(1);

	TEST_ASSERT(tty == 0 || tty == 1);
	TEST_INT_EQ(0, os_isatty(-1));
	TEST_ASSERT(os_tty_size(1, &cols, &rows) == 0 || os_tty_size(1, &cols, &rows) == -1);
	TEST_ASSERT(os_color_enabled(1) == 0 || os_color_enabled(1) == 1);
	TEST_INT_EQ(0, os_color_enabled(-1));
}

static void testOptionsUsage(void) {
	char* argv[] = { "app", "-v", "--output", "out.txt", "--level=5", "in1", "in2", NULL };
	static const struct OsOption longOpts[] = {
		{ "verbose", OS_NO_ARGUMENT, NULL, 'v' },
		{ "output", OS_REQUIRED_ARGUMENT, NULL, 'o' },
		{ "level", OS_REQUIRED_ARGUMENT, NULL, 'l' },
		{ NULL, 0, NULL, 0 }
	};
	int index = -1;

	os_getopt_reset();
	TEST_INT_EQ('v', os_getopt_long(7, argv, "vo:l:", longOpts, &index));
	TEST_INT_EQ('o', os_getopt_long(7, argv, "vo:l:", longOpts, &index));
	TEST_STR_EQ("out.txt", osOptArg);
	TEST_INT_EQ('l', os_getopt_long(7, argv, "vo:l:", longOpts, &index));
	TEST_STR_EQ("5", osOptArg);
	TEST_INT_EQ(-1, os_getopt_long(7, argv, "vo:l:", longOpts, &index));
	TEST_STR_EQ("in1", argv[osOptInd]);
}

int main(void) {
	testFileRoundTrip();
	testPathUsage();
	testEnvUsage();
	testProcessUsage();
	testTimeUsage();
	testTerminalUsage();
	testOptionsUsage();
	return os_test_summary("usage_app");
}
