#include <oscompat/common.h>
#include <oscompat/dynlib.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "wrapdl.h"

struct OsDynlib {
	wdl_h handle;
};

static wdl_h osDynlibSelf(void) {
#if OSCOMPAT_WINDOWS
	return (wdl_h)GetModuleHandleA(NULL);
#else
	return (wdl_h)dlopen(NULL, RTLD_NOW);
#endif
}

OsDynlib* os_dynlib_open(const char* path) {
	OsDynlib* lib;
	wdl_h handle = (path == NULL) ? osDynlibSelf() : wdlopen(path, WDL_DEFAULT);

	if (handle == NULL) {
		return NULL;
	}
	lib = (OsDynlib*)malloc(sizeof(OsDynlib));
	if (lib == NULL) {
		wdlclose(handle);
		errno = ENOMEM;
		return NULL;
	}
	lib->handle = handle;
	return lib;
}

void* os_dynlib_sym(OsDynlib* lib, const char* name) {
	size_t size = 0;
	if (lib == NULL || name == NULL) {
		errno = EINVAL;
		return NULL;
	}
	return (void*)(uintptr_t)wdlsym(lib->handle, name, &size, WDL_DEFAULT);
}

int os_dynlib_close(OsDynlib* lib) {
	int failed;
	if (lib == NULL) {
		errno = EINVAL;
		return -1;
	}
	failed = wdlclose(lib->handle) != 0 ? 1 : 0;
	free(lib);
	if (failed) {
		return -1;
	}
	return 0;
}

const char* os_dynlib_error(void) {
	const char* message = wdlerror();
	if (message == NULL) {
		return "no error";
	}
	return message;
}
