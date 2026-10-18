#include "test_util.h"

#include <oscompat/process.h>

#include <string.h>

int main(void) {
	char out[64];
	char small[2];
	int exitCode = -12345;

#if defined(_WIN32)
	const char* captureArgv[] = { "cmd", "/c", "<nul set /p =abc & exit 7", NULL };
	const char* exitArgv[] = { "cmd", "/c", "exit 7", NULL };
	const char* emptyArgv[] = { "cmd", "/c", "exit 0", NULL };
	const char* missingArgv[] = { "oscompat_no_such_program_zzz", NULL };
#else
	const char* captureArgv[] = { "sh", "-c", "printf abc; exit 7", NULL };
	const char* exitArgv[] = { "sh", "-c", "exit 7", NULL };
	const char* emptyArgv[] = { "sh", "-c", "exit 0", NULL };
	const char* missingArgv[] = { "oscompat_no_such_program_zzz", NULL };
#endif

	TEST_INT_EQ(0, os_run_capture(captureArgv, out, sizeof out, &exitCode));
	TEST_STR_EQ("abc", out);
	TEST_INT_EQ(7, exitCode);

	TEST_INT_EQ(0, os_run(exitArgv, &exitCode));
	TEST_INT_EQ(7, exitCode);

	exitCode = -1;
	TEST_INT_EQ(0, os_run_capture(emptyArgv, out, sizeof out, &exitCode));
	TEST_STR_EQ("", out);
	TEST_INT_EQ(0, exitCode);

	small[0] = (char)0x7f;
	small[1] = (char)0x7f;
	exitCode = -1;
	TEST_INT_EQ(0, os_run_capture(captureArgv, small, sizeof small, &exitCode));
	TEST_STR_EQ("a", small);
	TEST_INT_EQ(7, exitCode);

	exitCode = -1;
	TEST_INT_EQ(-1, os_run_capture(missingArgv, out, sizeof out, &exitCode));

	exitCode = -1;
	TEST_INT_EQ(-1, os_run(missingArgv, &exitCode));

	TEST_ASSERT(os_process_id() > 0);

	return os_test_summary("process");
}
