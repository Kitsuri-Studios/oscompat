#include "wrapdl.h"

#if defined(__ANDROID__)

static struct android_namespace_t *g_main;

static int wdlns_init_exported(void) {
    static const char *names[] = {"classloader", "default", NULL};
    if (!wdlns_ok()) return 0;
    for (int i = 0; names[i]; i++) {
        struct android_namespace_t *ns = wdlns_exported(names[i]);
        if (!ns) continue;
        g_main = ns;
        wdlns_link_default(g_main);
        return 1;
    }
    return 0;
}

int wdlns_init(const char *self_path) {
    (void)self_path;
    if (g_main) return 1;
    return wdlns_init_exported();
}

int wdlns_paths(const char *const *paths, size_t n) {
    (void)paths;
    (void)n;
    return g_main ? 1 : 0;
}

int wdlns_isolated(int yes) {
    (void)yes;
    return g_main ? 1 : 0;
}

struct android_namespace_t *wdlns_main(void) {
    return g_main;
}

#else

int wdlns_init(const char *self_path) { (void)self_path; return 0; }
int wdlns_paths(const char *const *paths, size_t n) { (void)paths; (void)n; return 0; }
int wdlns_isolated(int yes) { (void)yes; return 0; }
struct android_namespace_t *wdlns_main(void) { return NULL; }

#endif
