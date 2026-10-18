#define _GNU_SOURCE 1

#include "test_util.h"

#include <oscompat/string.h>

#include <stdlib.h>
#include <string.h>

static void testMemmem(void) {
	static const char hay[] = "hello world";
	static const char needle[] = "world";
	static const char repeated[] = "aaaaab";
	static const char abc[] = "abcabc";
	static const unsigned char bin[] = { 'a', 'b', 0, 'c', 'd', 0, 'e', 'f' };
	static const unsigned char pat[] = { 'c', 'd', 0 };

	TEST_ASSERT(os_memmem(hay, sizeof(hay) - 1, needle, sizeof(needle) - 1) == (const void*)(hay + 6));
	TEST_ASSERT(os_memmem(hay, sizeof(hay) - 1, "", 0) == (const void*)hay);
	TEST_ASSERT(os_memmem(hay, 0, "", 0) == (const void*)hay);
	TEST_ASSERT(os_memmem(hay, sizeof(hay) - 1, "xyz", 3) == NULL);
	TEST_ASSERT(os_memmem(hay, 3, needle, sizeof(needle) - 1) == NULL);
	TEST_ASSERT(os_memmem(hay, sizeof(hay) - 1, hay, sizeof(hay) - 1) == (const void*)hay);
	TEST_ASSERT(os_memmem(repeated, 6, "aab", 3) == (const void*)(repeated + 3));
	TEST_ASSERT(os_memmem(abc, 6, "bc", 2) == (const void*)(abc + 1));
	TEST_ASSERT(os_memmem(abc, 6, "c", 1) == (const void*)(abc + 2));
	TEST_ASSERT(os_memmem(abc, 6, "cab", 3) == (const void*)(abc + 2));
	TEST_ASSERT(os_memmem("abc", 3, "abcd", 4) == NULL);
	TEST_ASSERT(os_memmem(bin, sizeof(bin), pat, sizeof(pat)) == (const void*)(bin + 3));
	TEST_ASSERT(os_memmem(bin, sizeof(bin), "\0e", 2) == (const void*)(bin + 5));
	TEST_ASSERT(os_memmem(bin, sizeof(bin), "gh", 2) == NULL);

#if defined(__GLIBC__)
	{
		static const char text[] = "the quick brown fox jumps over the lazy dog";
		size_t textLen = sizeof(text) - 1;
		size_t i;

		for (i = 0; i < textLen; i++) {
			TEST_ASSERT(os_memmem(text, textLen, text + i, textLen - i) == memmem(text, textLen, text + i, textLen - i));
		}
		TEST_ASSERT(os_memmem(text, textLen, "brown", 5) == memmem(text, textLen, "brown", 5));
		TEST_ASSERT(os_memmem(text, textLen, "o", 1) == memmem(text, textLen, "o", 1));
		TEST_ASSERT(os_memmem(text, textLen, "zz", 2) == memmem(text, textLen, "zz", 2));
	}
#endif
}

static void testMemrchr(void) {
	static const char s[] = "abcabc";
	static const char withNul[] = "ab\0c";

	TEST_ASSERT(os_memrchr(s, 'b', 6) == (const void*)(s + 4));
	TEST_ASSERT(os_memrchr(s, 'a', 6) == (const void*)(s + 3));
	TEST_ASSERT(os_memrchr(s, 'c', 6) == (const void*)(s + 5));
	TEST_ASSERT(os_memrchr(s, 'z', 6) == NULL);
	TEST_ASSERT(os_memrchr(s, 'a', 0) == NULL);
	TEST_ASSERT(os_memrchr(s, 'a', 3) == (const void*)s);
	TEST_ASSERT(os_memrchr(s, 'a', 1) == (const void*)s);
	TEST_ASSERT(os_memrchr(withNul, '\0', 4) == (const void*)(withNul + 2));
	TEST_ASSERT(os_memrchr(withNul, 'c', 4) == (const void*)(withNul + 3));

#if defined(__GLIBC__)
	{
		size_t i;

		for (i = 0; i <= 6; i++) {
			TEST_ASSERT(os_memrchr(s, 'a', i) == memrchr(s, 'a', i));
			TEST_ASSERT(os_memrchr(s, 'c', i) == memrchr(s, 'c', i));
			TEST_ASSERT(os_memrchr(s, 'z', i) == memrchr(s, 'z', i));
		}
	}
#endif
}

