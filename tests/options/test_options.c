#include "test_util.h"

#include <oscompat/options.h>

static int osTestFlag = 0;

static const struct OsOption osTestLongOptions[] = {
	{ "alpha", OS_NO_ARGUMENT, NULL, 'a' },
	{ "bravo", OS_REQUIRED_ARGUMENT, NULL, 'b' },
	{ "charlie", OS_NO_ARGUMENT, NULL, 'c' },
	{ "delta", OS_OPTIONAL_ARGUMENT, NULL, 'd' },
	{ "flag", OS_NO_ARGUMENT, &osTestFlag, 1 },
	{ NULL, 0, NULL, 0 }
};

static const struct OsOption osTestAmbiguous[] = {
	{ "alpha", OS_NO_ARGUMENT, NULL, 'a' },
	{ "alpine", OS_NO_ARGUMENT, NULL, 'p' },
	{ NULL, 0, NULL, 0 }
};

int main(void) {
	char* flags[] = { "prog", "-a", "-b", "operand", NULL };
	char* clustered[] = { "prog", "-abc", "-d", "value", "-eVAL", NULL };
	char* unknown[] = { "prog", "-x", NULL };
	char* stop[] = { "prog", "-a", "--", "-b", NULL };
	char* grouped[] = { "prog", "-ab", NULL };
	char* missing[] = { "prog", "-d", NULL };
	char* longArgs[] = { "prog", "--alpha", "--bravo=hello", "--flag", NULL };
	char* longSeparate[] = { "prog", "-a", "--bravo", "world", NULL };
	char* longOptional[] = { "prog", "--delta", "--delta=set", NULL };
	char* ambiguous[] = { "prog", "--alp", NULL };
	char* longUnknown[] = { "prog", "--nope", NULL };
	char* repeat[] = { "prog", "-a", "-b", NULL };
	int index = -1;

	os_getopt_reset();
	TEST_INT_EQ('a', os_getopt(4, flags, "ab"));
	TEST_INT_EQ('b', os_getopt(4, flags, "ab"));
	TEST_INT_EQ(-1, os_getopt(4, flags, "ab"));
	TEST_INT_EQ(3, osOptInd);

	os_getopt_reset();
	TEST_INT_EQ('a', os_getopt(5, clustered, "abcd:e:"));
	TEST_INT_EQ('b', os_getopt(5, clustered, "abcd:e:"));
	TEST_INT_EQ('c', os_getopt(5, clustered, "abcd:e:"));
	TEST_INT_EQ('d', os_getopt(5, clustered, "abcd:e:"));
	TEST_STR_EQ("value", osOptArg);
	TEST_INT_EQ('e', os_getopt(5, clustered, "abcd:e:"));
	TEST_STR_EQ("VAL", osOptArg);
	TEST_INT_EQ(-1, os_getopt(5, clustered, "abcd:e:"));

	os_getopt_reset();
	TEST_INT_EQ('a', os_getopt(2, grouped, "ab"));
	TEST_INT_EQ('b', os_getopt(2, grouped, "ab"));
	TEST_INT_EQ(-1, os_getopt(2, grouped, "ab"));

	os_getopt_reset();
	osOptErr = 0;
	TEST_INT_EQ('?', os_getopt(2, unknown, "ab"));
	TEST_INT_EQ('x', osOptOpt);
	osOptErr = 1;

	os_getopt_reset();
	TEST_INT_EQ('a', os_getopt(4, stop, "ab"));
	TEST_INT_EQ(-1, os_getopt(4, stop, "ab"));
	TEST_INT_EQ(3, osOptInd);

	os_getopt_reset();
	osOptErr = 0;
	TEST_INT_EQ('?', os_getopt(2, missing, "d:"));
	osOptErr = 1;

	os_getopt_reset();
	TEST_INT_EQ(':', os_getopt(2, missing, ":d:"));

	os_getopt_reset();
	TEST_INT_EQ('a', os_getopt_long(4, longArgs, "ab", osTestLongOptions, &index));
	TEST_INT_EQ(0, index);
	TEST_INT_EQ('b', os_getopt_long(4, longArgs, "ab", osTestLongOptions, &index));
	TEST_STR_EQ("hello", osOptArg);
	TEST_INT_EQ(1, index);
	osTestFlag = 0;
	TEST_INT_EQ(0, os_getopt_long(4, longArgs, "ab", osTestLongOptions, &index));
	TEST_INT_EQ(1, osTestFlag);
	TEST_INT_EQ(4, index);
	TEST_INT_EQ(-1, os_getopt_long(4, longArgs, "ab", osTestLongOptions, &index));

	os_getopt_reset();
	TEST_INT_EQ('a', os_getopt_long(4, longSeparate, "ab", osTestLongOptions, &index));
	TEST_INT_EQ('b', os_getopt_long(4, longSeparate, "ab", osTestLongOptions, &index));
	TEST_STR_EQ("world", osOptArg);
	TEST_INT_EQ(-1, os_getopt_long(4, longSeparate, "ab", osTestLongOptions, &index));

	os_getopt_reset();
	TEST_INT_EQ('d', os_getopt_long(3, longOptional, "d", osTestLongOptions, &index));
	TEST_ASSERT(osOptArg == NULL);
	TEST_INT_EQ('d', os_getopt_long(3, longOptional, "d", osTestLongOptions, &index));
	TEST_STR_EQ("set", osOptArg);

	os_getopt_reset();
	osOptErr = 0;
	TEST_INT_EQ('?', os_getopt_long(2, ambiguous, "a", osTestAmbiguous, &index));
	os_getopt_reset();
	TEST_INT_EQ('?', os_getopt_long(2, longUnknown, "a", osTestLongOptions, &index));
	osOptErr = 1;

	os_getopt_reset();
	TEST_INT_EQ('a', os_getopt(3, repeat, "ab"));
	TEST_INT_EQ('b', os_getopt(3, repeat, "ab"));
	TEST_INT_EQ(-1, os_getopt(3, repeat, "ab"));
	os_getopt_reset();
	TEST_INT_EQ(1, osOptInd);
	TEST_INT_EQ('a', os_getopt(3, repeat, "ab"));
	TEST_INT_EQ('b', os_getopt(3, repeat, "ab"));

	return os_test_summary("options");
}
