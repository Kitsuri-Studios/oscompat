#include <oscompat/bytes.h>

uint16_t os_load_le16(const void* p) {
	const unsigned char* b = (const unsigned char*)p;

	return (uint16_t)((uint16_t)b[0] | (uint16_t)((uint16_t)b[1] << 8));
}

uint32_t os_load_le32(const void* p) {
	const unsigned char* b = (const unsigned char*)p;

	return (uint32_t)((uint32_t)b[0] |
		((uint32_t)b[1] << 8) |
		((uint32_t)b[2] << 16) |
		((uint32_t)b[3] << 24));
}

uint64_t os_load_le64(const void* p) {
	const unsigned char* b = (const unsigned char*)p;

	return (uint64_t)((uint64_t)b[0] |
		((uint64_t)b[1] << 8) |
		((uint64_t)b[2] << 16) |
		((uint64_t)b[3] << 24) |
		((uint64_t)b[4] << 32) |
		((uint64_t)b[5] << 40) |
		((uint64_t)b[6] << 48) |
		((uint64_t)b[7] << 56));
}

uint16_t os_load_be16(const void* p) {
	const unsigned char* b = (const unsigned char*)p;

	return (uint16_t)((uint16_t)((uint16_t)b[0] << 8) | (uint16_t)b[1]);
}

uint32_t os_load_be32(const void* p) {
	const unsigned char* b = (const unsigned char*)p;

	return (uint32_t)(((uint32_t)b[0] << 24) |
		((uint32_t)b[1] << 16) |
		((uint32_t)b[2] << 8) |
		(uint32_t)b[3]);
}

uint64_t os_load_be64(const void* p) {
	const unsigned char* b = (const unsigned char*)p;

	return (uint64_t)(((uint64_t)b[0] << 56) |
		((uint64_t)b[1] << 48) |
		((uint64_t)b[2] << 40) |
		((uint64_t)b[3] << 32) |
		((uint64_t)b[4] << 24) |
		((uint64_t)b[5] << 16) |
		((uint64_t)b[6] << 8) |
		(uint64_t)b[7]);
}

void os_store_le16(void* p, uint16_t v) {
	unsigned char* b = (unsigned char*)p;

	b[0] = (unsigned char)(v & 0xFFu);
	b[1] = (unsigned char)((v >> 8) & 0xFFu);
}

void os_store_le32(void* p, uint32_t v) {
	unsigned char* b = (unsigned char*)p;

	b[0] = (unsigned char)(v & 0xFFu);
	b[1] = (unsigned char)((v >> 8) & 0xFFu);
	b[2] = (unsigned char)((v >> 16) & 0xFFu);
	b[3] = (unsigned char)((v >> 24) & 0xFFu);
}

void os_store_le64(void* p, uint64_t v) {
	unsigned char* b = (unsigned char*)p;

	b[0] = (unsigned char)(v & 0xFFu);
	b[1] = (unsigned char)((v >> 8) & 0xFFu);
	b[2] = (unsigned char)((v >> 16) & 0xFFu);
	b[3] = (unsigned char)((v >> 24) & 0xFFu);
	b[4] = (unsigned char)((v >> 32) & 0xFFu);
	b[5] = (unsigned char)((v >> 40) & 0xFFu);
	b[6] = (unsigned char)((v >> 48) & 0xFFu);
	b[7] = (unsigned char)((v >> 56) & 0xFFu);
}

void os_store_be16(void* p, uint16_t v) {
	unsigned char* b = (unsigned char*)p;

	b[0] = (unsigned char)((v >> 8) & 0xFFu);
	b[1] = (unsigned char)(v & 0xFFu);
}

void os_store_be32(void* p, uint32_t v) {
	unsigned char* b = (unsigned char*)p;

	b[0] = (unsigned char)((v >> 24) & 0xFFu);
	b[1] = (unsigned char)((v >> 16) & 0xFFu);
	b[2] = (unsigned char)((v >> 8) & 0xFFu);
	b[3] = (unsigned char)(v & 0xFFu);
}

void os_store_be64(void* p, uint64_t v) {
	unsigned char* b = (unsigned char*)p;

	b[0] = (unsigned char)((v >> 56) & 0xFFu);
	b[1] = (unsigned char)((v >> 48) & 0xFFu);
	b[2] = (unsigned char)((v >> 40) & 0xFFu);
	b[3] = (unsigned char)((v >> 32) & 0xFFu);
	b[4] = (unsigned char)((v >> 24) & 0xFFu);
	b[5] = (unsigned char)((v >> 16) & 0xFFu);
	b[6] = (unsigned char)((v >> 8) & 0xFFu);
	b[7] = (unsigned char)(v & 0xFFu);
}
