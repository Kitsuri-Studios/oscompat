#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
    int pass;
    int fail;
} test_log;

void test_init(test_log *t);
void test_free(test_log *t);
void test_ok(test_log *t, const char *name);
void test_fail(test_log *t, const char *name, const char *fmt, ...);
void test_note(test_log *t, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