static void testStrchrnul(void) {
	static const char s[] = "hello";
	static const char empty[] = "";

	TEST_ASSERT(os_strchrnul(s, 'l') == s + 2);
	TEST_ASSERT(os_strchrnul(s, 'h') == s);
	TEST_ASSERT(os_strchrnul(s, 'o') == s + 4);
	TEST_ASSERT(os_strchrnul(s, 'z') == s + 5);
	TEST_INT_EQ('\0', os_strchrnul(s, 'z')[0]);
	TEST_ASSERT(os_strchrnul(s, '\0') == s + 5);
	TEST_ASSERT(os_strchrnul(s, 'l')[0] == 'l');
	TEST_ASSERT(os_strchrnul(empty, 'a') == empty);
	TEST_ASSERT(os_strchrnul(empty, '\0') == empty);

#if defined(__GLIBC__)
	{
		int c;

		for (c = 0; c < 256; c++) {
			TEST_ASSERT(os_strchrnul(s, c) == strchrnul(s, c));
		}
	}
#endif
}

static void testStrCaseCmp(void) {
	TEST_INT_EQ(0, os_strcasecmp("", ""));
	TEST_INT_EQ(0, os_strcasecmp("Hello", "hello"));
	TEST_INT_EQ(0, os_strcasecmp("HELLO", "hello"));
	TEST_ASSERT(os_strcasecmp("abc", "abd") < 0);
	TEST_ASSERT(os_strcasecmp("abd", "abc") > 0);
	TEST_ASSERT(os_strcasecmp("", "a") < 0);
	TEST_ASSERT(os_strcasecmp("a", "") > 0);
	TEST_ASSERT(os_strcasecmp("abc", "ab") > 0);
	TEST_ASSERT(os_strcasecmp("ab", "abc") < 0);
	TEST_ASSERT(os_strcasecmp("ABC", "abd") < 0);

	TEST_INT_EQ(0, os_strncasecmp("Hello", "hello", 5));
	TEST_INT_EQ(0, os_strncasecmp("abc", "xyz", 0));
	TEST_INT_EQ(0, os_strncasecmp("", "", 0));
	TEST_INT_EQ(0, os_strncasecmp("abc", "abd", 2));
	TEST_ASSERT(os_strncasecmp("abc", "abd", 3) < 0);
	TEST_ASSERT(os_strncasecmp("abc", "ab", 3) > 0);
	TEST_ASSERT(os_strncasecmp("ABC", "abd", 3) < 0);
	TEST_ASSERT(os_strncasecmp("abd", "ABC", 3) > 0);
	TEST_INT_EQ(0, os_strncasecmp("a", "A", 1));

#if defined(__GLIBC__)
	{
		static const char* const words[] = { "", "a", "A", "ab", "AB", "abc", "ABC", "abd", "hello", "HELLO", "Hellz" };
		size_t i;

		for (i = 0; i < sizeof(words) / sizeof(words[0]); i++) {
			size_t j;

			for (j = 0; j < sizeof(words) / sizeof(words[0]); j++) {
				int mine = os_strcasecmp(words[i], words[j]);
				int ref = strcasecmp(words[i], words[j]);
				size_t n;

				TEST_ASSERT((mine < 0 && ref < 0) || (mine == 0 && ref == 0) || (mine > 0 && ref > 0));
				for (n = 0; n <= 4; n++) {
					int mineN = os_strncasecmp(words[i], words[j], n);
					int refN = strncasecmp(words[i], words[j], n);

					TEST_ASSERT((mineN < 0 && refN < 0) || (mineN == 0 && refN == 0) || (mineN > 0 && refN > 0));
				}
			}
		}
	}
#endif
}

