#include "test_util.h"

#include <oscompat/bytes.h>

#include <string.h>

static void testLittleEndian16(void) {
	unsigned char buf[8];

	memset(buf, 0xAA, sizeof(buf));
	os_store_le16(buf, 0x1234);
	TEST_INT_EQ(0x34, buf[0]);
	TEST_INT_EQ(0x12, buf[1]);
	TEST_INT_EQ(0xAA, buf[2]);
	TEST_INT_EQ(0x1234, os_load_le16(buf));

	os_store_le16(buf, 0x00FF);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0x00, buf[1]);
	TEST_INT_EQ(0x00FF, os_load_le16(buf));

	os_store_le16(buf, 0xFF00);
	TEST_INT_EQ(0x00, buf[0]);
	TEST_INT_EQ(0xFF, buf[1]);
	TEST_INT_EQ(0xFF00, os_load_le16(buf));

	os_store_le16(buf, 0x0000);
	TEST_INT_EQ(0x00, buf[0]);
	TEST_INT_EQ(0x00, buf[1]);
	TEST_INT_EQ(0x0000, os_load_le16(buf));

	os_store_le16(buf, 0xFFFF);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0xFF, buf[1]);
	TEST_INT_EQ(0xFFFF, os_load_le16(buf));
}

static void testBigEndian16(void) {
	unsigned char buf[8];

	memset(buf, 0xAA, sizeof(buf));
	os_store_be16(buf, 0x1234);
	TEST_INT_EQ(0x12, buf[0]);
	TEST_INT_EQ(0x34, buf[1]);
	TEST_INT_EQ(0xAA, buf[2]);
	TEST_INT_EQ(0x1234, os_load_be16(buf));

	os_store_be16(buf, 0x00FF);
	TEST_INT_EQ(0x00, buf[0]);
	TEST_INT_EQ(0xFF, buf[1]);
	TEST_INT_EQ(0x00FF, os_load_be16(buf));

	os_store_be16(buf, 0xFF00);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0x00, buf[1]);
	TEST_INT_EQ(0xFF00, os_load_be16(buf));

	os_store_be16(buf, 0xFFFF);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0xFF, buf[1]);
	TEST_INT_EQ(0xFFFF, os_load_be16(buf));
}

static void testLittleEndian32(void) {
	unsigned char buf[8];

	memset(buf, 0xAA, sizeof(buf));
	os_store_le32(buf, 0x12345678u);
	TEST_INT_EQ(0x78, buf[0]);
	TEST_INT_EQ(0x56, buf[1]);
	TEST_INT_EQ(0x34, buf[2]);
	TEST_INT_EQ(0x12, buf[3]);
	TEST_INT_EQ(0xAA, buf[4]);
	TEST_INT_EQ(0x12345678, os_load_le32(buf));

	os_store_le32(buf, 0x000000FFu);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0x00, buf[1]);
	TEST_INT_EQ(0x00, buf[2]);
	TEST_INT_EQ(0x00, buf[3]);
	TEST_INT_EQ(0x000000FF, os_load_le32(buf));

	os_store_le32(buf, 0xFF000000u);
	TEST_INT_EQ(0x00, buf[0]);
	TEST_INT_EQ(0x00, buf[1]);
	TEST_INT_EQ(0x00, buf[2]);
	TEST_INT_EQ(0xFF, buf[3]);
	TEST_INT_EQ(0xFF000000, os_load_le32(buf));

	os_store_le32(buf, 0xDEADBEEFu);
	TEST_INT_EQ(0xEF, buf[0]);
	TEST_INT_EQ(0xBE, buf[1]);
	TEST_INT_EQ(0xAD, buf[2]);
	TEST_INT_EQ(0xDE, buf[3]);
	TEST_INT_EQ(0xDEADBEEF, os_load_le32(buf));
}

