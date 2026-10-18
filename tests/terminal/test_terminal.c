#if !defined(_WIN32)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif
#endif

#include "test_util.h"

#include <oscompat/env.h>
#include <oscompat/terminal.h>

int main(void) {
	int ttyStd = os_isatty(1);
	int ttyInvalid = os_isatty(-1);
	int cols = 0;
	int rows = 0;
	int sizeStd = os_tty_size(1, &cols, &rows);
	int sizeInvalid = os_tty_size(-1, &cols, &rows);
	int color = os_color_enabled(1);

	TEST_ASSERT(ttyStd == 0 || ttyStd == 1);
	TEST_INT_EQ(0, ttyInvalid);

	TEST_ASSERT(sizeStd == 0 || sizeStd == -1);
	if (sizeStd == 0) {
		TEST_ASSERT(cols >= 0);
		TEST_ASSERT(rows >= 0);
	}
	TEST_ASSERT(sizeInvalid == 0 || sizeInvalid == -1);

	TEST_ASSERT(color == 0 || color == 1);

	TEST_INT_EQ(0, os_color_enabled(-1));

	os_setenv("TERM", "dumb", 1);
	TEST_INT_EQ(0, os_color_enabled(1));

	os_setenv("NO_COLOR", "1", 1);
	TEST_INT_EQ(0, os_color_enabled(1));

	os_setenv("NO_COLOR", "", 1);
	os_unsetenv("TERM");
	TEST_INT_EQ(ttyStd, os_color_enabled(1));

	os_unsetenv("NO_COLOR");

	return os_test_summary("terminal");
}