static void testStrDup(void) {
	static const char s[] = "hello";
	char* d;

	d = os_strdup(s);
	TEST_ASSERT(d != NULL);
	TEST_ASSERT(d != s);
	TEST_MEM_EQ(s, d, sizeof(s));
	free(d);

	d = os_strdup("");
	TEST_ASSERT(d != NULL);
	TEST_INT_EQ(0, d[0]);
	free(d);

	d = os_strndup(s, 2);
	TEST_ASSERT(d != NULL);
	TEST_STR_EQ("he", d);
	free(d);

	d = os_strndup(s, 5);
	TEST_ASSERT(d != NULL);
	TEST_STR_EQ("hello", d);
	free(d);

	d = os_strndup(s, 100);
	TEST_ASSERT(d != NULL);
	TEST_STR_EQ("hello", d);
	free(d);

	d = os_strndup(s, 0);
	TEST_ASSERT(d != NULL);
	TEST_STR_EQ("", d);
	free(d);

	d = os_strndup("ab\0cd", 5);
	TEST_ASSERT(d != NULL);
	TEST_STR_EQ("ab", d);
	free(d);

#if defined(__GLIBC__)
	{
		static const char* const cases[] = { "", "a", "hello", "hello world" };
		size_t i;

		for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
			size_t n;

			for (n = 0; n <= 12; n++) {
				char* mine = os_strndup(cases[i], n);
				char* ref = strndup(cases[i], n);

				TEST_ASSERT(mine != NULL && ref != NULL);
				TEST_ASSERT(strcmp(mine, ref) == 0);
				free(mine);
				free(ref);
			}
		}
	}
#endif
}

static void testStrlcpy(void) {
	char buf[16];
	size_t n;

	memset(buf, 'X', sizeof(buf));
	n = os_strlcpy(buf, "hello", sizeof(buf));
	TEST_INT_EQ(5, n);
	TEST_STR_EQ("hello", buf);
	TEST_INT_EQ('X', buf[6]);

	memset(buf, 'X', sizeof(buf));
	n = os_strlcpy(buf, "hello", 4);
	TEST_INT_EQ(5, n);
	TEST_STR_EQ("hel", buf);
	TEST_INT_EQ(0, buf[3]);
	TEST_INT_EQ('X', buf[4]);

	buf[0] = 'Q';
	buf[1] = 'Q';
	n = os_strlcpy(buf, "hello", 0);
	TEST_INT_EQ(5, n);
	TEST_INT_EQ('Q', buf[0]);
	TEST_INT_EQ('Q', buf[1]);

	buf[0] = 'Q';
	buf[1] = 'Q';
	n = os_strlcpy(buf, "hello", 1);
	TEST_INT_EQ(5, n);
	TEST_INT_EQ(0, buf[0]);
	TEST_INT_EQ('Q', buf[1]);

	n = os_strlcpy(buf, "hello", 6);
	TEST_INT_EQ(5, n);
	TEST_STR_EQ("hello", buf);

	n = os_strlcpy(buf, "", sizeof(buf));
	TEST_INT_EQ(0, n);
	TEST_INT_EQ(0, buf[0]);

	n = os_strlcpy(buf, "", 0);
	TEST_INT_EQ(0, n);
}