static void testBigEndian32(void) {
	unsigned char buf[8];

	memset(buf, 0xAA, sizeof(buf));
	os_store_be32(buf, 0x12345678u);
	TEST_INT_EQ(0x12, buf[0]);
	TEST_INT_EQ(0x34, buf[1]);
	TEST_INT_EQ(0x56, buf[2]);
	TEST_INT_EQ(0x78, buf[3]);
	TEST_INT_EQ(0xAA, buf[4]);
	TEST_INT_EQ(0x12345678, os_load_be32(buf));

	os_store_be32(buf, 0x000000FFu);
	TEST_INT_EQ(0x00, buf[0]);
	TEST_INT_EQ(0x00, buf[1]);
	TEST_INT_EQ(0x00, buf[2]);
	TEST_INT_EQ(0xFF, buf[3]);
	TEST_INT_EQ(0x000000FF, os_load_be32(buf));

	os_store_be32(buf, 0xFF000000u);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0x00, buf[1]);
	TEST_INT_EQ(0x00, buf[2]);
	TEST_INT_EQ(0x00, buf[3]);
	TEST_INT_EQ(0xFF000000, os_load_be32(buf));

	os_store_be32(buf, 0xDEADBEEFu);
	TEST_INT_EQ(0xDE, buf[0]);
	TEST_INT_EQ(0xAD, buf[1]);
	TEST_INT_EQ(0xBE, buf[2]);
	TEST_INT_EQ(0xEF, buf[3]);
	TEST_INT_EQ(0xDEADBEEF, os_load_be32(buf));
}

static void testLittleEndian64(void) {
	unsigned char buf[16];

	memset(buf, 0xAA, sizeof(buf));
	os_store_le64(buf, 0x0102030405060708ULL);
	TEST_INT_EQ(0x08, buf[0]);
	TEST_INT_EQ(0x07, buf[1]);
	TEST_INT_EQ(0x06, buf[2]);
	TEST_INT_EQ(0x05, buf[3]);
	TEST_INT_EQ(0x04, buf[4]);
	TEST_INT_EQ(0x03, buf[5]);
	TEST_INT_EQ(0x02, buf[6]);
	TEST_INT_EQ(0x01, buf[7]);
	TEST_INT_EQ(0xAA, buf[8]);
	TEST_ASSERT(os_load_le64(buf) == 0x0102030405060708ULL);

	os_store_le64(buf, 0x00000000000000FFULL);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0x00, buf[7]);
	TEST_ASSERT(os_load_le64(buf) == 0x00000000000000FFULL);

	os_store_le64(buf, 0xFF00000000000000ULL);
	TEST_INT_EQ(0x00, buf[0]);
	TEST_INT_EQ(0xFF, buf[7]);
	TEST_ASSERT(os_load_le64(buf) == 0xFF00000000000000ULL);

	os_store_le64(buf, 0xFEDCBA9876543210ULL);
	TEST_INT_EQ(0x10, buf[0]);
	TEST_INT_EQ(0x32, buf[1]);
	TEST_INT_EQ(0x54, buf[2]);
	TEST_INT_EQ(0x76, buf[3]);
	TEST_INT_EQ(0x98, buf[4]);
	TEST_INT_EQ(0xBA, buf[5]);
	TEST_INT_EQ(0xDC, buf[6]);
	TEST_INT_EQ(0xFE, buf[7]);
	TEST_ASSERT(os_load_le64(buf) == 0xFEDCBA9876543210ULL);
}

static void testBigEndian64(void) {
	unsigned char buf[16];

	memset(buf, 0xAA, sizeof(buf));
	os_store_be64(buf, 0x0102030405060708ULL);
	TEST_INT_EQ(0x01, buf[0]);
	TEST_INT_EQ(0x02, buf[1]);
	TEST_INT_EQ(0x03, buf[2]);
	TEST_INT_EQ(0x04, buf[3]);
	TEST_INT_EQ(0x05, buf[4]);
	TEST_INT_EQ(0x06, buf[5]);
	TEST_INT_EQ(0x07, buf[6]);
	TEST_INT_EQ(0x08, buf[7]);
	TEST_INT_EQ(0xAA, buf[8]);
	TEST_ASSERT(os_load_be64(buf) == 0x0102030405060708ULL);

	os_store_be64(buf, 0x00000000000000FFULL);
	TEST_INT_EQ(0x00, buf[0]);
	TEST_INT_EQ(0xFF, buf[7]);
	TEST_ASSERT(os_load_be64(buf) == 0x00000000000000FFULL);

	os_store_be64(buf, 0xFF00000000000000ULL);
	TEST_INT_EQ(0xFF, buf[0]);
	TEST_INT_EQ(0x00, buf[7]);
	TEST_ASSERT(os_load_be64(buf) == 0xFF00000000000000ULL);

	os_store_be64(buf, 0xFEDCBA9876543210ULL);
	TEST_INT_EQ(0xFE, buf[0]);
	TEST_INT_EQ(0xDC, buf[1]);
	TEST_INT_EQ(0xBA, buf[2]);
	TEST_INT_EQ(0x98, buf[3]);
	TEST_INT_EQ(0x76, buf[4]);
	TEST_INT_EQ(0x54, buf[5]);
	TEST_INT_EQ(0x32, buf[6]);
	TEST_INT_EQ(0x10, buf[7]);
	TEST_ASSERT(os_load_be64(buf) == 0xFEDCBA9876543210ULL);
}

