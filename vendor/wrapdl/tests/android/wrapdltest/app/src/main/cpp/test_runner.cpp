#include "test_runner.h"

#include "test_util.h"
#include "wrapdl.h"

#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void libpath(char *out, size_t n, const char *dir, const char *name) {
    snprintf(out, n, "%s/%s", dir, name);
}

static void test_lib_on_disk(test_log *t, const char *dir, const char *name) {
    char path[512];
    libpath(path, sizeof(path), dir, name);
    if (access(path, R_OK) == 0) test_note(t, "  on disk: %s", name);
    else test_note(t, "  missing on disk: %s (need extractNativeLibs)", name);
}

static void test_open_close(test_log *t, const char *dir) {
    char path[512];
    libpath(path, sizeof(path), dir, "libdummyA.so");

    wdl_h h = wdlopen(path, WDL_DEFAULT);
    if (!h) {
        test_fail(t, "wdlopen dummyA", "%s", wdlerror() ? wdlerror() : "?");
        return;
    }
    test_ok(t, "wdlopen dummyA");

    wdl_h h2 = wdlopen(path, WDL_LOAD);
    if (!h2) test_fail(t, "wdlopen WDL_LOAD", "%s", wdlerror() ? wdlerror() : "?");
    else test_ok(t, "wdlopen WDL_LOAD");

    wdlclose(h);
    test_ok(t, "wdlclose dummyA");
}

static void test_sym(test_log *t, const char *dir) {
    char path[512];
    libpath(path, sizeof(path), dir, "libdummyA.so");
    wdl_h h = wdlopen(path, WDL_DEFAULT);
    if (!h) return;

    typedef int (*fn)(void);
    fn f = (fn)wdlsym(h, "dummyA_value", NULL, WDL_DEFAULT);
    if (!f || f() != 42) test_fail(t, "wdlsym dummyA", "bad ptr or value");
    else test_ok(t, "wdlsym dummyA_value");

    size_t sz = 0;
    fn f2 = (fn)wdlsym(h, "dummyA_value", &sz, WDL_DEBUG);
    if (!f2) test_note(t, "  WDL_DEBUG sym optional miss");
    else test_ok(t, "wdlsym WDL_DEBUG dummyA");

    wdlclose(h);
}

static void test_addr_iter(test_log *t, const char *dir) {
    char path[512];
    libpath(path, sizeof(path), dir, "libdummyB.so");
    wdl_h h = wdlopen(path, WDL_DEFAULT);
    if (!h) return;

    wdl_sym s = wdlsym(h, "dummyB_value", NULL, WDL_DEFAULT);
    wdl_info info;
    void *cache = NULL;
    if (s && wdladdr((void *)s, &info, &cache, WDL_DEFAULT))
        test_ok(t, "wdladdr");
    else
        test_fail(t, "wdladdr", "failed");

    wdlclear(&cache);
    test_ok(t, "wdlclear");

    int n = 0;
    auto cb = +[](struct dl_phdr_info *i, size_t, void *d) -> int {
        if (i && i->dlpi_name && i->dlpi_name[0]) (*(int *)d)++;
        return 0;
    };
    if (wdliter(cb, &n, WDL_FULLPATH) == 0)
        test_ok(t, "wdliter WDL_FULLPATH");
    else
        test_fail(t, "wdliter", "?");
    test_note(t, "  phdr count=%d", n);

    wdlclose(h);
}

static void test_ns_bypass(test_log *t, const char *dir) {
    if (!wdlns_ok()) {
        test_fail(t, "wdlns_ok", "bypass not loaded");
        return;
    }
    test_ok(t, "wdlns_ok");

    struct android_namespace_t *ns = wdlns_escape("wrapdl_test", dir, dir, WDL_NS_SHARED, dir, NULL);
    if (!ns) {
        test_fail(t, "wdlns_escape", "null");
        return;
    }
    test_ok(t, "wdlns_escape");

    char path[512];
    libpath(path, sizeof(path), dir, "libdummyB.so");
    void *h = wdlns_open(path, RTLD_NOW, ns);
    if (!h) test_fail(t, "wdlns_open dummyB", "%s", dlerror() ? dlerror() : "?");
    else test_ok(t, "wdlns_open dummyB");
}

