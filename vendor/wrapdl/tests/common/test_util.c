#include <string.h>
#include "test_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __ANDROID__
#include <android/log.h>
#define TEST_LOG_TAG "WrapDLTest"
static void test_logcat(const char *line) {
    int lvl = (strncmp(line, "[fail]", 6) == 0) ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO;
    __android_log_print(lvl, TEST_LOG_TAG, "%s", line);
}
#endif

static void test_grow(test_log *t, size_t need) {
    if (t->len + need + 1 <= t->cap) return;
    size_t ncap = t->cap ? t->cap * 2 : 512;
    while (ncap < t->len + need + 1) ncap *= 2;
    char *nb = realloc(t->buf, ncap);
    if (!nb) return;
    t->buf = nb;
    t->cap = ncap;
}

void test_init(test_log *t) {
    memset(t, 0, sizeof(*t));
}

void test_free(test_log *t) {
    free(t->buf);
    memset(t, 0, sizeof(*t));
}

void test_note(test_log *t, const char *fmt, ...) {
    va_list ap;
    char line[512];
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    test_grow(t, strlen(line) + 2);
    t->len += (size_t)snprintf(t->buf + t->len, t->cap - t->len, "%s\n", line);
#ifdef __ANDROID__
    test_logcat(line);
#endif
}

void test_ok(test_log *t, const char *name) {
    t->pass++;
    test_note(t, "[ok] %s", name);
}

void test_fail(test_log *t, const char *name, const char *fmt, ...) {
    char msg[384];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    t->fail++;
    test_note(t, "[fail] %s: %s", name, msg);
}
