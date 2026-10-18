#include "wrapdl.h"

int dummyC_value(void) {
    return 126;
}

int dummyC_call_b(const char *b_path) {
    wdl_h h = wdlopen(b_path, WDL_DEFAULT);
    if (!h) return -1;
    typedef int (*fn)(void);
    fn f = (fn)wdlsym(h, "dummyB_value", NULL, WDL_DEFAULT);
    if (!f) {
        wdlclose(h);
        return -2;
    }
    int v = f();
    wdlclose(h);
    return v;
}
