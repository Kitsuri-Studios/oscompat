#ifndef OSCOMPAT_TEST_UTIL_H
#define OSCOMPAT_TEST_UTIL_H

#include <stdio.h>
#include <string.h>

static int osTestFailures = 0;
static int osTestChecks = 0;

#define TEST_ASSERT(cond) \
	do { \
		osTestChecks++; \
		if (!(cond)) { \
			osTestFailures++; \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
		} \
	} while (0)

#define TEST_INT_EQ(expected, actual) \
	do { \
		long long osE = (long long)(expected); \
		long long osA = (long long)(actual); \
		osTestChecks++; \
		if (osE != osA) { \
			osTestFailures++; \
			fprintf(stderr, "FAIL %s:%d: expected %lld got %lld\n", \
				__FILE__, __LINE__, osE, osA); \
		} \
	} while (0)

#define TEST_STR_EQ(expected, actual) \
	do { \
		const char* osE = (expected); \
		const char* osA = (actual); \
		osTestChecks++; \
		if (osE == NULL || osA == NULL || strcmp(osE, osA) != 0) { \
			osTestFailures++; \
			fprintf(stderr, "FAIL %s:%d: expected \"%s\" got \"%s\"\n", \
				__FILE__, __LINE__, osE ? osE : "(null)", osA ? osA : "(null)"); \
		} \
	} while (0)

#define TEST_MEM_EQ(expected, actual, size) \
	do { \
		size_t osN = (size_t)(size); \
		osTestChecks++; \
		if (memcmp((expected), (actual), osN) != 0) { \
			osTestFailures++; \
			fprintf(stderr, "FAIL %s:%d: memory mismatch (%lu bytes)\n", \
				__FILE__, __LINE__, (unsigned long)osN); \
		} \
	} while (0)

static int os_test_summary(const char* name) {
	if (osTestFailures == 0) {
		printf("PASS %s (%d checks)\n", name, osTestChecks);
		return 0;
	}
	printf("FAIL %s (%d of %d checks failed)\n", name, osTestFailures, osTestChecks);
	return 1;
}

#endif