static void test_ns_merge(test_log *t, const char *dir) {
    struct android_namespace_t *ns_a = wdlns_new("wrapdl_a", dir, dir, WDL_NS_ISOLATED, dir, NULL);
    struct android_namespace_t *ns_b = wdlns_new("wrapdl_b", dir, dir, WDL_NS_ISOLATED, dir, NULL);
    if (!ns_a || !ns_b) {
        test_fail(t, "wdlns_new", "create failed");
        return;
    }
    test_ok(t, "wdlns_new x2");

    wdlns_link_default(ns_a);
    wdlns_link_default(ns_b);

    if (!wdlns_linkall(ns_a, ns_b))
        test_fail(t, "wdlns_linkall", "link failed");
    else
        test_ok(t, "wdlns_linkall");

    char bpath[512], cpath[512];
    libpath(bpath, sizeof(bpath), dir, "libdummyB.so");
    libpath(cpath, sizeof(cpath), dir, "libdummyC.so");

    void *hb = wdlns_open(bpath, RTLD_NOW, ns_b);
    if (!hb) test_fail(t, "wdlns_open B in ns_b", "%s", dlerror() ? dlerror() : "?");
    else test_ok(t, "wdlns_open dummyB in ns_b");

    void *hc = wdlns_open(cpath, RTLD_NOW, ns_a);
    if (!hc) test_fail(t, "wdlns_open C in ns_a", "%s", dlerror() ? dlerror() : "?");
    else test_ok(t, "wdlns_open dummyC in linked ns_a");
}

static void test_cross_ns_dep(test_log *t, const char *dir) {
    char cpath[512], bpath[512];
    libpath(cpath, sizeof(cpath), dir, "libdummyC.so");
    libpath(bpath, sizeof(bpath), dir, "libdummyB.so");

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
    if (v != 84) test_fail(t, "cross-ns dep", "got %d", v);
    else test_ok(t, "dummyC loads dummyB via wdlopen");
    wdlclose(h);
}

static void test_runtime_patch(test_log *t, const char *self_path, const char *dir) {
    if (!self_path || !self_path[0]) {
        test_note(t, "  wdlns_init skipped (no self path)");
        return;
    }
    if (wdlns_init(self_path))
        test_ok(t, "wdlns_init");
    else
        test_fail(t, "wdlns_init", "failed");

    const char *paths[] = {dir};
    if (wdlns_paths(paths, 1))
        test_ok(t, "wdlns_paths");
    else
        test_note(t, "  wdlns_paths skipped");

    if (wdlns_main()) test_ok(t, "wdlns_main");
}

static void test_system_sym(test_log *t) {
    static const char *sym =
        "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEblPjPPNS_10InputEventE";

    wdl_h h = wdlopen("libinput.so", WDL_FORCE);
    if (!h) h = wdlopen("libinput.so", WDL_DEFAULT);
    if (!h) {
        test_note(t, "  libinput.so not mapped (ok on some devices)");
        return;
    }
    test_ok(t, "wdlopen libinput.so");

    wdl_sym s = wdlsym(h, sym, NULL, WDL_DEBUG);
    if (!s) s = wdlsym(h, sym, NULL, WDL_DEFAULT);
    if (s) {
        test_ok(t, "wdlsym libinput consume");
        test_note(t, "  sym=%p", (void *)s);
    } else {
        test_fail(t, "wdlsym libinput", "symbol missing");
    }
    wdlclose(h);
}

static void test_decor(test_log *t) {
    wdl_h h = wdlopen("log", WDL_DECOR | WDL_DEFAULT);
    if (!h) test_note(t, "  WDL_DECOR log optional miss");
    else {
        test_ok(t, "wdlopen WDL_DECOR log");
        wdlclose(h);
    }
}

std::string run_wrapdl_tests(const char *lib_dir, const char *self_path) {
    test_log log;
    test_init(&log);
    test_note(&log, "=== wrapdl android ===");
    test_note(&log, "libdir=%s", lib_dir ? lib_dir : "?");
    test_lib_on_disk(&log, lib_dir, "libdummyA.so");
    test_lib_on_disk(&log, lib_dir, "libdummyB.so");
    test_lib_on_disk(&log, lib_dir, "libdummyC.so");

    test_runtime_patch(&log, self_path, lib_dir);
    test_open_close(&log, lib_dir);
    test_sym(&log, lib_dir);
    test_addr_iter(&log, lib_dir);
    test_decor(&log);
    test_ns_merge(&log, lib_dir);
    test_ns_bypass(&log, lib_dir);
    test_cross_ns_dep(&log, lib_dir);
    test_system_sym(&log);

    test_note(&log, "pass=%d fail=%d", log.pass, log.fail);
    std::string out = log.buf ? log.buf : "";
    test_free(&log);
    return out;
}
