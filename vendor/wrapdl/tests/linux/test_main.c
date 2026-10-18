#include "test_util.h"
#include "wrapdl.h"

#include <link.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef TEST_LIB_DIR
#define TEST_LIB_DIR "."
#endif

static int iter_count;

static void libpath(char *out, size_t n, const char *name) {
    snprintf(out, n, "%s/%s", TEST_LIB_DIR, name);
}

static int iter_cb(struct dl_phdr_info *info, size_t size, void *data) {
    (void)size;
    (void)data;
    if (info && info->dlpi_name && info->dlpi_name[0]) iter_count++;
    return 0;
}

static void test_dummy_open_close(test_log *t) {
    char path[512];
    libpath(path, sizeof(path), "libdummyA.so");

    if (access(path, R_OK) != 0)
        test_note(t, "  skip: %s missing", path);

    wdl_h h = wdlopen(path, WDL_DEFAULT);
    if (!h) {
        test_fail(t, "wdlopen dummyA", "%s", wdlerror() ? wdlerror() : "?");
        return;
    }
    test_ok(t, "wdlopen dummyA");
    wdlclose(h);
    test_ok(t, "wdlclose dummyA");
}

static void test_dummy_sym(test_log *t) {
    char path[512];
    libpath(path, sizeof(path), "libdummyA.so");
    wdl_h h = wdlopen(path, WDL_DEFAULT);
    if (!h) return;

    typedef int (*fn)(void);
    fn f = (fn)wdlsym(h, "dummyA_value", NULL, WDL_DEFAULT);
    if (!f || f() != 42)
        test_fail(t, "wdlsym dummyA", "bad ptr or value");
    else
        test_ok(t, "wdlsym dummyA_value");
    wdlclose(h);
}

static void test_dummy_addr(test_log *t) {
    char path[512];
    libpath(path, sizeof(path), "libdummyB.so");
    wdl_h h = wdlopen(path, WDL_DEFAULT);
    if (!h) return;

    wdl_sym s = wdlsym(h, "dummyB_value", NULL, WDL_DEFAULT);
    wdl_info info;
    void *cache = NULL;
    if (!s || !wdladdr((void *)s, &info, &cache, WDL_DEFAULT))
        test_fail(t, "wdladdr dummyB", "no result");
    else
        test_ok(t, "wdladdr dummyB");
    wdlclear(&cache);
    test_ok(t, "wdlclear");
    wdlclose(h);
}

static void test_cross_dep(test_log *t) {
    char cpath[512], bpath[512];
    libpath(cpath, sizeof(cpath), "libdummyC.so");
    libpath(bpath, sizeof(bpath), "libdummyB.so");

    wdl_h h = wdlopen(cpath, WDL_DEFAULT);
    if (!h) {
        test_fail(t, "wdlopen dummyC", "%s", wdlerror() ? wdlerror() : "?");
        return;
    }

    typedef int (*call_b_fn)(const char *);
    call_b_fn cb = (call_b_fn)wdlsym(h, "dummyC_call_b", NULL, WDL_DEFAULT);
    if (!cb) {
        test_fail(t, "dummyC_call_b", "missing");
        wdlclose(h);
        return;
    }

    int v = cb(bpath);
    if (v != 84)
        test_fail(t, "cross-dep dummyC->B", "got %d", v);
    else
        test_ok(t, "dummyC loads dummyB");
    wdlclose(h);
}

static void test_iter(test_log *t) {
    iter_count = 0;
    if (wdliter(iter_cb, NULL, WDL_DEFAULT) != 0)
        test_fail(t, "wdliter", "callback failed");
    else
        test_ok(t, "wdliter");
    test_note(t, "  modules seen: %d", iter_count);
}

int main(void) {
    test_log log;
    test_init(&log);
    test_note(&log, "=== wrapdl linux ===");
    test_note(&log, "libdir=%s", TEST_LIB_DIR);

    test_dummy_open_close(&log);
    test_dummy_sym(&log);
    test_dummy_addr(&log);
    test_cross_dep(&log);
    test_iter(&log);

    test_note(&log, "pass=%d fail=%d", log.pass, log.fail);
    if (log.buf) fputs(log.buf, stdout);

    int rc = log.fail ? 1 : 0;
    test_free(&log);
    return rc;
}
