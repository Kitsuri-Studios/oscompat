#if !defined(_WIN32)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#endif

#include "test_util.h"

#include <oscompat/common.h>
#include <oscompat/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int osJoinPath(char* out, size_t cap, const char* dir, const char* name) {
	size_t dirLen = strlen(dir);
	size_t nameLen = strlen(name);
	if (dirLen + 1 + nameLen + 1 > cap) {
		return -1;
	}
	memcpy(out, dir, dirLen);
#if OSCOMPAT_WINDOWS
	out[dirLen] = '\\';
#else
	out[dirLen] = '/';
#endif
	memcpy(out + dirLen + 1, name, nameLen + 1);
	return 0;
}

static int osMakeScratchDir(char* buf, size_t cap) {
	char file[OSCOMPAT_PATH_MAX];
	size_t len;
	if (os_temp_file(file, sizeof(file)) != 0) {
		return -1;
	}
	if (remove(file) != 0) {
		return -1;
	}
	if (os_mkdir_p(file) != 0) {
		return -1;
	}
	len = strlen(file);
	if (len + 1 > cap) {
		os_remove_recursive(file);
		return -1;
	}
	memcpy(buf, file, len + 1);
	return 0;
}

int main(void) {
	char dir[OSCOMPAT_PATH_MAX];
	char filePath[OSCOMPAT_PATH_MAX];
	char missing[OSCOMPAT_PATH_MAX];
	char nested[OSCOMPAT_PATH_MAX];
	char leaf[OSCOMPAT_PATH_MAX];
	char tempPath[OSCOMPAT_PATH_MAX];
	const char binary[] = { 'a', '\0', 'b', '\n', (char)0xff, '\0', 'z' };
	size_t binarySize = sizeof(binary);
	char* data = NULL;
	size_t size = 0;

	TEST_INT_EQ(0, osMakeScratchDir(dir, sizeof(dir)));
	TEST_INT_EQ(1, os_file_exists(dir));
	TEST_INT_EQ(0, os_is_file(dir));
	TEST_INT_EQ(1, os_is_dir(dir));

	TEST_INT_EQ(0, osJoinPath(filePath, sizeof(filePath), dir, "data.bin"));
	TEST_INT_EQ(0, os_write_file(filePath, binary, binarySize));
	TEST_INT_EQ(1, os_file_exists(filePath));
	TEST_INT_EQ(1, os_is_file(filePath));
	TEST_INT_EQ(0, os_is_dir(filePath));

	TEST_INT_EQ(0, os_read_file(filePath, &data, &size));
	TEST_ASSERT(data != NULL);
	TEST_INT_EQ((long long)binarySize, (long long)size);
	TEST_MEM_EQ(binary, data, binarySize);
	TEST_INT_EQ(0, data[size]);
	free(data);
	data = NULL;
	size = 0;

	TEST_INT_EQ(0, osJoinPath(missing, sizeof(missing), dir, "nope.bin"));
	TEST_INT_EQ(0, os_file_exists(missing));
	TEST_INT_EQ(0, os_is_file(missing));
	TEST_INT_EQ(0, os_is_dir(missing));
	TEST_INT_EQ(-1, os_read_file(missing, &data, &size));
	TEST_ASSERT(data == NULL);

	TEST_INT_EQ(0, osJoinPath(nested, sizeof(nested), dir, "a/b/c"));
	TEST_INT_EQ(0, os_mkdir_p(nested));
	TEST_INT_EQ(0, os_mkdir_p(nested));
	TEST_INT_EQ(1, os_is_dir(nested));
	TEST_INT_EQ(0, osJoinPath(leaf, sizeof(leaf), nested, "leaf.txt"));
	TEST_INT_EQ(0, os_write_file(leaf, "hello", 5));
	TEST_INT_EQ(1, os_is_file(leaf));

	TEST_INT_EQ(0, os_temp_file(tempPath, sizeof(tempPath)));
	TEST_ASSERT(tempPath[0] != '\0');
	TEST_INT_EQ(1, os_file_exists(tempPath));
	TEST_INT_EQ(1, os_is_file(tempPath));
	TEST_INT_EQ(0, os_remove_recursive(tempPath));
	TEST_INT_EQ(0, os_file_exists(tempPath));

	TEST_INT_EQ(0, os_remove_recursive(dir));
	TEST_INT_EQ(0, os_file_exists(dir));

	return os_test_summary("file");
}
