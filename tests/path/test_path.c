#include "test_util.h"

#include <oscompat/common.h>
#include <oscompat/path.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OSCOMPAT_WINDOWS
#define TEST_SEP "\\"
#else
#define TEST_SEP "/"
#endif

static int osEndsWithTestPath(const char* name) {
	size_t len = strlen(name);
	if (len > 4 && name[len - 4] == '.' &&
		(name[len - 3] == 'e' || name[len - 3] == 'E') &&
		(name[len - 2] == 'x' || name[len - 2] == 'X') &&
		(name[len - 1] == 'e' || name[len - 1] == 'E')) {
		len -= 4;
	}
	return len >= 6 && memcmp(name + len - 6, "t_path", 6) == 0;
}

static void testRealpath(void) {
	char fixed[4096];
	char small[2];
	char* dyn = os_realpath(".", NULL, 0);
	TEST_ASSERT(dyn != NULL);
	if (dyn != NULL) {
		TEST_ASSERT(os_path_is_absolute(dyn));
		TEST_ASSERT(os_realpath(".", fixed, sizeof(fixed)) != NULL);
		TEST_STR_EQ(dyn, fixed);
		free(dyn);
	}
	TEST_ASSERT(os_realpath(".", small, sizeof(small)) == NULL);
	TEST_INT_EQ(ENAMETOOLONG, errno);
	TEST_ASSERT(os_realpath("/nonexistent-oscompat-xyz", NULL, 0) == NULL);
}

static void testRealpathTemp(void) {
	char tmp[4096];
	char* res;
	TEST_INT_EQ(0, os_temp_dir(tmp, sizeof(tmp)));
	res = os_realpath(tmp, NULL, 0);
	TEST_ASSERT(res != NULL);
	if (res != NULL) {
		TEST_ASSERT(os_path_is_absolute(res));
		free(res);
	}
}

static void testExe(void) {
	char exe[4096];
	char dir[4096];
	size_t elen;
	size_t dlen;
	TEST_INT_EQ(0, os_exe_path(exe, sizeof(exe)));
	TEST_INT_EQ(0, os_exe_dir(dir, sizeof(dir)));
	elen = strlen(exe);
	dlen = strlen(dir);
	TEST_ASSERT(osEndsWithTestPath(exe));
	TEST_ASSERT(dlen < elen);
	TEST_MEM_EQ(exe, dir, dlen);
	TEST_ASSERT(exe[dlen] == '/' || exe[dlen] == '\\');
}

static void testIsAbsolute(void) {
	TEST_INT_EQ(1, os_path_is_absolute("/"));
	TEST_INT_EQ(1, os_path_is_absolute("/usr/lib"));
	TEST_INT_EQ(0, os_path_is_absolute("usr/lib"));
	TEST_INT_EQ(0, os_path_is_absolute(""));
	TEST_INT_EQ(0, os_path_is_absolute("."));
	TEST_INT_EQ(0, os_path_is_absolute("./a"));
}

static void testJoin(void) {
	char buf[4096];
	TEST_INT_EQ(0, os_path_join(buf, sizeof(buf), TEST_SEP "a", "b"));
	TEST_STR_EQ(TEST_SEP "a" TEST_SEP "b", buf);
	TEST_INT_EQ(0, os_path_join(buf, sizeof(buf), TEST_SEP "a" TEST_SEP, "b"));
	TEST_STR_EQ(TEST_SEP "a" TEST_SEP "b", buf);
	TEST_INT_EQ(0, os_path_join(buf, sizeof(buf), "a", "b"));
	TEST_STR_EQ("a" TEST_SEP "b", buf);
	TEST_INT_EQ(0, os_path_join(buf, sizeof(buf), TEST_SEP "a", TEST_SEP "b"));
	TEST_STR_EQ(TEST_SEP "b", buf);
	TEST_INT_EQ(0, os_path_join(buf, sizeof(buf), "", "b"));
	TEST_STR_EQ("b", buf);
	TEST_INT_EQ(0, os_path_join(buf, sizeof(buf), TEST_SEP, "b"));
	TEST_STR_EQ(TEST_SEP "b", buf);
	TEST_INT_EQ(-1, os_path_join(buf, 1, TEST_SEP "a", "b"));
}