static void testStrlcat(void) {
	char buf[16];
	char small[6];
	size_t n;

	os_strlcpy(buf, "abc", sizeof(buf));
	n = os_strlcat(buf, "def", sizeof(buf));
	TEST_INT_EQ(6, n);
	TEST_STR_EQ("abcdef", buf);

	os_strlcpy(buf, "abc", sizeof(buf));
	n = os_strlcat(buf, "", sizeof(buf));
	TEST_INT_EQ(3, n);
	TEST_STR_EQ("abc", buf);

	os_strlcpy(buf, "abc", sizeof(buf));
	n = os_strlcat(buf, "def", 5);
	TEST_INT_EQ(6, n);
	TEST_STR_EQ("abcd", buf);

	os_strlcpy(buf, "abc", sizeof(buf));
	n = os_strlcat(buf, "def", 4);
	TEST_INT_EQ(6, n);
	TEST_STR_EQ("abc", buf);

	os_strlcpy(buf, "abc", sizeof(buf));
	n = os_strlcat(buf, "def", 3);
	TEST_INT_EQ(6, n);
	TEST_STR_EQ("abc", buf);

	os_strlcpy(buf, "abc", sizeof(buf));
	n = os_strlcat(buf, "def", 2);
	TEST_INT_EQ(5, n);
	TEST_STR_EQ("abc", buf);

	os_strlcpy(buf, "abc", sizeof(buf));
	n = os_strlcat(buf, "def", 0);
	TEST_INT_EQ(3, n);
	TEST_STR_EQ("abc", buf);

	os_strlcpy(buf, "", sizeof(buf));
	n = os_strlcat(buf, "xyz", sizeof(buf));
	TEST_INT_EQ(3, n);
	TEST_STR_EQ("xyz", buf);

	os_strlcpy(buf, "abcdef", sizeof(buf));
	n = os_strlcat(buf, "ghijkl", sizeof(buf));
	TEST_INT_EQ(12, n);
	TEST_STR_EQ("abcdefghijkl", buf);

	memset(small, 'X', sizeof(small));
	small[0] = 'a';
	small[1] = '\0';
	n = os_strlcat(small, "bcdef", sizeof(small));
	TEST_INT_EQ(6, n);
	TEST_STR_EQ("abcde", small);
}

static void testStrsep(void) {
	char buf[] = "aaa,bbb,,ccc";
	char* p = buf;

	TEST_STR_EQ("aaa", os_strsep(&p, ","));
	TEST_STR_EQ("bbb", os_strsep(&p, ","));
	TEST_STR_EQ("", os_strsep(&p, ","));
	TEST_STR_EQ("ccc", os_strsep(&p, ","));
	TEST_ASSERT(p == NULL);
	TEST_ASSERT(os_strsep(&p, ",") == NULL);

	{
		char empty[] = "";
		char* q = empty;
		char* r = os_strsep(&q, ",");

		TEST_ASSERT(r == empty);
		TEST_STR_EQ("", r);
		TEST_ASSERT(q == NULL);
		TEST_ASSERT(os_strsep(&q, ",") == NULL);
	}

	{
		char leading[] = ",,a";
		char* q = leading;

		TEST_STR_EQ("", os_strsep(&q, ","));
		TEST_STR_EQ("", os_strsep(&q, ","));
		TEST_STR_EQ("a", os_strsep(&q, ","));
		TEST_ASSERT(q == NULL);
	}

	{
		char plain[] = "abc";
		char* q = plain;

		TEST_STR_EQ("abc", os_strsep(&q, ","));
		TEST_ASSERT(q == NULL);
	}

	{
		char multi[] = "a;b,c";
		char* q = multi;

		TEST_STR_EQ("a", os_strsep(&q, ";,"));
		TEST_STR_EQ("b", os_strsep(&q, ";,"));
		TEST_STR_EQ("c", os_strsep(&q, ";,"));
		TEST_ASSERT(os_strsep(&q, ";,") == NULL);
	}

	{
		char nodelim[] = "abc";
		char* q = nodelim;

		TEST_STR_EQ("abc", os_strsep(&q, ""));
		TEST_ASSERT(q == NULL);
	}

	TEST_ASSERT(os_strsep(NULL, ",") == NULL);

#if defined(__GLIBC__)
	{
		char mine[] = "a,,b,,,c,";
		char ref[] = "a,,b,,,c,";
		char* qm = mine;
		char* qr = ref;
		int i;

		for (i = 0; i < 8; i++) {
			char* rm = os_strsep(&qm, ",");
			char* rr = strsep(&qr, ",");

			TEST_ASSERT(rm == NULL ? rr == NULL : (rr != NULL && strcmp(rm, rr) == 0));
		}
	}
#endif
}

