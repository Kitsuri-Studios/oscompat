#include <oscompat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fail(const char* what) {
	fprintf(stderr, "consumer: %s failed\n", what);
	return 1;
}

int main(void) {
	char work[OSCOMPAT_PATH_MAX];
	char note[OSCOMPAT_PATH_MAX];
	char exe[OSCOMPAT_PATH_MAX];
	char* data = NULL;
	size_t size = 0;
	OsDynlib* lib;

	if (os_temp_file(work, sizeof(work)) != 0) {
		return fail("temp_file");
	}
	if (os_remove_recursive(work) != 0) {
		return fail("remove temp file");
	}
	if (os_mkdir_p(work) != 0) {
		return fail("mkdir_p");
	}
	if (!os_is_dir(work)) {
		return fail("is_dir");
	}

	if (os_path_join(note, sizeof(note), work, "note.txt") != 0) {
		return fail("path_join");
	}
	if (os_write_file(note, "hello oscompat", 14) != 0) {
		return fail("write_file");
	}
	if (os_read_file(note, &data, &size) != 0) {
		return fail("read_file");
	}
	if (size != 14 || memcmp(data, "hello oscompat", 14) != 0) {
		free(data);
		return fail("file contents");
	}
	free(data);

	if (os_exe_path(exe, sizeof(exe)) != 0) {
		return fail("exe_path");
	}
	if (!os_path_is_absolute(exe)) {
		return fail("exe_path absolute");
	}

	lib = os_dynlib_open(NULL);
	if (lib == NULL) {
		return fail("dynlib_open self");
	}
	if (os_dynlib_close(lib) != 0) {
		return fail("dynlib_close");
	}

	if (os_remove_recursive(work) != 0) {
		return fail("cleanup");
	}
	if (os_file_exists(work)) {
		return fail("cleanup check");
	}

	printf("consumer ok\n");
	return 0;
}