static void testNormalize(void) {
	char buf[4096];
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "a/./b/../c"));
	TEST_STR_EQ("a" TEST_SEP "c", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "/a/b/../c"));
	TEST_STR_EQ(TEST_SEP "a" TEST_SEP "c", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "a//b"));
	TEST_STR_EQ("a" TEST_SEP "b", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "./a"));
	TEST_STR_EQ("a", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "/.."));
	TEST_STR_EQ(TEST_SEP, buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "a/.."));
	TEST_STR_EQ(".", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), ""));
	TEST_STR_EQ(".", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "/a/../../b"));
	TEST_STR_EQ(TEST_SEP "b", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "../a"));
	TEST_STR_EQ(".." TEST_SEP "a", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "/"));
	TEST_STR_EQ(TEST_SEP, buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "a/../../b"));
	TEST_STR_EQ(".." TEST_SEP "b", buf);
#if !OSCOMPAT_WINDOWS
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "//"));
	TEST_STR_EQ("//", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "///"));
	TEST_STR_EQ("/", buf);
	TEST_INT_EQ(0, os_path_normalize(buf, sizeof(buf), "//a/b"));
	TEST_STR_EQ("//a/b", buf);
#endif
}

static void testDirname(void) {
	char buf[4096];
	TEST_INT_EQ(0, os_path_dirname("/usr/lib", buf, sizeof(buf)));
	TEST_STR_EQ("/usr", buf);
	TEST_INT_EQ(0, os_path_dirname("/usr/", buf, sizeof(buf)));
	TEST_STR_EQ("/", buf);
	TEST_INT_EQ(0, os_path_dirname("usr", buf, sizeof(buf)));
	TEST_STR_EQ(".", buf);
	TEST_INT_EQ(0, os_path_dirname("/", buf, sizeof(buf)));
	TEST_STR_EQ("/", buf);
	TEST_INT_EQ(0, os_path_dirname("///", buf, sizeof(buf)));
	TEST_STR_EQ("/", buf);
	TEST_INT_EQ(0, os_path_dirname("a/b", buf, sizeof(buf)));
	TEST_STR_EQ("a", buf);
	TEST_INT_EQ(0, os_path_dirname("a/b/", buf, sizeof(buf)));
	TEST_STR_EQ("a", buf);
	TEST_INT_EQ(0, os_path_dirname("/a", buf, sizeof(buf)));
	TEST_STR_EQ("/", buf);
	TEST_INT_EQ(0, os_path_dirname("", buf, sizeof(buf)));
	TEST_STR_EQ(".", buf);
}

static void testBasename(void) {
	char buf[4096];
	TEST_INT_EQ(0, os_path_basename("/usr/lib", buf, sizeof(buf)));
	TEST_STR_EQ("lib", buf);
	TEST_INT_EQ(0, os_path_basename("/usr/", buf, sizeof(buf)));
	TEST_STR_EQ("usr", buf);
	TEST_INT_EQ(0, os_path_basename("usr", buf, sizeof(buf)));
	TEST_STR_EQ("usr", buf);
	TEST_INT_EQ(0, os_path_basename("/", buf, sizeof(buf)));
	TEST_STR_EQ("/", buf);
	TEST_INT_EQ(0, os_path_basename("///", buf, sizeof(buf)));
	TEST_STR_EQ("/", buf);
	TEST_INT_EQ(0, os_path_basename("a/b/", buf, sizeof(buf)));
	TEST_STR_EQ("b", buf);
	TEST_INT_EQ(0, os_path_basename("", buf, sizeof(buf)));
	TEST_STR_EQ(".", buf);
	TEST_INT_EQ(0, os_path_basename("/a/", buf, sizeof(buf)));
	TEST_STR_EQ("a", buf);
}

static void testDirs(void) {
	char buf[4096];
	TEST_INT_EQ(0, os_home_dir(buf, sizeof(buf)));
	TEST_ASSERT(buf[0] != '\0');
	TEST_ASSERT(os_path_is_absolute(buf));
	TEST_INT_EQ(0, os_temp_dir(buf, sizeof(buf)));
	TEST_ASSERT(buf[0] != '\0');
	TEST_ASSERT(os_path_is_absolute(buf));
}

int main(void) {
	testRealpath();
	testRealpathTemp();
	testExe();
	testIsAbsolute();
	testJoin();
	testNormalize();
	testDirname();
	testBasename();
	testDirs();
	return os_test_summary("path");
}
