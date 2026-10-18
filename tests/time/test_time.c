#include "test_util.h"

#include <oscompat/time.h>

int main(void) {
	uint64_t monoFirst = os_time_monotonic_ms();
	uint64_t monoSecond = os_time_monotonic_ms();
	uint64_t sleepStart;
	uint64_t sleepEnd;
	uint64_t usStart;
	uint64_t usEnd;
	uint64_t realtime = os_time_realtime_ms();

	TEST_ASSERT(monoSecond >= monoFirst);

	sleepStart = os_time_monotonic_ms();
	os_sleep_ms(20);
	sleepEnd = os_time_monotonic_ms();
	TEST_ASSERT(sleepEnd - sleepStart >= 15);

	TEST_ASSERT(realtime > 1600000000000ULL);

	usStart = os_time_monotonic_us();
	os_sleep_us(2000);
	usEnd = os_time_monotonic_us();
	TEST_ASSERT(usEnd - usStart >= 1000);

	os_sleep_ms(0);
	os_sleep_us(0);
	TEST_ASSERT(os_time_monotonic_ms() >= monoSecond);

	return os_test_summary("time");
}
