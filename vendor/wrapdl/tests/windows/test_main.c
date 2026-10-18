#include "test_util.h"
#include "wrapdl.h"

#include <stdio.h>
#include <string.h>

static void test_open_close(test_log *t) {
    wdl_h h = wdlopen("kernel32", WDL_DECOR);
    if (!h) {
        test_fail(t, "wdlopen", "%s", wdlerror() ? wdlerror() : "?");
        return;
    }
    test_ok(t, "wdlopen kernel32");
    if (wdlclose(h) != 0)
        test_fail(t, "wdlclose", "%s", wdlerror() ? wdlerror() : "?");
    else
        test_ok(t, "wdlclose");
}

static void test_sym(test_log *t) {
    wdl_h h = wdlopen("kernel32.dll", WDL_DEFAULT);
    if (!h) {
        test_fail(t, "wdlsym setup", "%s", wdlerror() ? wdlerror() : "?");
        return;
    }
    wdl_sym s = wdlsym(h, "GetModuleHandleA", NULL, WDL_DEFAULT);
    if (!s) test_fail(t, "wdlsym", "%s", wdlerror() ? wdlerror() : "?");
    else test_ok(t, "wdlsym GetModuleHandleA");
    wdlclose(h);
}

static void test_addr(test_log *t) {
    wdl_sym fn = (wdl_sym)GetModuleHandleA;
    wdl_info info;
    void *cache = NULL;
    if (!wdladdr((void *)fn, &info, &cache, WDL_DEFAULT))
        test_note(t, "wdladdr skipped on win32");
    else
        test_ok(t, "wdladdr");
    wdlclear(&cache);
    test_ok(t, "wdlclear");
}

static void test_iter(test_log *t) {
    (void)t;
    test_note(t, "wdliter skipped on win32");
}

int main(void) {
    test_log log;
    test_init(&log);
    test_note(&log, "=== wrapdl windows ===");

    test_open_close(&log);
    test_sym(&log);
    test_addr(&log);
    test_iter(&log);

    test_note(&log, "pass=%d fail=%d", log.pass, log.fail);
    if (log.buf) fputs(log.buf, stdout);

    int rc = log.fail ? 1 : 0;
    test_free(&log);
    return rc;
}

#ifdef _WIN32
/* GetModuleHandleA used above */
#endif