static void testLoadFromBytes(void) {
	static const unsigned char leBytes[8] = { 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 };
	static const unsigned char beBytes[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

	TEST_ASSERT(os_load_le64(leBytes) == 0x0102030405060708ULL);
	TEST_ASSERT(os_load_be64(beBytes) == 0x0102030405060708ULL);
	TEST_INT_EQ(0x0708, os_load_le16(leBytes));
	TEST_INT_EQ(0x0102, os_load_be16(beBytes));
	TEST_INT_EQ(0x05060708, os_load_le32(leBytes));
	TEST_INT_EQ(0x01020304, os_load_be32(beBytes));
}

static void testRoundTrip(void) {
	static const uint16_t v16[] = { 0x0000, 0x0001, 0x00FF, 0x0100, 0x1234, 0x8000, 0xABCD, 0xFFFF };
	static const uint32_t v32[] = { 0x00000000u, 0x00000001u, 0x000000FFu, 0x00000100u, 0x12345678u,
		0x80000000u, 0xDEADBEEFu, 0xFFFFFFFEu, 0xFFFFFFFFu };
	static const uint64_t v64[] = { 0x0000000000000000ULL, 0x0000000000000001ULL, 0x00000000000000FFULL,
		0x0000000000000100ULL, 0x0102030405060708ULL, 0x8000000000000000ULL,
		0xFEDCBA9876543210ULL, 0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFFULL };
	unsigned char buf[16];
	size_t i;

	for (i = 0; i < sizeof(v16) / sizeof(v16[0]); i++) {
		os_store_le16(buf, v16[i]);
		TEST_ASSERT(os_load_le16(buf) == v16[i]);
		os_store_be16(buf, v16[i]);
		TEST_ASSERT(os_load_be16(buf) == v16[i]);
	}

	for (i = 0; i < sizeof(v32) / sizeof(v32[0]); i++) {
		os_store_le32(buf, v32[i]);
		TEST_ASSERT(os_load_le32(buf) == v32[i]);
		os_store_be32(buf, v32[i]);
		TEST_ASSERT(os_load_be32(buf) == v32[i]);
	}

	for (i = 0; i < sizeof(v64) / sizeof(v64[0]); i++) {
		os_store_le64(buf, v64[i]);
		TEST_ASSERT(os_load_le64(buf) == v64[i]);
		os_store_be64(buf, v64[i]);
		TEST_ASSERT(os_load_be64(buf) == v64[i]);
	}
}

static void testOppositeOrders(void) {
	unsigned char buf[16];

	os_store_le16(buf, 0x0102);
	TEST_INT_EQ(0x0201, os_load_be16(buf));
	os_store_be16(buf, 0x0102);
	TEST_INT_EQ(0x0201, os_load_le16(buf));

	os_store_le32(buf, 0x01020304u);
	TEST_INT_EQ(0x04030201, os_load_be32(buf));
	os_store_be32(buf, 0x01020304u);
	TEST_INT_EQ(0x04030201, os_load_le32(buf));

	os_store_le64(buf, 0x0102030405060708ULL);
	TEST_ASSERT(os_load_be64(buf) == 0x0807060504030201ULL);
	os_store_be64(buf, 0x0102030405060708ULL);
	TEST_ASSERT(os_load_le64(buf) == 0x0807060504030201ULL);
}

static void testUnaligned(void) {
	unsigned char raw[24];
	size_t off;

	for (off = 1; off <= 7; off++) {
		memset(raw, 0, sizeof(raw));
		os_store_le16(raw + off, 0xA1B2);
		TEST_INT_EQ(0xB2, raw[off]);
		TEST_INT_EQ(0xA1, raw[off + 1]);
		TEST_INT_EQ(0xA1B2, os_load_le16(raw + off));

		memset(raw, 0, sizeof(raw));
		os_store_be16(raw + off, 0xA1B2);
		TEST_INT_EQ(0xA1, raw[off]);
		TEST_INT_EQ(0xB2, raw[off + 1]);
		TEST_INT_EQ(0xA1B2, os_load_be16(raw + off));

		memset(raw, 0, sizeof(raw));
		os_store_le32(raw + off, 0x0A0B0C0Du);
		TEST_INT_EQ(0x0D, raw[off]);
		TEST_INT_EQ(0x0C, raw[off + 1]);
		TEST_INT_EQ(0x0B, raw[off + 2]);
		TEST_INT_EQ(0x0A, raw[off + 3]);
		TEST_INT_EQ(0x0A0B0C0D, os_load_le32(raw + off));

		memset(raw, 0, sizeof(raw));
		os_store_be32(raw + off, 0x0A0B0C0Du);
		TEST_INT_EQ(0x0A, raw[off]);
		TEST_INT_EQ(0x0B, raw[off + 1]);
		TEST_INT_EQ(0x0C, raw[off + 2]);
		TEST_INT_EQ(0x0D, raw[off + 3]);
		TEST_INT_EQ(0x0A0B0C0D, os_load_be32(raw + off));

		memset(raw, 0, sizeof(raw));
		os_store_le64(raw + off, 0x0102030405060708ULL);
		TEST_INT_EQ(0x08, raw[off]);
		TEST_INT_EQ(0x01, raw[off + 7]);
		TEST_ASSERT(os_load_le64(raw + off) == 0x0102030405060708ULL);

		memset(raw, 0, sizeof(raw));
		os_store_be64(raw + off, 0x0102030405060708ULL);
		TEST_INT_EQ(0x01, raw[off]);
		TEST_INT_EQ(0x08, raw[off + 7]);
		TEST_ASSERT(os_load_be64(raw + off) == 0x0102030405060708ULL);
	}
}

static void testBounds(void) {
	unsigned char buf[16];
	unsigned char copy[16];
	size_t i;

	for (i = 0; i < sizeof(buf); i++) {
		buf[i] = 0x5C;
	}
	os_store_le64(buf + 4, 0x0102030405060708ULL);
	for (i = 0; i < 4; i++) {
		TEST_INT_EQ(0x5C, buf[i]);
	}
	for (i = 12; i < sizeof(buf); i++) {
		TEST_INT_EQ(0x5C, buf[i]);
	}

	for (i = 0; i < sizeof(buf); i++) {
		buf[i] = 0x5C;
	}
	os_store_be64(buf + 4, 0x0102030405060708ULL);
	for (i = 0; i < 4; i++) {
		TEST_INT_EQ(0x5C, buf[i]);
	}
	for (i = 12; i < sizeof(buf); i++) {
		TEST_INT_EQ(0x5C, buf[i]);
	}

	for (i = 0; i < sizeof(buf); i++) {
		buf[i] = (unsigned char)i;
	}
	memcpy(copy, buf, sizeof(buf));
	TEST_INT_EQ(0x0302, os_load_le16(buf + 2));
	TEST_INT_EQ(0x0203, os_load_be16(buf + 2));
	TEST_INT_EQ(0x05040302, os_load_le32(buf + 2));
	TEST_INT_EQ(0x02030405, os_load_be32(buf + 2));
	TEST_ASSERT(os_load_le64(buf) == 0x0706050403020100ULL);
	TEST_ASSERT(os_load_be64(buf) == 0x0001020304050607ULL);
	TEST_MEM_EQ(copy, buf, sizeof(buf));
}

int main(void) {
	testLittleEndian16();
	testBigEndian16();
	testLittleEndian32();
	testBigEndian32();
	testLittleEndian64();
	testBigEndian64();
	testLoadFromBytes();
	testRoundTrip();
	testOppositeOrders();
	testUnaligned();
	testBounds();
	return os_test_summary("bytes");
}