static void testStrtokR(void) {
	char buf[] = "  a b  c ";
	char* save = NULL;

	TEST_STR_EQ("a", os_strtok_r(buf, " ", &save));
	TEST_STR_EQ("b", os_strtok_r(NULL, " ", &save));
	TEST_STR_EQ("c", os_strtok_r(NULL, " ", &save));
	TEST_ASSERT(os_strtok_r(NULL, " ", &save) == NULL);
	TEST_ASSERT(os_strtok_r(NULL, " ", &save) == NULL);

	{
		char t[] = "a;b,c";
		char* sv = NULL;

		TEST_STR_EQ("a", os_strtok_r(t, ";,", &sv));
		TEST_STR_EQ("b", os_strtok_r(NULL, ";,", &sv));
		TEST_STR_EQ("c", os_strtok_r(NULL, ";,", &sv));
		TEST_ASSERT(os_strtok_r(NULL, ";,", &sv) == NULL);
	}

	{
		char t[] = "";
		char* sv = NULL;

		TEST_ASSERT(os_strtok_r(t, ",", &sv) == NULL);
	}

	{
		char t[] = ",,,";
		char* sv = NULL;

		TEST_ASSERT(os_strtok_r(t, ",", &sv) == NULL);
	}

	{
		char* sv = NULL;

		TEST_ASSERT(os_strtok_r(NULL, ",", &sv) == NULL);
	}

	{
		char t[] = "x y";
		char* sv = NULL;

		TEST_ASSERT(os_strtok_r(t, " ", &sv) == t);
		TEST_INT_EQ('x', t[0]);
		TEST_INT_EQ(0, t[1]);
		TEST_ASSERT(os_strtok_r(NULL, " ", &sv) == t + 2);
	}

#if defined(__GLIBC__)
	{
		char mine[] = "  aa bb  cc dd ";
		char ref[] = "  aa bb  cc dd ";
		char* sm = NULL;
		char* sr = NULL;
		int i;

		for (i = 0; i < 6; i++) {
			char* rm = os_strtok_r(i == 0 ? mine : NULL, " ", &sm);
			char* rr = strtok_r(i == 0 ? ref : NULL, " ", &sr);

			TEST_ASSERT(rm == NULL ? rr == NULL : (rr != NULL && strcmp(rm, rr) == 0));
		}
	}
#endif
}

static void testExplicitBzero(void) {
	unsigned char buf[32];
	size_t i;

	memset(buf, 0xAA, sizeof(buf));
	os_explicit_bzero(buf, sizeof(buf));
	for (i = 0; i < sizeof(buf); i++) {
		TEST_INT_EQ(0, buf[i]);
	}

	memset(buf, 0x5A, sizeof(buf));
	os_explicit_bzero(buf, 0);
	for (i = 0; i < sizeof(buf); i++) {
		TEST_INT_EQ(0x5A, buf[i]);
	}

	memset(buf, 0xFF, sizeof(buf));
	os_explicit_bzero(buf + 4, 3);
	for (i = 0; i < sizeof(buf); i++) {
		if (i >= 4 && i < 7) {
			TEST_INT_EQ(0, buf[i]);
		} else {
			TEST_INT_EQ(0xFF, buf[i]);
		}
	}

	memset(buf, 0x11, sizeof(buf));
	os_explicit_bzero(buf, 1);
	TEST_INT_EQ(0, buf[0]);
	TEST_INT_EQ(0x11, buf[1]);
}

int main(void) {
	testMemmem();
	testMemrchr();
	testStrchrnul();
	testStrCaseCmp();
	testStrDup();
	testStrlcpy();
	testStrlcat();
	testStrsep();
	testStrtokR();
	testExplicitBzero();
	return os_test_summary("string");
}
